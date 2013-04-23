Either
======
The `either<L,R>` datatype is used when a parameter, return value, or variable may be _either_ one type _or_ another, but not both at once. 

Differences in ftl vs. Haskell
------------------------------
The most notable difference between the Haskell and ftl versions is that while in Haskell, Either is a Functor and Monad in its Right type, in ftl it's an instance in its Left type. This is because of various technical reasons explained elsewhere.

Requirements on _L_ and _R_
---------------------------
Both _L_ and _R_ must be _CopyConstructible_.

Concepts implemented
--------------------
### Low-level concepts
* CopyConstructible
* MoveConstructible (if both _L_ and _R_ are)
* Assignable (both Move and Copy)
* EqComparable

### High-level concepts
* [Functor](Functor.md)
* [Applicative](Applicative.md)
* [Monad](Monad.md)

Memory
------
### Stack space
Storage enough for the largest of _L_ and _R_, and one _bool_.

### Heap space
Exactly equivalent to _L_ if of left type, otherwise exactly equivalent to _R_.

Limitations
-----------
It is currently not possible to instantiate an _either_ where _L_ = _R_. In particular, this is because the constructors will have no idea if the assigned value should make it go left or right.

Examples
--------
### Construction and access
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
