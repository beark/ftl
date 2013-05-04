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
std::future<int> ei = eventuallInt();
std::future<float> ef = eventuallyFloat();
std::future<object> eo = eventuallyObject();

result_type result = computeAlotOfStuff(ei.get(), ef.get(), eo.get());
```
Not too bad, really. But what if one of the _eventually_ computations takes a long while? Well, we have to wait for that to complete, obviously. But what if we're not even sure we _need_ the result as soon as possible? What if we simply want another `future` instead, so all the inputs have a chance to complete while we do other stuff, and we are only forced to wait when we really do need `result`? Well, we could always rewrite `computeAlotOfStuff` to take futures as input and spit out a new one. But that takes effort, and it might be a library function too. Right, so let's wrap it in an `async` call, then. Sure, that's possible, but it _also_ takes effort and adds cruft to our code. Let's just use future's [applicative](docs/Applicative.md) instance instead.
`` cpp
auto result = curry(computeAlotOfStuff) % ei * ef * eo;
```
This might look a bit strange, but once you've learnt applicative style programming, this is actually just as clear as `computeAlotOfStuff(ei.get(), ef.get(), eo.get())`, except its even slightly cleaner (less noise with all the `get`s and parens). It helps a bit to read this if you ignore `curry` for now, and then view `operator%` as an opening parenthesis, `operator*` as comma, and then insert a closing parenthesis at the end of the expression. I.e., like this `computeAlotOfStuff(ei, ef, eo);`. This is not really what happens of course, but conceptually it should make it easier to understand what's happening in applicative code.

The _really_ neat thing about this is that FTL doesn't actually concern itself with concurrent or asynchronous programming. What we mean is, you might expect a library that is _about_ asynchronous or lazy computations to give you such nice and concise ways of composing asynchronous computations, but in FTL, it is merely one _minor side effect_ of what it actually does: provides C++ with the same, incredibly powerful abstractions as Haskellers have been using for years.

