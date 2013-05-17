Maybe
=====
The `maybe<T>` datatype simply implements the idea that you may have an optional function parameter, or a function that _maybe_ returns a value. It is very similar to Boost.Optional&mdash;and in certain syntactical aspects, gets its inspiration from there&mdash;but its true origin is the `Maybe` data type of Haskell. 

Differences in ftl vs. Haskell
------------------------------
There is no significant difference, save for what the language change introduces.

Requirements on _T_
-------------------
Must be _CopyConstructible_.

Concepts implemented
--------------------
### Low-level concepts
* DefaultConstructible
* CopyConstructible
* MoveConstructible (if _T_ is)
* Assignable (both Move and Copy)
* Dereferenceable

### High-level concepts
* Orderable
* [Monoid](Monoid.md)
* [Functor](Functor.md)
* [Applicative](Applicative.md)
* [Monad](Monad.md)
* [Foldable](Foldable.md)

Memory
------
### Stack space
Storage for _T_ and one _bool_.

### Heap space
None if _nothing_, exactly equivalent to what _T_ stores if valued.

Limitations
-----------
None in particular.

Examples
--------
### Construction
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
### Value access
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

