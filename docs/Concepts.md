Concepts
========
ftl makes heavy use of concepts, even though they are technically not yet a part of the C++ standard. Therefore, while each concept is formally defined, there is no way of enforcing a type to actually follow them. Obviously, supplying a type that does _not_ conform to a required concept to a templated function may result in all manner of obscure and long-winded template errors. There is a reason any function claiming to require concept _X_ does so.

Basic concepts
--------------
These are low-level concepts mostly taken right out of the standard library. Most of them should require no explanation as their name says it all, and most should be familiar from the standard library.

### Relating to construction
* DefaultConstructible
* CopyConstructible
* MoveConstructible
* FullyConstructible (implies _all_ of the above)

### Relating to assignment
* CopyAssignable
* MoveAssignable
* Assignable (implies both of the above)

### Relating to comparison and ordering
* EqComparable (`operator==` and `operator!=` are implemented)
* Orderable (supports all of the comparison operators: _<_, _<=_, _==_, _>=_, and _>_)

### Miscellaneous concepts
* Swappable (works with `std::swap`)

Higher level concepts
---------------------
These are concepts whose characteristics are a bit more advanced and require further explanation. Most of these should be quite familiar to functional programmers, particularly Haskellers.

* [Monoid](Monoid.md)
* [Functor](Functor.md)
* [Applicative Functor](Applicative.md)
* [Monad](Monad.md)

