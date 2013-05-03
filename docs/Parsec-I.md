Parser Combinator Part I: Simple Parser
=======================================

In this tutorial, we will use a parser combinator library built on FTL to create a simple parser. For now, we disregard the implementation of the library itself, and focus on simply using its monadic interface to combine smaller, basic parsers into more advanced ones. [Part II](Parsec-II.md) concerns itself with how the library itself was built.

Chapter 1: Parser Combinator API
--------------------------------
Before we do anything, we should familiarise ourselves with the API we intend to use, that of the parser combinator library. If you wish, you can head straight to [source](../examples/parser_combinator/parser_combinator.h) and have a glance at that, otherwise a short description follows. If you do sift through the source, remember that this tutorial does not concern itself with the implementation details, so there is no need to look beyond declarations and public parts of data types.

On to the good stuff. A central part of the API is the type `parser<T>`, which denotes a parser capable of somehow extracting a _T_ from an input stream. How it does this, we do not know, all we know is that it consumes characters and produces something of the stated type. The direct interface to this type can be summarised as:
```cpp
/**
 * A parser of Ts.
 *
 * This is the central data type of the library.
 *
 * \par Concepts
 * \li Monad
 */
template<typename T>
class parser {
    ftl::either<T,error> run(std::istream&);
};
```
What's that? only one single, lonely little method? Something must surely be wrong? Well, let's give it a chance to prove itself at least...

So, what else does this interface tell us? First, apparently parsers are monads. This actually tells us quite a bit: we now know we can use all of the [functor](Functor.md), [applicative functor](Applicative.md), and [monad](Monad.md) interfaces to interact with parsers. Neat. Second, we also know that the `run` method apparently can fail: its return type of `either<T,error>` clearly indicates that it will either return whatever we wanted it to parse, or an error.

Alright, now we've deduced quite a bit about this `parser<T>` type, but we still can't do much, because we're missing a rather fundamental thing: how do we actually create parsers? Because looking at the interface, there are no public constructors! Let's browse further down the parser combinator API, and see if there is something there to enlighten us.
```cpp
/**
 * Combinator to try parsers in sequence.
 *
 * First tries to run \c p1, and if that fails, tries p2. If both fail, a parse
 * error is returned.
 */
template<typename T>
parser<T> operator|| (parser<T> p1, parser<T> p2);
```
Okay, that sounds useful, but it's just another combining operation (in addition to those provided by the monad instance). We still can't produce any parsers to actually combine. Let's keep going.
```cpp
/**
 * Parses any one character.
 *
 * This parser can only fail if the end of stream has been reached.
 */
parser<char> anyChar();

```
Perfect! This looks like exactly what we need: a function that returns an actual parser, without requiring one as input. It's not all that interesting though, it will only ever return whatever character happens to be next in the input stream. Let's see what else we have to play with (only an excerpt shown below, consult the [source](../examples/parser_combinator/parser_combinator.h) for more).
```cpp
/**
 * Parses one specific character.
 *
 * This parser will fail if the next character in the stream is not equal
 * to \c c.
 */
parser<char> parseChar(char c);

/**
 * Parses one of the characters in str.
 *
 * This parser will fail if the next character in the stream does not appear
 * in str.
 */
parser<char> oneOf(std::string str);

/**
 * Greedily parses 0 or more of p.
 *
 * This parser cannot fail. If end of stream is reached or p fails on the
 * first run, the result will be an empty string.
 */
parser<std::string> many(parser<char> p);
```
All of these are really quite basic, but already we can combine them into surprisingly advanced parsers using the combining functions we saw before.
```cpp
parser<char> sillyExample() {
    using ftl::operator>>=;
    return anyChar() >>= parseChar;
}
```
For instance, when run, the above parser will successfully parse any sequence of two characters in a row, and return whatever the character was. The parser will fail if the input stream's next two characters are not the same. I.e., the sequence "aa" will parse fine and return 'a', while the sequence "ab" will return an `error`. This works because the monadic `operator>>=` sequences two parsers and gives the result of the first as input to the second. Hence, if we were to invoke `sillyExample().run()`, first `anyChar().run()` would be invoked, and if its result is not an error, it is given as `parseChar(char)`'s argument, after which _that_ parser's `run` would be invoked and its result returned. Once you are more familiar with monads, all this would be essentially self-evident from the code above.

