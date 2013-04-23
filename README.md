ftl - The Functional Template Library
=====================================

C++ template library for fans of functional programming. The goal of this project is to implement a useful subset of the Haskell Prelude (and a couple of other libraries) in C++. Presently, this subset is small, but rapidly growing. Note, however, that the library and its API are still in heavy flux and the interface of any data type or concept may yet change without notice.

To use the ftl, you need a compiler that implements at least as much of C++11 as gcc-4.7. Recent releases of clang, for instance, should be fine, but have not been tested. MSVC has not been tested, but it seems highly unlikely that it'll work. In fact, as of the time of this writing, the library has only been compiled and tested using gcc-4.7 and gcc-4.8. Should someone wish try ftl with additional compilers, both reports of results and compatibility patches are welcome.

Table of contents
-----------------
### Concepts
* [Monoids](docs/Monoid.md)
* [Functors](docs/Functor.md)
* [Applicative Functors](#applicative-functors)
* Monads

### Data types
* function
* [maybe](#maybe)
* [either](#either)

### Include headers
* ftl/applicative.h
* ftl/either.h
* ftl/maybe.h

## Applicative Functors
Applicative functors are a subset of [functors](#functor), with a few additional operations available. Most notably, `apply` (or `operator*` if you don't mind bringing that into scope). So what does this `apply` thing do? In a way, it's not too dissimlar to Functor's `map`, except instead of a free function, it applies a function wrapped in the Applicative instance. The use case of this is simple: whenever you find yourself with a function you wish you could map on some Functor, but find that the function itself is wrapped in that same Functor, then you actually want Applicative's `apply`. If the exact type of the Functor is known, you can of course do this anyway, but abstracting the operation into Applicative can allow for more general code.

A quick example to show what `apply` does (see [Maybe](#maybe) for an explanation on the `value` function):
```cpp
#include <ftl/maybe.h>

int main(int argc, char** argv) {
    using namespace ftl;

    // ftl overloads operator* for applicatives, it is equivalent to
    // applicative<F>::apply(F1, F2);
    auto mb = value([](int x){ return x/3; }) * value(9);
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
Had either of the above `value`s been `nothing` instead, "nothing" would have been printed.

A slightly more involved example, showing off proper "applicative style" programming, as seen in Haskell.
```cpp
int foo(int x, int y) {
    return x/2 + y/3;
}

template<template<typename> class F, typename Fn>
F<int> baz(Fn f, F<int> a, F<int> b) {
    using ftl::operator%;    // Equivalent to functor<F>::map
    using ftl::operator*;    // Equivalent to applicative<F>::apply

    return f % a * b;
}

int main(int argc, char** argv) {
    using ftl::curry;
    using ftl::value;
    auto foobar = curry(foo);    // See http://en.wikipedia.org/wiki/Currying
                                 // for an explanation.

    // While we again use maybe as example (because it's quite easy to
    // understand), we could've used *any* applicative, really.
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
But what good is this? We could have accomplished very similar results using code like:
```cpp
if(a && b)
    return foo(*a, *b);
else
    return maybe<int>();
```
The answer is twofold:
* Every `apply` saves us from cluttering code with if-checks or equivalent (whatever the applicative action "hides" for us), which in larger algorithms can add up to a LOT of clutter. This also means it is impossible to forget about such checks, potentially saving time spent on searching for bugs.
* Writing functions in applicative style is far more generalised. The various actions (`map`, `apply`, etc) all serve as a common interface that work on a surprisingly large set of data types, and if we can limit ourselves to use only these, then suddenly a very large amount of functions become reusable for completely different data types. The Applicative Functor concept is a very powerful abstraction.

Notable instance of Applicative include:
* `maybe<T>`
* `either<L,R>`
* `std::tuple<T,Ts...>`
* `ftl::function<R,Ps...>`
* `std::shared_ptr<T>`

## Either
The `either<L,R>` datatype is used when a parameter, return value, or variable may be _either_ one type _or_ another, but never both. Interesting concepts implemented by `either<L,R>` include Functor and Monad.

One notable difference between the Haskell and ftl versions is that while in Haskell, Either is a Functor and Monad in its Right type, in ftl it's an instance in its Left type. This is because of various technical reasons to be explained elsewhere.

So, what does `either<L,R>` look like in use? Something like this:
```cpp
ftl::either<int, std::string> usingEitherToSignalError() {
    if(someErrorCondition()) {
        return ftl::either<int, std::string>("An error description");
    }
    else {
        return ftl::either<int, std::string>(someComputationYieldingInt());
    }
}

void checkingState(const ftl::either<typeA,typeB>& e) {
    if(e.isLeft()) {
        std::cout << e.left();
    }
    else {
        std::cout << e.right();
    }
}
```

## Maybe
The `maybe<T>` datatype simply implements the idea that you may have an optional function parameter, or a function that _maybe_ returns a value. It is very similar to Boost.Optional&mdash;and in certain syntactical aspects, gets its inspiration from there&mdash;but its true origin is the `Maybe` data type of Haskell. Similarly to Haskell's `Maybe`, `maybe<T>` implements a number of useful concepts (type classes in Haskell), such as `Monoid`, `Functor`, etc. These are not always _exactly_ the same in _ftl_ as in Haskell, but they're always founded on the same ideas and express the same abstractions (or as similar as is possible in C++).

On to some examples. The following shows how you can define a function that might return an integer, or it might return nothing.
```cpp
ftl::maybe<int> possiblyGetAnInt() {
    if(someCondition) {
        // ftl::value is a convenience function that constructs a maybe with the
        // inner type of its argument.
        return ftl::value(getAnInt());
    }

    else {
        // If you prefer, you may simply also use maybe's default c-tor, which
        // is equivalent.
        return ftl::maybe<int>::nothing();
    }
}
```
Here, we see how we can access the value of a `maybe<T>`:
```cpp
void foo(const maybe<std::string>& m) {
    // Use operator bool to check for nothingness
    if(m) {
        // Dereference operator to read or write the value
        std::cout << *m << std::endl;

        // Member access operator also works
        std::cout << m->size() << std::endl;
    }

   // Finally, we can use maybe's Functor instance to work with its value
   // The given lambda (or function object) is only invoked if the maybe
   // instance is a value, never if it's nothing.
   ftl::functor<maybe>::map(
       [](const std::string& s) { std::cout << s << std::endl; return s},
       m);
}
```

