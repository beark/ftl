ftl - The Functional Template Library
=====================================

C++ template library for fans of functional programming. The goal of this project is to implement a useful subset of the Haskell Prelude (and a couple of other libraries) in C++. To date, this subset is very minimal, but there are plans to expand.

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

The significance of the above is perhaps not apparent yet, but as the library grows and more monoid implementors are added, the use of this abstraction should become clear. To get a preview of what will be available, read up on the Monoid type class in Haskell.