A second, slightly less contrived example:
```cpp
// Assume we already have this
int string2int(const std::string&);

// We can then easily parse natural numbers
parser<int> parseNatural() {
    using ftl::operator%;
    return string2int % many1(oneOf("0123456789"));
}
```
Here, we rely on yet another of the Functor series of operations, specifically the `map` function (in the form of `operator%` above). What it does is essentially to "lift" a plain old value level function into some context&mdash;in this case the context of 'parsers'. Or in other words, we make `string2int` accept a `parser<std::string>` instead of a plain `std::string` and return a `parser<int>` instead of a plain `int`. `many1` above is similar to the previously mentioned `many`, except that it must successfully parse at least _one_ of the given parsers, or a parse failure is returned.

Alright, we have covered enough to do something actually useful. Let's move on!

Chapter 2: Doing Something Non-Trivial
--------------------------------------
At this point, we have a library to build parsers and the knowledge how to use it, so what do we do? For this particular tutorial, we'll try to stay in the shallow end of the pool a bit longer, but still put together something that shouldn't tax us _too_ much. Let's make ourselves a parser that parses a lisp-list of integers to an `std::vector<int>`. This should provide us with a few challenges, yet not grow out of hand.

We can give it a go using a top-down approach. A list in lisp has an opening parenthesis, then a space separated list of elements, and then a closing parenthesis. Starting from that, we intuitively get:
```cpp
parser<std::vector<int>> parseLipList() {
    using namesapce ftl;
    return parseChar('(')
        >> parseList()
        << parseChar(')');
}
```
where `parseList`'s implementation is yet to be determined. How the new operators work should be quite intuitive, if you've read the public `monad` interface, but for easier reference:
* `operator>>` performs the given monadic computations in sequence, discarding the result of the first one. Its result is whatever the right hand side computes.
* `operator<<` is almost a mirror of the above, except it _also_ uses left-to-right sequencing. In other words, the left hand side is computed, then the right hand side, then the right result is discarded and the _left_ result is returned.

In our context of parsers, this simply means that `parseChar('(')` will run first, but its result ignored. Then will run our as of yet unimplemented `parseList()`, with its result somehow stored for us. Finally, `parseChar(')')` will run, with its result discarded in favour of whatever was left of it (which is of course `parseList`).

Okay, so we have our top level parser, time to descend. What should `parseList` do? Well, it should keep parsing whitespace separated numbers as long as there are numbers to find. It should also concatenate all these numbers into a single `std::vector`. Let's sketch this out:
```cpp
parser<std::vector<int>> parseList() {
    return cons % parseNatural() * (whitespace() >> parseList());
}
```
A few new things here that require explanation. `operator*` is from [applicative functors](Applicative.md), which we know parsers to be. What it does is to "unwrap" the function on its left hand side (`cons % parseInt()`), "unwrap" the contextual value on its right hand side (`whitespace() >> lazy(parseList)`), apply the function to the value, and finally re-wrap the result. But how is `cons % parseInt()` a function? Didn't we just recently see that `operator%`/`map` already applies a function to a context wrapped value? The only conclusion is that applying `cons` to an int must result in _another_ function. `cons` is _curried_!

Unfortunately, C++ does not give us automatically curried functions like in Haskell, so `cons` is not actually likely to be of the form we require, assuming we implement it as a normal function when we get to that. Fortunately, FTL provides us an easy means to curry functions: `curry`. Right, this gives us:
```cpp
parser<std::vector<int>> parseList() {
    return curry(cons) % parseNatural() * (whitespace() >> parseList());
}
```
However, looking closer at our "sketch", we notice a nasty thing at the end: we have an unconditional recursion. Like this, our parser will expect an infinitely long list of whitespace separated natural numbers. It seems we must somehow make the recursion conditional. We need the ability to _optionally_ parse something, or else go with some default value. Can we create something like that?
```cpp
template<typename T>
parser<T> option(parser<T> p, T t) {
	return p || ftl::monad<parser>::pure(t);
}
```
Easily! The OR-combinator we found earlier comes in very handy, and monad's `pure` gives us the final piece. See, it wasn't quite true what was said in Chapter 1&mdash;even without _anything_ save the monad instance, we _can_ create parsers. Only, they're rather useless on their own. Parsers creates with `pure` will consume no input and always yield the exact value we gave to `pure`. Our `option` above then quite simply reads as "parse p, but if that fails, just default to t".

