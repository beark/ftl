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

