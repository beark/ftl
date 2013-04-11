ftl - The Functional Template Library
=====================================

C++ template library for fans of functional programming. The goal of this project is to implement a useful subset of the Haskell Prelude (and a couple of other libraries) in C++. To date, this subset is very minimal, but there are plans to expand.

Maybe
-----
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
   ftl::fmap(
       [](const std::string& s) { std::cout << s << std::endl; return s},
       m);
}
```

Monoids
-------
A few examples of what the monoid-part of the library can be used for (in its present state):
```cpp
#include <ftl/monoid.h>
#include <ftl/list.h>	// Used instead of <list> when ftl-features are desired

/* Note that this function will work on any type that can act as a monoid.
   Default instances for monoid are:
   * Primitive number types, either as sums or products (using sum or prod,
     respectively)
   * Tuples where all member types are already monoids.
   * std::lists, regardless of contained type.
   * maybe<T>, if T is a monoid.
 */
template<typename T>
T monoidExample(T m1, T m2) {
    // This operator is provided for convenience only, you do not have to bring
    // it into scope unless you want to. The alternative is to use
    // ftl::monoid<T>::append
    using ftl::operator^;

    return m1 ^ m1 ^ m2 ^ m2;
}
    
int main(int argc, char** argv) {
    using ftl::sum;
    using ftl::prod;
    using std::list;

    // ftl::operator^ will act as +
    auto x = monoidExample(sum(1), sum(2));

    // ftl::operator^ will act as *
    auto y = monoidExample(prod(2), prod(3));

    // ftl::operator^ will perform concatenation
    auto z = monoidExample(list<int>{1,2}, list<int>{3,4});
   
    std::cout << x << std::endl;
    std::cout << y << std::endl;

    for(auto e : z) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
        
    return 0;
}
```
Output:
```
user@home:~/ftl_example$ ./ex
6
36
1 2 1 2 3 4 3 4
```
Another example, demonstrating the composability of monoids.
```cpp
#include <ftl/maybe.h>

template<typename M>
M compose(M m1, M m2, M m3) {
    using ftl::operator^;
    return m1 ^ m2 ^ m3;
}

int main(int argc, char** argv) {
    using ftl::sum;
    using ftl::value;

    auto x = value(sum(2));
    auto y = value(sum(3));
    auto z = ftl::maybe<ftl::sum_monoid<int>>::nothing();

    // Because maybe is a monoid if its value_type is, this works--and exactly
    // as you'd expect: Nothings are ignored, values are append:ed.
    // Hence, this is equivalent of value(sum(2 + 3))
    auto result = compose(x, y, z);
    if(result)
        std::cout << (int)*result << std::endl;

    return 0;
}
```
Output:
```
user@home:~/ftl_example$ ./ex
5
```

The significance of the above is perhaps not apparent yet, but as the library grows and more monoid implementations are added, the use of this abstraction should become clear. To get a preview of what will be available, read up on the Monoid type class in Haskell.

