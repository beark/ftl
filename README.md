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

