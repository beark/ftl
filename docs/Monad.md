Monad
=====
There are already more than enough tutorials on monads, comparing them to all sorts of bizarre things (conversations, burritos, elephants, ...). Here's a [bunch](http://www.haskell.org/haskellwiki/Monad_tutorials_timeline). Thus, this documentation will not bother defining or explaining them, except the laws they must follow as well as how they appear in FTL.

Laws
----
While there is no mechanism in place to enforce these laws, _all_ monads should follow these laws. Otherwise, generalised algorithms might make assumptions that don't hold.
### Left identity
`pure(a) >>= f` is equivalent to `f(a)`

### Right identity
`m >>= pure` is equivalent to `m`

### Associativity
`(m >>= f) >>= g` is equivalent to `m >>= [f,g](A a){ return f(a) >>= g; };`

### More on the laws
As is to be expected when dealing with concepts so heavily inspired from Haskell, the Haskellers themselves have produced [plenty](http://www.haskell.org/haskellwiki/Monad_Laws) of material available for those who want more than this documentation offers.

FTL definition
--------------
In FTL, the only real difference between a Monad and an [Applicative](Applicative.md), is that just like applicative functors add operations to plain ol' regular [functors](Functor.md), monads add one new basic operation&mdash;`bind`&mdash;and a couple of free standing other operations that in reality just use `bind`. Also, any type that implements the Monad concept gets functor and applicative for free, no catch. Anyway, here's how FTL defines the monad concept:
```cpp
template<template<typename> class M>
struct monad {
    template<typename A>
    static M<A> pure(const A&);

    template<typename F, typename A, typename B = ...>
    static M<B> map(F, const M<A>&);

    template<typename F, typename A, typename B = ...>
    static M<B> bind(const M<A>&, F);

    static constexpr bool instance;
};
```
So what's the difference between `map` and `bind`, then? Looks like they just had their parameters flipped, right? Well, because "functions" in C++ can be absolutely anything and everything that defines an `operator()`, these type signatures do not actually expose correctly what _F_ above must conform to. For `map`, _F_ is required to be a function from _A_ to _B_ (and the _B_ is actually properly inferred in all FTL-instances of Monad), whereas in `bind`, _F_ is a function form _A_ to _M&lt;B&gt;_ (and again, FTL correctly derives _B_ in all its monad instances).

In addition to the above interface definition, the following free functions and operators are defined in `<ftl/monad.h>`:
```cpp
/* This is not actually the complete definition, there is also some TMP to 
 * "hide" this operator for types that aren't monad instances.
 */
template<
        template<typename> class M,
        typename F,
        typename A>
auto operator >>= (const M<A>& m, F f) -> decltype(monad<M>::bind(m,f)) {
    return monad<M>::bind(m, f);
}

/* Binds m1 with a function that discards its element and returns m2
 * Useful when we don't care about the result of m1, but we *do* care about
 * the side effects of M's bind.
 */
template<
        template<typename> class M,
        typename A,
        typename B>
M<B> operator >> (const M<A>& m1, const M<B>& m2);

template<
        template<typename> class M,
        typename A,
        typename F,
        typename B = typename result_of<F(A)>::type>
M<B> ap(M<F>, const M<A>&);
```
The last one, `ap`, is actually [applicative's](Applicative.md) `apply` in disguise, defined in terms of monad's `bind`. This is how all monads are also applicative functors.

FTL instances
-------------
The following standard library types are instances of Monad:
* `std::shared_ptr<T>`.
* `std::future<T>`

Further, the following FTL-native types implement this concept:
* The type aliases `ftl::list<T>` and `ftl::vector<T>`.
* `maybe<T>`
* `either<L,_>`

Custom instances
----------------
Perhaps the easiest way to show how to make a type an instance is to simply show an existing instance. Here is [maybe](Maybe.md)'s instance:
```cpp
template<>
struct monad<maybe> {
    template<typename A>
    static constexpr maybe<A> pure(const A& a)
    noexcept(std::is_nothrow_copy_constructible<A>::value) {
        return value(a);
    }

    template<typename A>
    static constexpr maybe<A> pure(A&& a)
    noexcept(std::is_nothrow_move_constructible<A>::value) {
        return value(std::move(a));
    }

    template<
            typename F,
            typename A,
            typename B = typename decayed_result<F(A)>::type>
    static maybe<B> map(F f, maybe<A> m) {
        return m ? value(f(*m)) : maybe<B>();
    }
		

    template<
            typename F,
            typename A,
            typename B = typename decayed_result<F(A)>::type::value_type>
    static maybe<B> bind(const maybe<A>& m, F f) {
        return m ? f(*m) : maybe<B>::nothing();
    }

    static constexpr bool instance = true;
};
```
There is some noise in there with the _noexcept_ clauses and similar&mdash;that is all optional. But naturally, if you can guarantee _constexpr_ or _noexcept_, that is a good idea to do. The move overload of `pure` is also a sensible thing. For an explanation on `decayed_result`, see the [type level functions](TypeLevel.md)

In any case, aside from the above mentined things, `maybe`'s instance implementation is in fact minimal. If desirable, one could also overload `ap`, for instance, to provide a more performant version.

Examples
-------
### Maybe start simple?
```cpp
#include <string>
#include <ftl/maybe.h>

// Defined elsewhere
maybe<guid_t> getId(const std::string&);
maybe<my_obj> getObj(guid_t);
maybe<std::string> getDescription(const my_obj&);

int main(int argc, char** argv) {
    auto obj_desc =
        (getId("interesting.object") >>= getObj) >>= getDescription;

    if(obj_desc)
        std::cout << *obj_desc << std::endl;

    return 0;
}

```
Note how, despite performing a total of three actions that could fail (return _nothing_), we only had to check for nothingness once at the end. This is because `maybe`'s `bind` already does this for us. But the really nice part is that if we'd changed the interface to use `either` instad of `maybe` (using _right_ values to contain an error description or similar), the only lines in `main` that would change are the final two to print the object description.

