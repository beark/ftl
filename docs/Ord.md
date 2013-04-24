Ord
===
The _ord_ data type encapsulates an ordering. It is most often used as an intermediate return value from a comparing operation of some kind, often to be passed directly to one of `lessThan`, `equal`, or similar.

The data type itself is trivial and hardly worthy of discussion; it is through the convenience functions that surround it that it becomes useful.

Concepts implemented
--------------------
### Low-level concepts
* FullyConstructible
* Assignable
* EqComparable
* Orderable

### High-level concepts
* [Monoid](Monoid.md)

Memory
------
### Stack space
Storage for a single _enum_.

### Heap space
None.

Examples
--------
### getComparator
The most basic use of _ord_ is as return value from a comparison, for which the comparator function returned by _getComparator_ is often used.
```cpp
auto cmp = getComparator<int>();
if(cmp(1,1) == ord::Eq) {
    std::cout << "1 == 1" << std::endl;
}
if(cmp(1,2) == ord::Lt) {
    std::cout << "1 < 2" << std::endl;
}
if(cmp(2,1) == ord::Gt) {
    std::cout << "2 > 1" << std::endl;
}
```

### lessThan, equal, greaterThan
Sometimes, we just want a boolean answer to "is X less than Y?", those are the times _lessThan_, _equal_, and _greaterThan_ are useful.
```cpp
// Sometimes, asking for a comparator is just too much trouble, for those times,
// there is compare
if(lessThan(compare(1,2)))
    std::cout << "1 < 2" << std::endl;
if(equal(compare(2,2)))
    std::cout << "2 == 2" << std::endl;
```

### comparing
Often, we do not wish to directly compare objects, but instead compare some property of them. That's where _comparing_ comes in.
```cpp
auto cmp = comparing(&std::string::size);
if(lessThan(cmp(std::string("a"), std::string("aa"))))
    std::cout << "'a' is shorter than 'aa'" << std::endl;
```
The second version of comparing uses free functions instead.
```cpp
struct myObj { ... };

int foo(myObj);

void example(const myObj& a, const myObj& b) {
    auto cmp = comparing(foo);
    if(cmp(a, b) == ord::Eq) {
        std::cout << "Hurray! Whatever ints foo returns for a and b are equal!";
        std::cout << std::endl;
    }
}
```

### True power
The real power of the _ord_ data type does not become apparent until we start to look at its Monoid instance, however. Especially in combination with _function_'s monoid instance. A good example of this can be found in the last example in the [monoid](Monoid.md) article.

