FTL - The Functional Template Library
=====================================

C++ template library for fans of functional programming. The goal of this project is to implement a useful subset of the Haskell Prelude (and a couple of other libraries) in C++. Presently, this subset is small, but rapidly growing. Note, however, that the library and its API are still in heavy flux and the interface of any data type or concept may yet change without notice.

To use the FTL, you need a compiler that implements at least as much of C++11 as gcc-4.7. Unfortunately, as of this time, that's essentially only gcc-4.7 and later, as clang&mdash;3.2 and later of which appears to handle everything else fine&mdash;does not allow template specialization on number of template parameters (e.g. the two versions of the functor interface/struct). MSVC is untested, but believed to be incompatible due to lack of C++11 support.

Documentation index
-------------------
### Concepts
* [Basic concepts](docs/Concepts.md)
* [Monoids](docs/Monoid.md)
* [Functors](docs/Functor.md)
* [Applicative Functors](docs/Applicative.md)
* [Monads](docs/Monad.md)

### Data types
* [function](docs/Function.md)
* [either](docs/Either.md)
* [maybe](docs/Maybe.md)
* [ord](docs/Ord.md)

### Examples & Tutorials
* [Parser Combinator Part I: Simple Parser](docs/Parsec-I.md)
* ...

Showcases
---------
A couple of quick showcases of some rather neat things the FTL gives you.

### Calculating futures
Let's say you have a couple of asynchronous function calls you want to make, and whose results you want to use in an algorithm of some kind. Normally, this might look something like
```cpp
// Assume these are declared somewhere and do something useful
std::future<int> eventuallyInt();
std::future<float> eventuallyFloat();
std::future<object> eventuallyObject();

// Then here's where we use those computations
result_type result = computeAlotOfStuff(
                         eventuallyInt().get(),
                         eventuallyFloat().get(),
                         eventuallyObject().get());
```
Not too bad, really. But what if one of the _eventually_ computations takes a long while to complete? Well, we have to wait for it to complete, obviously. But what if we're not even sure we need the result _right now_? What if we simply want another `future` instead, so that all the inputs have a chance to complete while we do other stuff, and so that we are only forced to wait when we really do _need_ `result`?

Well, we could always rewrite `computeAlotOfStuff` to take futures as input and spit out a new future as result. But that takes effort, and `computeAlotOfStuff` might be a library function too, and thus impossible to rewrite. Right, so let's wrap it in an `async` call, then. We can do that regardless of who defined it and where. Sure, but it still takes effort and adds cruft to our code.

Let's just use future's [applicative](docs/Applicative.md) instance instead.
```cpp
auto result = curry(computeAlotOfStuff) %
                  eventuallyInt() * eventuallyFloat() * eventuallyObject();
```
This might look a bit strange, but once you've learnt applicative style programming, this is actually just as clear as the plain, original function call that `get`ed on the futures. Except, this is even slightly cleaner (less noise with all the `get`s and parens gone). It helps a bit to read this if you ignore `curry` for now (or, you can read about it [here](http://en.wikipedia.org/wiki/Currying), and then view `operator%` as an opening parenthesis, `operator*` as comma, and then insert a closing parenthesis at the end of the expression. I.e., like this:
```cpp
computeAlotOfStuff(eventuallyInt(), eventuallyFloat(), eventuallyObject());
```
This is not really what happens of course, but conceptually it should make it easier to understand what's going on in applicative code.

Anyway, the _really_ neat thing about this is that FTL doesn't actually particularly concern itself with asynchronous programming. What we mean is, you might expect a library that is _about_ asynchronous or lazy computations to give you such nice and concise ways of composing asynchronous computations, but in FTL, it is merely one _minor side effect_ of what it actually does: provides C++ with the same, incredibly powerful abstractions as Haskellers have been using for years.