Alright, we can return to our `parseList` again. It now looks like this:
```cpp
parser<std::vector<int>> parseList() {
    return curry(cons)
        % parseNatural()
        * option(whitespace() >> parseList(), std::vector<int>());
}
```
Hmm, we have another problem. While _running_ the parser will not unconditionally recurse, actually _creating_ it does. Every call to `parseList()` will call `parseList()` again. That's no good. But surely a parser must be able to recurse somehow? Scanning the parser generator API again, we find:
```cpp
/**
 * Lazily run the parser generated by f
 *
 * This is useful e.g. if you want a parser to recurse.
 */
template<typename T>
parser<T> lazy(ftl::function<parser<T>> f);

/// \overload
template<typename T>
parser<T> lazy(parser<T>(*f)());
```
There we go, exactly what we need. Applying to our own parser, we now have:
```cpp
parser<std::vector<int>> parseList() {
    return curry(cons)
        % parseNatural()
        * option(whitespace() >> lazy(parseList), std::vector<int>());
}
```
Finally, our "sketch" is no longer a sketch; we are ready to descend further. Now, `parseNatural()` we have since before, and `parseList` we just defined. So `whitespace()` and `cons(int, std::vector<int>)` are the only two unknowns. Turns out, `cons` is already in FTL, though genericised to `cons(T t, container<T>)`. Neat, only `whitespace()` left then.

After the previous exercise, it's essentially trivial:
```cpp
parser<std::string> whitespace() {
    return many1(oneOf(" \t\r\n"));
}
```
While we actually don't care at all what whitespace we actually parse, the parser generator library as it exists gives us no way of declaring a parser that consumes input without also producing _something_. Or in other words, we cannot create a parser that is run for its side effects only. This doesn't much matter, we discard the parsed whitespace in `operator>>` anyway. Also, we're ready to test our parser now, so who cares?

Chapter 3: Running the Parser
-----------------------------
Building a quick `main` to try all this out is not too troublesome.
```cpp
int main(int arc, char** argv) {
    using namespace std;

    // We could use auto to omit these rather convoluted types in the
    // declaration, but as this tutorial aims to be clear and easy to grasp,
    // they're included.
    parser<vector<int>> parser = parseLispList();
    ftl::either<vector<int>,error> res = parser.run(cin);

    while(!res) {
        // error has the single method message() to get a string describing what
        // the parser expected to find, but didn't.
        cout << "expected " << res.right().message() << endl;

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        res = parser.run(std::cin);
    }

    for(auto e : *res) {
        cout << e << ", ";
    }

    cout << endl;

    return 0;
}
```
We start by creating an instance of our parser, and then we run it on `std::cin` so we can try giving it various input and see how it reacts. This is not ideal, but it works as a test. In any case, after we've run it, we simply _keep_ running it until we get an input that parses. We do this using [either](Either.md)'s error-handling related interface, which among other things provides us with the ability to do the whole `while(!res)` thing.

Running the test, you'll note that giving the input
```
(1 2<return>
```
does not result in a parse error or anything, you simply get a new line with a further prompt. This is because our `whitespace` parser greedily eats the newline and asks for more. There is nothing about hitting return that signals the program that "end of input" has been reached. Thus, you can simply continue on the next line:
```
(1 2<return>
3 4)<return>
1, 2, 3, 4,
```
Try a couple of other inputs, including faulty ones, to verify that the parser works as expected (but remember its limited scope, there are many things you might think it _should_ do that it probably doesn't, because we didn't define it to).

The complete source code for this tutorial can be found [here](../examples/parser_combinatorics.cpp), and a Makefile to compile it (and the other examples) [here](../examples/Makefile).

