FTL - The Functional Template Library
=====================================

C++ template library for fans of functional programming. The goal of this project is to implement a useful subset of the Haskell Prelude (and a couple of other libraries) in C++. Presently, this subset is small, but rapidly growing. Note, however, that the library and its API are still in heavy flux and the interface of any data type or concept may yet change without notice.

To use the FTL, you need a compiler that implements at least as much of C++11 as gcc-4.8. As of this time, known compatible compilers include&mdash;obviously&mdash;gcc 4.8 and clang 3.2 with libc++. Unfortunately clang with libstdc++ does not appear to work. While it would be a lovely thing, MSVC is not currently compatible, including the recent 2013 CTP.

The full API reference can be found [here](http://libftl.org/api/).

Tutorials
---------
* [Parser Combinator Part I: Simple Parser](docs/Parsec-I.md)
* [Parser Combinator Part II: Parser Generator Library](docs/Parsec-II.md)

Showcases
---------
A couple of quick showcases of some rather neat things the FTL gives you.

### Curried Function Calling
One of the typically functional conventions brought to C++ by FTL is support for curried functions and function objects. A curried function is an n-ary function that can be invoked one argument at a time; each step returning a _new_ function object of arity _n-1_. Once enough parameters have been applied (when _n_ reaches _0_), the actual computation is performed and the result is returned. One of the uses of this is to achieve very convenient partial function application. For example:
```cpp
auto plus = ftl::curry(std::plus<int>);
auto addOne = plus(1);

auto x = addOne(2); // x = 3
auto y = addOne(x); // y = 4
```
As mentioned, all of the function objects provided by FTL are curried by default and do not require an `ftl::curry` call first. Partial application is thus in many cases extremely concise and clean. Note, however, that without, for instance, a wrapping lambda function, it is not possible to partially apply parameters other than in the exact order they appear. For example, the following are all valid:
```cpp
auto f = curriedTernaryFn(1);
auto g = curriedTernaryFn(1,2);

f(2)(3) == f(2,3) && f(2,3) == g(3); // true
```
But it is not possible to "skip" a parameter or leave a placeholder, as in:
```cpp
using std::placeholders::_1;
auto f = std::bind(ternaryFn, _1, 2, 3);
```
Currying by itself is a very nice thing to have, but what _truly_ makes it shine is when used in combination with e.g. higher order functions. See for instance applicative style coding, which is not nearly as nice without currying.

### Expanding The Standard Library
One of the nice things about FTL is that it does not try to replace or supercede the standard library, it tries to _expand_ it when possible. These expansions include giving existing types concept instances for e.g. Functor, Monad, Monoid, and others. For example, in FTL, `std::shared_ptr` is a monad. This means we can sequence a series of operations working on shared pointers without ever having to explicitly check for validity&mdash;while still being assured there are no attempts to access an invalid pointer.

For example, given
```cpp
shared_ptr<a> foo();
shared_ptr<b> bar(a);
```

We can simply write
```cpp
shared_ptr<b> ptr = foo() >>= bar;
```

Instead of
```cpp
shared_ptr<b> ptr(nullptr);
auto ptra = foo();
if(ptra) {
    ptr = bar(*ptra);
}
```

Which would be the equivalent FTL-less version of the above.

Monadic code may perhaps often look strange if you're not used to all the operators, but once you've got that, reading it becomes amazingly easy and clear. `operator>>=` above is used to sequence two monadic computations, where the second is dependant on the result of the first. Exactly what it does varies with monad instance, but in the case of `shared_ptr`, it essentially performs `nullptr` checks and either aborts the expression (returning a `nullptr` initialised `shared_ptr`), or simply passes the valid result forward.

Other types that have been similarly endowed with new powers include: `std::future`, `std::list`, `std::vector`, `std::forward_list`, `std::map`, `std::unordered_map`, `std::set`, and more.

### Applying Applicatives
Adding a bit of the Applicative concept to the mix, we can do some quite concise calculations. Now, if we are given:
```cpp
int algorithm(int, int, int);
ftl::maybe<int> maybeGetAValue();
ftl::maybe<int> maybeGetAnother();
ftl::maybe<int> maybeGetAThird();
```
where `ftl::maybe` is a type similar to e.g. `boost::optional`, provided by FTL (mostly to make sure there are no external dependencies save the standard library).

Then we can compute:
```cpp
/* ftl::operator% is short for ftl::fmap, basically the classic "map" function
 * of functional programming.
 * Similarly, ftl::operator* is short for ftl::aapply, the work horse function of
 * applicative programming style. It basically applies a function (the left hand
 * side) to one argument at a time (the right hand side).
 */
using ftl::operator%;
using ftl::operator*;
auto result = ftl::curry(algorithm) % maybeGetAValue() * maybeGetAnother() * maybeGetAThird();
```
which woould compute the result of algorithm, but only if every one of the `maybe` functions returned a value.

In other words, without Functor's `fmap` and Applicative's `aapply`, it would have looked something like:
```cpp
ftl::maybe<int> result;
auto x = maybeGetAValue(), y = maybeGetAnother(), z = maybeGetAThird();
if(x && y && z) {
    result = ftl::value(algorithm(*x, *y, *z));
}
```
If `algorithm` had happened to be wrapped in an `ftl::function`, or else be one of the built-in, curried-by-default function objects of FTL, then the `curry` call could have been elided for even cleaner code.

Exactly what operation is done by `apply` varies from type to type. For example, with containers, it generally implies combining their elements in every possible combination. Thus
```cpp
curry(std::plus<int>) % std::list<int>{1,2} * std::list<int>{5,10};
```
can be read as "for each element in `{1,2}`, combine it using `std::plus<int>` with each element in `{5,10}`", resulting in the list `{6, 11, 7, 12}`.

Of course, what happens on a more technical level is that `std::plus<int>` is partially applied to each element in the first list, resulting in a list of unary functions, that in turn gets applied to each element in the second list. A much less technical way of thinking&mdash;to gain some intuition for how applciative expressions behave&mdash;could be that `operator%` is an alias for function application, and `operator*` separates parameters. Except the parameters happen to be wrapped in some "context" or "container".

This type of function application scales to arbitrary arity.

### Transformers
No, not as in Optimus Prime! As in a monad transformer: a type transformer that takes one monad as parameter and "magically" adds functionality to it in the form of one of many other monads. For example, let's say you want to add the functionality of the `maybe` monad to the list monad. You'd have to create a new type that combines the powers, then write all of those crazy monad instances and whatnot, right? Wrong!

```cpp
template<typename T>
using listM = ftl::maybeT<std::list<T>>;
```
Bam! All the powers of lists and `maybe`s in one! What exactly does that mean though? Well, let's see if we can get an intuition for it.

```cpp
// With the inplace tag, we can call list's constructor directly
listM<int> ms{ftl::inplace_tag(), ftl::value(1), ftl::maybe<int>{}, ftl::value(2)};

// Kind of useless, but demonstrates what's going on
for(auto m : *ms) {
    if(m)
        std::cout << *m << ", ";
    else
        std::cout << "nothing, ";
}
std::cout << std::endl;
```
If run, the above would produce:
```
1, nothing, 2, 
```
So, pretty much a list of `maybe`s then, what's the point? The point is, the new type `listM` is a monad, in pretty much the same way as `std::list` is, _except_ we can apply, bind, and map functions that work with `T`s. That is, given the above list, `ms`, we can do:

```cpp
auto ns = [](int x){ return x-1; } % ms;

// Let's say this invokes the same print loop as before
print(ns);
``` 
Same deal, but if `ms` was a regular, untransformed list of `maybe`:
```cpp
auto ns = [](maybe<int> x){ return x ? maybe<int>((*x)-1) : return x; } % ms;

print(ns);
``` 
Output (in both cases):
```
0, nothing, 1, 
```
So, basically, this saves us the trouble of having to check for nothingness in the elements of `ns` (coincidentally&mdash;or not&mdash;exactly what the maybe monad does: allow us to elide lots of ifs). 

Right, this is kinda neat, but not really all that exciting yet. The excitement comes when we stop to think a bit before we just arbitrarily throw together a couple of monads. For instance, check out the magic of the `either`-transformer on top of the `function` monad in the parser generator tutorial [part 2](docs/Parsec-II.md).

### Yet More Concepts
In addition to the above concepts, FTL contains several more of the classic Haskell type classes: Monoids, Foldables, Zippables, and Orderables. These all express nice and abstracted interfaces that apply to many types, both built-in, standard library ones, and user defined ones (assuming one adds a concept instance). For instance, the code below uses the fact that an ordering is a Monoid to sort a vector of `MyType` by first one of its properties, then another.
```cpp
using namespace ftl;
std::sort(vec.begin(), vec.end(),
    asc(comparing(&MyType::someProperty) ^ comparing(&MyType::anotherProperty))
);
```
The equivalent version without FTL might look a bit like this:
```cpp
std::sort(vec.begin(), vec.end(), [](const MyType& lhs, const MyType& rhs){
    return lhs.someProperty() == rhs.someProperty()
        ? lhs.anotherProperty() < rhs.anotherProperty()
        : lhs.someProperty() < rhs.someProperty();
});
```
Personally, I find the former a lot cleaner and easier to read. The sort order is explicitly stated (`asc`), and then it just straight up says that we sort by comparing first `someProperty` and then `anotherProperty`. Fewer chances of subtle typos, less chance of screwing up the comparisons, much easier to extend, what's not to like?

