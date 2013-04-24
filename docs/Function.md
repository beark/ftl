Function
========
You might wonder why ftl provides its own function data type. After all, the standard library already provides `std::function`, which `ftl::function` is indeed very similar to&mdash;almost identical, in fact. What, then, is the justification for this data type?

Simple, the way it accepts template parameters. To provide as full a Haskell-like library as possible, it was desired that functions could act as [functors](Functor.md), [monoids](Monoid.md), and all sorts of other concepts. While possible to an extent with `std::function`, truly generalised functor code, for example, was _not_. Consider:
```cpp
template<template<typename> class F, typename...Ts>
F<float,Ts...> intToFloat(const F<int,Ts...>& f) {
    auto conv = [](int x){ return float(x); };
    return conv % f;
}
```
This function would work on _any_ functor containing an _int_ (and an unknown and arbitrary amount of other types&mdash;parameters in the case of functions), and return one of the same value, but now a _float_, while preserving everything else. It is not possible to unify the above type signature with `std::function`'s. It works excellently with `ftl::function` however.

For everything else but the template parameters, `ftl::function` is essentially identical to `std::function`, and you may simply look that up on cppreference.com or similar.

Concepts implemented
--------------------
### Low-level concepts
* FullyConstructible
* Assignable
* Swappable
* Callable

### High-level concepts
* [Monoid](Monoid.md)
* [Functor](Functor.md)
* [Applicative](Applicative.md)

Memory
------
### Stack space
Depends on architecture. Enough space for one pointer and 16 bytes.

### Heap space
Depends on callable object encapsulated. Raw function pointers, method pointers, and function objects which fit in 16 bytes result in no heap space. Larger function objects are allocated on the heap.

Examples
--------
### Storing and calling a function
```cpp
int foo(int x, int y);

void example() {
    function<int,int,int> f = foo;

    std::cout << f(1, 2) << std::endl;
}
```

### Some higher-order usage
```cpp
function<float,float> foo(function<int,int> f) {
    return [f] (float x) {
        float y = f(int(x));
        return y/3.f;
    };
}
```

