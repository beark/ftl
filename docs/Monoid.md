Monoid
======

Formal definition
-----------------------
Monoids are a concept from abstract algebra, and are defined as a set, _S_, together with a binary operation, _•_, for which the following three laws must hold:

* __Closure__

  For all _a_, _b_ in _S_, the result of `a • b` is also in _S_

* __Associativity__

  For all _a_, _b_, and _c_ in _S_, the equation `(a • b) • c = a • (b • c)` is satisfied.

* __Identity__

  There exists an element _id_ in _S_ such that `a • id = id • a = a` holds, for any element _a_ in _S_.

ftl definition
--------------
In ftl, the above definition is implemented as a templated ```struct monoid{};```, which must be specialised for any type that is to model a monoid. In other words, the 'set' in the mathematical definition is substituted for a type in ftl. Further, due to C++ only having a limited set of operators available for overload, the associated binary operation is presently either expressed using `monoid<some_type>::append(a, b)` (borrowing from Haskell's name for the monoid operation), or by using `operator^`. These two options are equivalent, and the latter is in fact implemented on top of the former.

The identity element is given by the same specialisation as mentioned above, using `monoid<some_type>::id()`.

In addition, ftl provides compile time constants to check if a particular type is an instance of its various concepts. In the case of monoid, it is simply encapsulated in the same interface struct as the rest of the monoid-belonging things.

ftl's monoid API may thus be described as:
```cpp
template<typename M>
struct monoid {
    static M id();
    static M append(M,M);
    static constexpr bool instance;
};

// Only actually enabled if M is a monoid, using the above compile time constant.
template<typename M>
M operator^(M a, M b) {
    return monoid<M>::append(a, b);
}
```

As for the three laws, there is currently no way of enforcing them in a neat and clean way in standard C++, and it is thus possible for any type to claim to be a monoid simply by specialising the monoid struct. All of the instances provided by ftl follow the laws, however.

ftl instances
-------------
The following primitive and standard types have predefined monoid instances in ftl:
* All primitive integer and floating point types, using either of the two thin wrappers `ftl::sum_monoid<T>` and `ftl::prod_monoid<T>`. The former makes the inner type a monoid by using _0_ as the identity element and _+_ as the associated operation, while the latter uses _1_ and _*_.
* For booleans there are also two thin wrappers: `ftl::any` and `ftl::all`. These are defined as using _false_ combined with _||_, and _true_ combined with _&&_, respectively.
* `std::vector<T>` and, isomorphically, `std::list<T>` are both monoids for any _T_. Identity is the empty container, while concatenation is the monoid operation.
* For all monoids _M_, ```std::shared_ptr<M>``` is also a monoid, by using an empty pointer as the identity and the following as monoid operation:
  ```
  a • b <=>
  if(a) {
      if(b) 
           return make_shared(*a • *b);
      else
           return a;
  }
  else {
      if(b)
          return b;
      else
          return shared_ptr<M>();
  }
  ```
* For all monoids _M_ and any parameter pack _Parameters_, `std::function<M(Parameters...)>` is a monoid. This by using a function that always returns `monoid<M>::id()` as identity, and by using
  ```
  (f1 • f2)(params...) <=> f1(params...) • f2(params...)
  ```
  as basis for the monoid operation.
* For all monoids _M1_, _M2_, ..., _Mn_, `std::tuple<M1, M2, ..., Mn>`, is a monoid by simply applying element-wise identity or monoid operations as apropriate.

In addition to the above, the following ftl-defined types have monoid instances:
* `ftl::ord` is a monoid, using `ftl::ord::Eq` as identity, and the following for the binary operation:
  ```
  a • b <=>
  if(a == ord::Lt || a == ord::Gt)
      return a;
  else
      return b;
  ```
* For all monoids _M_, `ftl::maybe<M>`, in a way isomorphic to how `std::shared_ptr` is a monoid.
* For all monoids _M_ and any parameter pack `Parameters`, `ftl::function<M,Parameters...>` equivalently to `std::function<M(Parameters...)>`

Custom instances
----------------
To make a new type into a monoid, simply specialise the `monoid` struct using your own type and implement the required minimal interface, as shown here:
```
template<>
struct monoid<my_type> {
    static my_type id() {
        return ...;
    }

    static my_type append(my_type a, my_type b) {
        return ...;
    }

    static constexpr bool instance = true;
};
```
Making one or both of the functions `constexpr` or `noexcept` is encouraged, if `my_type` allows it. Further, making _append_ take `const&` references is entirely possible, should that make more sense. In fact, you're free to provide more overloads should you desire, for instance to allow move semantics and such. The above is merely a minimal complete implementation, stripped as bare as possible. As such, the final `constexpr bool` is _not_ optional. It is used in many places, for instance in `ftl::operator^` to make sure SFINAE discounts it on types that are not monoids.

Examples
--------
### Demonstrating maybe's monoid instance.
```cpp
#include <ftl/maybe.h>

template<typename M>
M foo(M m1, M m2, M m3) {
    using ftl::operator^;
    return m1 ^ m2 ^ m3;
}

int main(int argc, char** argv) {
    using ftl::sum;
    using ftl::value;

    // 'sum' constructs a value of type sum_monoid. To use prod_monoid, there
    // is the 'prod' constructor. Point being to allow the inner type to be
    // automatically inferred, same as with maybe's 'value' constructor.
    auto x = value(sum(2));
    auto y = value(sum(3));
   
    // nothing is maybe's identity element, so z won't actually do anything
    // below, except demonstrate the fact that it does nothing.
    auto z = ftl::maybe<ftl::sum_monoid<int>>(); 

    // This essentially expands to
    // value(sum(2)) ^ value(sum(3)) ^ nothing
    // which in turn expands to
    // value(sum(2) ^ sum(3)) => value(sum(2+3))
    auto result = foo(x, y, z);
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
### An example using the monoid instance of [ftl::ord](Ord.md) and [ftl::function](Function.md):
```cpp
#include <vector>
#include <string>
#include <ftl/ord.h>
#include <ftl/functional.h>

int main(int argc, char** argv) {
    using ftl;
    using std::string;

    std::vector<string> v{ "these","words","will","be","sorted","by","length",
                           "first","and","then","lexicographically"};

    // lessThan, comparing and getComparator are convenience functions provided
    // by ord.h specifically to ease integration with stdlib's sort and
    // similar.
    std::sort(v.begin(), v.end(),
        lessThan(comparing(&string::size) ^ getComparator<string>()));

    for(auto& e : v) {
        std::cout << e << ", ";
    }
    std::cout << std::endl;
}
```
Output:
```
user@home:~/ftl_example$ ./ex
be, by, and, then, will, first, these, words, length, sorted, lexicographically,
```
Confused by how this works? Well, ```ftl::comparing``` takes as argument a "getter" for some member and returns a function that orders two objects based on the value of this member (in this particular case, it returns an ```ftl::function<ftl::ord,const std::string&, const std::string&>```). Then, ```ftl::getComparator<string>``` returns the standard compare function for strings, which simply orders them based on their own <,>,== operators. Because ```ftl::ord``` is a monoid, any function returning an ordering is _also_ a monoid, and hence we can simply combine these two ordering functions with the monoid operator. Thus, the result of ```comparing(&string::size) ^ getComparator<string>()``` is a function that orders two strings, first by their size, and then by string's built-in operators. Finally, ```ftl::lessThan``` creates a function that returns true if the ordering function given as argument orders the left hand side as "smaller" than the right hand side.
