FTL - The Functional Template Library
=====================================

C++ template library for fans of functional programming. The goal of this project is to implement a useful subset of the Haskell Prelude (and a couple of other libraries) in C++. Presently, this subset is small, but rapidly growing. Note, however, that the library and its API are still in heavy flux and the interface of any data type or concept may yet change without notice.

To use the FTL, you need a compiler that implements at least as much of C++11 as gcc-4.7. As of this time, that's more or less gcc-4.7 (and later), as well as clang-3.2 and later. Both of these have been tested for a number of uses of FTL. MSVC is untested, but believed to be incompatible due to lack of C++11 support.

The full API reference can be found [here](http://libftl.org/api/).

Tutorials
---------
* [Parser Combinator Part I: Simple Parser](docs/Parsec-I.md)
* [Parser Combinator Part II: Parser Generator Library](docs/Parsec-II.md)

Showcases
---------
A couple of quick showcases of some rather neat things the FTL gives you.

### Expanding The Standard Library
One of the nice things about FTL is that it does not try to replace or supercede the standard library, it tries to _expand_ it when possible. These expansions include giving existing types concept instances for e.g. [Monad](Monad.md), [Monoid](Monoid.md), and others. For example, in FTL, `std::shared_ptr` is a monad. This means we can sequence a series of operations working on shared pointers without ever having to explicitly check for validity&mdash;while still being assured there are no attempts to access an invalid pointer.

For example, given
```cpp
    shared_ptr<a> foo();
    shared_ptr<b> bar(a);
```

We can simply write
```cpp
    shared_ptr<b> ptr = foo() >>= bar;
```

Instead of
```cpp
    shared_ptr<b> ptr(nullptr);
    auto ptra = foo();
    if(ptra) {
        ptr = bar(*ptra);
    }
```

Which would be the equivalent FTL-less version of the above.

Monadic code may perhaps often look strange if you're not used to all the operators, but once you've got that, reading it becomes amazingly easy and clear. `operator>>=` above is used to sequence two monadic computations, where the second is dependant on the result of the first. Exactly what it does varies with monad instance, but in the case of `shared_ptr`, it essentially performs `nullptr` checks and either aborts the expression (returning a `nullptr` initialised `shared_ptr`), or simply passes the valid result forward.

Other types that have been similarly endowed with new powers include: `std::future`, `std::list`, `std::vector`, and `std::forward_list`.

### Applying Applicatives
Adding a bit of [Applicative](Applicative.md) to the mix, we can do some quite concise calculations. Now, if we are given:
```cpp
    int algorithm(int, int, int);
    shared_ptr<int> getSomeShared();
    shared_ptr<int> getOtherShared();
    shared_ptr<int> getFinalShared();
```

Then we can compute:
```cpp
    auto result = curry(algorithm) % getSomeShared() * getOtherShared() * getFinalShared();
```

And of course the equivalent plain version:
```cpp
    std::shared_ptr<int> result;
    auto x = getSomeShared(), y = getOtherShared(), z = getFinalShared();
    if(x && y && z) {
        result = make_shared(algorithm(*x, *y, *z));
    }
```

If `algorithm` had happened to already be wrapped in an `ftl::function`, then the FTL-version would have been even shorter, because the `curry` call could have been elided. `ftl::function` supports both conventional calls and curried calls out of the box. That is, if `f` is an `ftl::function<int,int,int,int>`, it could be called in any of the following ways `f(1,2,3)`, `f(1)(2,3)`, `f(1)(2)(3)`.

