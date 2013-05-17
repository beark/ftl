Foldable
========
The foldable concept is an abstraction of data structures that can be folded in some manner, to get a summary value of some type. Typical examples are containers.

FTL definition
--------------
In FTL, the foldable concept takes the FTL-standard approach of defining a struct that instances must specialise to be considered as such. Technically, there are two such structs, but that is usually not important to know, because specialising them mostly looks the same anyway.

Foldable's API looks like this:
```cpp
	template<template<typename...> class F>
	struct foldable {
		template<
				typename M,
				typename = typename std::enable_if<monoid<M>::instance>::type,
				typename...Ts>
		static M fold(const F<M,Ts...>& f);

		template<
				typename A,
				typename Fn,
				typename M = typename std::result_of<Fn(A)>::type,
				typename = typename std::enable_if<monoid<M>::instance>::type,
				typename...Ts>
		static M foldMap(Fn&& fn, const F<A,Ts...>& f);

		template<
				typename Fn,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						B,
						typename decayed_result<Fn(A,B)>::type
						>::value
					>::type,
				typename...Ts>
		static B foldr(Fn&& fn, B&& z, const F<A,Ts...>& f);

		template<
				typename Fn,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						A,
						typename decayed_result<Fn(B,A)>::type
						>::value
					>::type,
				typename...Ts>
		static A foldl(Fn&& fn, A&& z, const F<B,Ts...>& f);

		static constexpr bool instance = false;
	};
```
Two of these have default implementations, available by inheriting `fold_default` and `foldMap_default`. Thus, the minimal complete instance is reduced to:
```cpp
	template<template<typename...> class F>
	struct foldable {
		template<
				typename Fn,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						B,
						typename decayed_result<Fn(A,B)>::type
						>::value
					>::type,
				typename...Ts>
		static B foldr(Fn&& fn, B&& z, const F<A,Ts...>& f);

		template<
				typename Fn,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						A,
						typename decayed_result<Fn(B,A)>::type
						>::value
					>::type,
				typename...Ts>
		static A foldl(Fn&& fn, A&& z, const F<B,Ts...>& f);

		static constexpr bool instance;
	};
```
Which is a bit more manageable.

FTL instances
-------------
The following standard types are instances of Foldable:
* `std::list`
* `std::vector`
* `std::shared_ptr`

The following FTL-defined types are also instances of Foldable:
* `ftl::list`
* `ftl::vector`
* `ftl::maybe`

Custom instances
----------------
Simply implement the minimal complete interface as given above. This would look e.g. like this:
```cpp
template<>
struct foldable<my_type> : foldMap_default<my_type>, fold_default<my_type> {
    template<
            typename Fn,
            typename A,
            typename B,
            typename = typename std::enable_if<
                std::is_same<
                    A,
                    typename decayed_result<Fn(B,A)>::type
                >::value
             >::type>
    static A foldl(Fn&& fn, A&& z, const my_type<B>& f) {
        // ...
    }

    template<
            typename Fn,
            typename A,
            typename B,
            typename = typename std::enable_if<
                std::is_same<
                    B,
                    typename decayed_result<Fn(A,B)>::type
                >::value
             >::type>
    static B foldr(Fn&& fn, B&& z, const my_type<A>& f) {
        // ...
    }

    static constexpr bool instance = true;
};
```
If specific examples are desired, consult the FTL source of any of the listed instances of Foldable.

Examples
--------
### Simple left fold
```cpp
#include <ftl/vector.h>

int main(int argc, char** argv) {
    using namespace ftl;
    
    vector<int> v{1,2,3,4,5};
    std::cout << foldr([](int x, int y){ return x+y; }, 0, v) << std::endl;

    return 0;
}

```
Output:
```
user@home:~/ftl_example$ ./ex
15
```
### Folding without zero using Monoid
If you find yourself with a container of some instance of Monoid, there is no need to provide an "initial" or "zero" value, you can simply exploit the fact that Monoid has its own: the identity element. For instance, the above example could just as well be written as:
```cpp
#include <ftl/vector.h>

int main(int argc, char** argv) {
    using namespace ftl;

    vector<int> v{1,2,3,4,5};
    auto v2 = sum<int> % v;		// Map sum function to each element of v,
                                // giving a vector<sum_monoid<int>>

    std::cout << foldable<vector>::fold(v2) << std::endl;

    return 0;
}
```

