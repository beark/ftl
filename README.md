ftl - The Functional Template Library
=====================================

C++ template library for fans of functional programming. The goal of this project is to implement a useful subset of the Haskell Prelude (and a couple of other libraries) in C++. To date, this subset is very minimal, but there are plans to expand.

Either
------
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

Functor
-------
Functor is a concept (as in the ones that might yet make it in C++14), or in Haskell terms, a type class. For a type to be considered a Functor, it really only needs to implement one function: `fmap`. The purpose of `fmap` is to map a function to some inner type contained in the type that is a Functor. For example, for `either<L,R>`, `fmap` maps the function to the `L` (but only if the either instance is "left"). After applying the function to the inner value, `fmap` should return a a new instance of the original functor, now containing whatever the mapped function returned. Perhaps this is better shown in an example:
```cpp
void foo() {
    // x will be maybe<int>
    auto x = ftl::value(12);

    // y will be maybe<float>
    auto y = ftl::fmap([] (int x) { return float(x) / 2.f; }, x);

    // z is nothing, of type maybe<std::string>
    auto z = ftl::maybe<std::string>();

    // w is also nothing, but of type maybe<int>
    // The lambda is never actually called.
    auto w = fmap([] (const std::string& s) { std::cout << s << std::endl; return 0; }, z);
}
```
The point of this is that it allows us to generalise functions to apply to any Functor. Consider the fact that at the point of invocation, you don't have to know what type you're `fmap`ing on, you only need to know that it is a Functor. We can thus write incredibly generalised functions like:
```cpp
template<template <typename, typename...> class F, typename...Rem>
F<float, Rem...> example(const F<int, Rem...>& f) {
    /* Assuming F is a Functor, we know it's a Functor on an int, and we claim
     * to return something quite similar, but that is a functor on a float.
     * Even though we have no idea what exactly F is, we can access its
     * contained value and do stuff with it, like simply converting it to a
     * float: */
    return ftl::fmap([] (int x) { return (float)x; }, f);
}
```

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
A monoid is, in layman's terms, any set of things for which there is an associated binary operation, and an identity element. In addition, the operation and identity must conform to the following laws:
```
a <> identity = a
identity <> a = a
a <> (b <> c) = (a <> b) <> c
(a is any element in the set encompassing the monoid, and <> is the associated binary operation)
```
Using the above definition, we can easily construct our own monoids out of sets of things. Take for instance the set that is all numbers. If we use 0 as identity and + as the operation, then all of the laws are obeyed, because
```
a + 0 = 0 + a = a
a + (b + c) = (a + b) + c
```
Numbers can also be seen as a monoid if we use 1 for identity and * for the operation.

A couple of monoids included in the library:
* Numbers, as either of the wrappers ```ftl::sum``` (using 0 and +), or ```ftl::prod``` (using 1 and *)
* Booleans, as either of the wrappers ```ftl::any``` (using false and logical OR), or ```ftl::all``` (using true and logical AND)
* Lists, using the empty list and concatenation
* ```ftl::ord```
* ```std::tuple<Ts...>```, if every T in Ts is a monoid 
* ```ftl::maybe<T>```, if ```T``` is a monoid.
* Any ```std::function<M(Ts...)>``` where M is a monoid. This is where things get interesting.

A few examples of actually using monoids:
```cpp
#include <ftl/monoid.h>
#include <ftl/list.h>	// Used instead of <list> when ftl-features are desired

// Note that this function will work on any type that can act as a monoid.
template<typename T>
T monoidExample(T m1, T m2) {
    // This operator is provided for convenience only, you do not have to bring
    // it into scope unless you want to. The alternative is to use
    // ftl::monoid<T>::append(m1, m2)
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
An example using the monoid instance of ftl::ord and std::function:
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
Confused by how this works? Well, ```ftl::comparing``` takes as argument a "getter" for some member and returns a function that orders two objects based on the value of this member (in this particular case, it returns an ```std::function<ftl::ord(const std::string&, const std::string&)>```). Then, ```ftl::getComparator<string>``` returns the standard compare function for strings, which simply orders them based on its own <,>,== operators. Because ```ftl::ord``` is a monoid, any function returning and ordering is _also_ a monoid, and hence we can simply combine these two ordering functions with the monoid operator. Thus, the result of ```comparing(&string::size) ^ getComparator<string>()``` is a function that orders two strings, first by their size, and then by string's built-in operators. Finally, ```ftl::lessThan``` creates a function that returns true if the ordering function given as argument orders the left hand side as "smaller" than the right hand side.
