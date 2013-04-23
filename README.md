ftl - The Functional Template Library
=====================================

C++ template library for fans of functional programming. The goal of this project is to implement a useful subset of the Haskell Prelude (and a couple of other libraries) in C++. Presently, this subset is small, but rapidly growing. Note, however, that the library and its API are still in heavy flux and the interface of any data type or concept may yet change without notice.

To use the ftl, you need a compiler that implements at least as much of C++11 as gcc-4.7. Recent releases of clang, for instance, should be fine, but have not been tested. MSVC has not been tested, but it seems highly unlikely that it'll work. In fact, as of the time of this writing, the library has only been compiled and tested using gcc-4.7 and gcc-4.8. Should someone wish try ftl with additional compilers, both reports of results and compatibility patches are welcome.

Table of contents
-----------------
### Concepts
* [Monoids](docs/Monoid.md)
* [Functors](docs/Functor.md)
* [Applicative Functors](docs/Applicative.md)
* Monads

### Data types
* function
* [maybe](docs/Maybe.md)
* [either](docs/Either.md)

### Include headers
* ftl/applicative.h
* ftl/either.h
* ftl/maybe.h
* ...

