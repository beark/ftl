Applicative Functor
===================

An applicative functor provides slightly more structure than a [functor](Functor.md), but slightly less than a [monad](Monad.ms). As such, less types can be viewed as applicative functors than functors, but on the other hand, we can do more interesting things with those types&mdash;even though we know nothing more about them than that they are applciative functors.

ftl definition
--------------
There is, as far as I know, no formal definition of applicative functors, they are purely an invention of the Haskell community. Thus, the ftl definition is entirely based on the behaviour provided in the Haskell definition.

As all the other major concepts in ftl, applicative functors are defined in terms of specialisations of a struct. The basic, unspecialised version of this struct&mdash;which defines the interface specialisations must conform to&mdash;is as follows:
```cpp
template<template<typename> class F>
struct applicative {
     template<typename A>
     static F<A> pure(const A& a) {
         return monad<F>::pure(a);
     }

     // B is automatically deduced as the result of Fn(A)
     template<typename Fn, typename A, typename B = ...>
     static F<B> map(Fn fn, const F<A>& f) {
         return monad<F>::map(fn, f);
     }

    template<typename Fn, typename A, typename B = ...>
    static F<B> apply(const F<Fn>& fn, const F<A>& f) {
        // ap is a free function part of the monad interface
        return ap(fn, f);
    }

    static constexpr bool instance = monad<F>::instance;
};

template<
        template<typename> class F,
        typename Fn,
        typename A,
        typename B = ...>
F<B> operator* (const F<Fn>& u, const F<A>& v) {
    return applicative<F>:::apply(u, v);
}
```
As you can see, all members of the applicative interface have default implementations relying on the [monad](Monad.md) concept. This is because just as all applicative functors are plain, regular functors, all monads are applicative functors. Thus, if a type is a monad, it is only necessary to implement the monad interface to get all three of functor, applicative, and monad.

ftl instances
-------------
The following standard library types have applicative instances defined:
* `std::vector<T>` and `std::list<T>` (details to follow)
* `std::tuple<T,Ts...>` (implementation description to come)
* `std::shared_ptr<T>`, using the following semantics:
  ```
  pure(some_val) = make_shared(some_val);

  map(f, some_val) = f(*some_ptr), if the pointer is valid, otherwise, an empty
                     pointer is returned.
  
  apply(fn_ptr, some_ptr) = (*fn_ptr)(*some_ptr), if and only if both pointers are
                            valid. Otherwise, an empty pointer.
  ```

In addition, the following ftl data structures are applicative functors:
* `maybe<T>`, in a manner isomorphic to `std::shared_ptr<T>`
* `either<L,R>` on L (details to follow)
* `function<R,Ps...>` on R (details to follow)

Custom instances
----------------
To make a type an instance of _applicative_, one can either simply make it an instance of _monad_ or specialise the struct given above. A minimal implementation is _pure_, _map_, _apply_, and the compile time constant _instance_. The static methods' signatures do not have to exactly match the ones given in the definition above, but they must be compatible. For example, you may want to pass by value in some case instead of by _const_ reference. You may also wish to overload one or more of the methods for rvalue reference, constness, etc. In particular, _pure_ can often be made both _constexpr_ and _noexcept_, as well as rvalue overloaded.

Examples
--------
### Example using [maybe](Maybe.md)
```cpp
#include <ftl/maybe.h>

int main(int argc, char** argv) {
    using namespace ftl;

    auto maybeFn = value([](int x){ return x/3; });
    auto mb = maybeFn * value(9);
    if(mb)
        std::cout << "value(" << *mb << ")" << std::endl;
    else
        std::cout << "nothing" << std::endl;

    return 0;
}
```
Output:
```
user@home:~/ftl_example$ ./ex
value(3)
```

### A more generalised example
```cpp
int foo(int x, int y) {
    return x/2 + y/3;
}

template<template<typename> class F, typename Fn>
F<int> baz(Fn f, F<int> a, F<int> b) {
    using ftl::operator%;    // Equivalent to functor<F>::map
    using ftl::operator*;    // Equivalent to applicative<F>::apply

    // Haskellers should feel quite comfortable with this, just substitute
    // % for <$> and * for <*>
    return f % a * b;
}

int main(int argc, char** argv) {
    using ftl::curry;
    using ftl::value;
    auto foobar = curry(foo);    // See http://en.wikipedia.org/wiki/Currying
                                 // for an explanation.

    // While we again use maybe as example (because it's quite easy to
    // understand), we could've used *any* applicative here, really.
    auto maybeAnswer = baz(foobar, value(6), value(9));
    if(maybeAnswer)
        std::cout << *maybeAnswer << std::endl;

    return 0;
}
```
Output:
```
user@home:~/ftl_example$ ./ex
6
```
