Functor
=======

Formal definition
-----------------
Given the two categories _C_ and _D_, the functor _F_ from _C_ to _D_ is then a mapping that:

* associates to each object _X_ in _C_ an object `F(X)` in _D_.
* associates to each morphism `f : X -> Y` in _C_ a morphism `F(f) : F(X) -> F(Y)` in _D_, such that the following conditions hold:
  
  1. `F(id_X) = id_F(X)` for every object _X_ in _C_
  2. `F(g ∘ f) = F(g) ∘ F(f)` for all morphisms `f : x -> y` and `g : y -> z`.
In other words, a mapping must preserve identity morphisms as well as composition of morphisms.

FTL definition
--------------
In FTL, the mapping mentioned in the formal definition isn't usually what we refer to as the functor. Instead, we call a particular _type_ a functor when it is possible to create such mappings to it. This slightly strange definition is inherited from Haskell, not because Haskell can do no wrong, but because it makes sense in the context of programming. After all, there are much fewer types than mappings to them, so it makes sense to implement the concept per type, rather than per mapping.

Much like the other concepts in FTL, _functor_ appears as a templated struct that each type that wants to claim to belong to must specialise. In the case of functors, this struct is quite minimal:
```cpp
template<template<typename> class F>
struct functor {
    // B is automatically derived and is the return type of the morphism Fn
    template<typename Fn, typename A, typename B = ....>
    static F<B> map(Fn fn, F<A> f) {
        return applicative<F>::map(fn, f);
    }

    static constexpr bool instance = applicative<F>::instance;
};

// The overloaded use of this operator is setup so it is only ever considered
// for types that specialise functor.
template<
        template<typename> class F>,
        typename Fn,
        typename A,
        typename B = ...>
F<B> operator% (Fn f, F<A> f) {
    return functor<F>::map(f, f);
}
```
As you can see, all of the members of the concept have default implementations, some pointing to the [applicative](Applicative.md) concept. That's because all applicative functors are per definition also regular functors, but the reverse is not necessarily true. This means that if a type implements _applicative_, it does not need to implement _functor_ too, that is provided automatically.

FTL instances
-------------
The following standard library types have been given functor implementations in FTL:
* `std::shared_ptr<T>`: by mapping a function to a `shared_ptr`, it is applied only if it is actively managing data. Otherwise, nothing is done and an empty pointer is returned.
* `std::vector<T>` and, isomorphically, `std::list<T>`. For containers, mapping a function has the effect of applying it once to each element in the container, and then collecting all the results and returning them in a new container.
* `std::tuple<T,Ts...>` is a functor on _T_. In other words, mapping a function applies it to the first field in the tuple and returns a new tuple with the value (and possibly type) of the first element changed.
* `std::future<T>` can map a function to a future value, yielding a future that when `get`ed applies the function after waiting for the original future.

In addition, the following FTL data types are functors:
* `maybe<T>`, in a way isomorphic to `std::shared_ptr<T>`.
* `either<L,R>` is a functor on _L_, similar to how `std::tuple<T,Ts...>` is a functor on _T_.
* `function<R,Ps...>` is a functor on _R_. Mapping a function to this type is exactly equivalent to composing the two functions (see [compose](functional.h.md#compose)).

Custom instances
----------------
To make a new type an instance of functor, you can implement either of the concepts [applicative functors](Applicative.md) or [monads](Monad.md), or you can create a specialisation for the struct given in the FTL definition above. For example, a functor instance for `maybe<T>` can be written as:
```cpp
template<>
struct functor<maybe> {
    template<
        typename F,
        typename A,
        typename B = typename decayed_result<F(A)>::type>
    static maybe<B> map(F f, const maybe<A>& m) {
        if(m)
            return value(f(*m));
        else
            return maybe<B>();
    }

    static constexpr bool instance = true;
};
```
The above is a minimal implementation, every method and member of functor implemented above _must_ be present to form a complete instance of functor. However, the exact signature of _map_ can vary, if you prefer pass-by-value, for instance. It is naturally also possible to overload for rvalue references or add a _noexcept_-specifier if apropriate, and so on.

Examples
--------
To be added.
