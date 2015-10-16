Parser Combinator Part I: Simple Parser
=======================================

In this tutorial, we will use a parser combinator library built on FTL to create a simple parser. For now, we disregard the implementation of the library itself, and focus on simply using its monadic interface to combine smaller, basic parsers into more advanced ones. [Part II](Parsec-II.md) concerns itself with how the library itself was built.

1 - Parser Combinator API
-------------------------
Before we do anything, we should familiarise ourselves with the API we intend to use: that of the parser combinator library. If you wish, you can head straight to [source](../examples/parser_combinator/parser_combinator.h) and have a glance at that, otherwise a short description follows. If you do sift through the source, remember that this tutorial does not concern itself with the implementation details, so there is no need to look beyond declarations and public parts of data types.

On to the good stuff. A central part of the API is the type `parser<T>`, which denotes a parser capable of somehow extracting a _T_ from an input stream. How it does this, we do not know. All we know is that it consumes characters and produces something of the stated type. The direct interface to this type can be summarised as:
```cpp
/**
 * A parser of Ts.
 *
 * This is the central data type of the library.
 *
 * \par Concepts
 * \li Monad
 * \li MonoidAlternative
 */
template<typename T>
using parser = /* some strange type */
```
What's that? Parsers are just an alias of some weird type, with essentially no public interface that we can find? Something must surely be wrong? Well, let's give it a chance to prove itself at least...

So, what _does_ this interface tell us? First, apparently parsers are monads. This actually tells us quite a bit: we now know we can use all of the functor, applicative functor, and monad interfaces to interact with parsers. Second, we know parsers are also monoidal alternatives, which gives us one more way of combining parsers, as well as the ability to create parsers that automatically fail. Ok, this is all good and stuff, but how do we run a parser?

```cpp
    /**
     * Function for running parsers.
     */
    template<typename T>
    ftl::either<error,T> run(parser<T> p, std::istream& is) {
```
Ah, that makes things easier. So, we're just supposed to invoke `run` on a parser and an input stream, and in return we get either an error&mdash;presumably if the parser fails&mdash;or whatever it was we wanted to parse.

Alright, now we've deduced quite a bit about this `parser<T>` type, but we still can't do much, because we're missing a rather fundamental thing: how do we actually create parsers (that _do_ something, besides failing)? Because looking at the interface, there are no public constructors! Let's browse further down the parser combinator API, and see if there is something there to enlighten us.
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
When run, the above parser will successfully parse any sequence of two characters in a row, and return whatever the character was. The parser will fail if the input stream's next two characters are not the same. I.e., the sequence "aa" will parse fine and return 'a', while the sequence "ab" will return an `error`.

This works because the monadic `operator>>=` sequences two parsers and gives the result of the first as input to the second, except if there was a parse error. Hence, if we were to invoke `sillyExample().run(someIstream)`, first `anyChar().run(someIstream)` would be invoked, and if its result is not an error, it would be given as `parseChar(char)`'s argument, after which _that_ parser's `run` would be invoked and _its_ result returned. Once you are more familiar with monads, all this would be essentially self-evident from the code above.

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
Here, we rely on yet another of the Functor series of operations: the `fmap` function (in the form of `operator%` above). What it does is essentially to "lift" a plain old value level function into some context&mdash;in this case the context of 'parsers'. Or in other words, it's simply the classical `map` function in disguise: if we squint enough, we can sort of imagine a parser as a container that contains zero or one values of the type it parses to. `fmap` then applies the function given to it (`string2int`) on all "elements" of the "container".

`many1` above is similar to the previously mentioned `many`, except that it must successfully parse at least _one_ of the given parsers, or a parse failure is returned. The latter is of course the equivalent of an empty container for purposes of `fmap`.

Alright, we have covered enough to do something actually useful. Let's move on!

2 - Doing Something Non-Trivial
-------------------------------
At this point, we have a library to build parsers and the knowledge to use it, so what do we do? For this particular tutorial, we'll try to stay in the shallow end of the pool, but still put together something that shouldn't bore us _too_ much. Let's make ourselves a parser that parses a lisp-list of integers to an `std::vector<int>`. This should provide us with a few challenges, yet not grow out of hand.

We can give it a go using a top-down approach. A list in lisp has an opening parenthesis, then a space separated list of elements, and then a closing parenthesis. Starting from that, we intuitively get:
```cpp
parser<std::vector<int>> parseLispList() {
    using namespace ftl;
    return parseChar('(')
        >> parseList()
        << parseChar(')');
}
```
where `parseList`'s implementation is yet to be determined. How the new operators work should be quite intuitive, if you've read the public `monad` interface, but for easier reference:
* `operator>>` returns a computation (parser, in this case) equivalent of doing its two operand computations in sequence from left to right, while discarding the result of the first computation. Should either sub-computation fail, the composite computation will of course also fail.
* `operator<<` is almost a mirror of the above, except it _also_ uses left-to-right sequencing. In other words, the left hand side is computed, then the right hand side, then the right result is discarded and the _left_ result is returned.

In our context of parsers, this above simply means that when we run the composite parser (parseLispList), `parseChar('(')` will run first. Its result is not very interesting however, so we discard it. After that, our as of yet unimplemented `parseList()` will be run, with its result saved for later. Finally, `parseChar(')')` will run and have its result discarded in favour of whatever was left of it (which is of course `parseList`). If either of the three sub-parsers fail, the parsing will stop (the remaining parsers will not be run) and the composite parser will fail with whatever error was encountered in the sub-parser.

Okay, so we have our top level parser, time to descend. What should `parseList` do? Well, it should keep parsing whitespace separated numbers as long as there are numbers to find. It should also concatenate all these numbers into a single `std::vector`. Let's sketch this out:
```cpp
parser<std::vector<int>> parseList() {
	using namespace ftl;
    return cons % parseNatural() * (whitespace() >> parseList());
}
```
A few new things here that require explanation. First, `operator*` is from applicative functors, which we know parsers to be. Just like the monadic operators we've already seen, it composes existing computations (or containers, or however you prefer to view these abstractions) to produce a new one, that is somehow a composite of the ones it was given.

In the case of "apply" (as the operation `operator* represents in FTL is often called), the composed computation basically does the following when run:
* "Unwrap" (run the parser, in this case) the value on its left hand side, which must result in a function.
* "Unwrap" the contextual value on its right hand side (`whitespace() >> lazy(parseList)`)
* And finally, apply the function from the first argument on the result from the second.

But how is `cons % parseInt()` a function? Well, it isn't. It's a parser that _results_ in a function when we run it. But how does it result in a function, then? Didn't we just recently see that `operator%`/`fmap` already applies a function to a context wrapped value? The only conclusion is that applying `cons` to an int must result in _another_ function. `cons` is a [curried](https://github.com/beark/ftl#curried-function-calling) binary function!

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
    using ftl::operator|;

    return p | ftl::monad<parser>::pure(t);
}
```
Easily! The OR-combinator from the monoidal alternative instance, and monad's `pure` gives us the final piece. Our `option` above quite simply reads as "parse p, but if that fails, default to t".

The OR-combinator is yet another combinator, this time resulting in a computation that represents first trying the first might-fail computation, and if it does fail, try the other. `pure` meanwhile is part of the monad interface and is a way to create a computation that always results in exactly the given value.

In the context of parsers, the former from above (the OR-combinator) should be obvious in meaning, and the latter is a means of creating a parser that consumes no input when run, but does produce a value (the one that was given). It will do so regardless of how many times it is run, and whether the input stream is empty, bad, or in a good state.

Alright, we can return to our `parseList` again. It now looks like this:
```cpp
parser<std::vector<int>> parseList() {
    return curry(cons)
        % parseNatural()
        * option(whitespace() >> parseList(), std::vector<int>());
}
```
Hmm, we have another problem. While _running_ the parser will not unconditionally recurse, actually _creating_ it will. Every call to `parseList()` will call `parseList()` again. That's no good. But surely a parser must be able to recurse somehow? Scanning the parser generator API again, we find:
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
Finally, our "sketch" is no longer a sketch; we are ready to descend further. Now, `parseNatural()` we have since before, and `parseList` we just defined. So `whitespace()` and `cons(int, std::vector<int>)` are the only two unknowns.

So, `cons` should hopefully be familiar to many readers already. If not, it's basically a funny name for "prepend" that, for historical reasons, stuck around in functional languages. It should simply prepend a value to a container. It's quite trivial to implement and not particularly informative to include, so if interested, just look at the source.

Only `whitespace()` left then. After the previous parts, it too is almost trivial:
```cpp
parser<std::string> whitespace() {
    return many1(oneOf(" \t\r\n"));
}
```
While we actually don't care at all what whitespace we actually parse, the parser generator library as it exists gives us no way of declaring a parser that consumes input without also producing _something_. Or in other words, we cannot create a parser that is run for its side effects only. This doesn't much matter, we discard the parsed whitespace in `operator>>` anyway. Also, we're ready to test our parser now, so who cares?

3 - Running the Parser
-----------------------------
Building a quick `main` to try all this out is not too troublesome.
```cpp
int main(int arc, char** argv) {
    using namespace std;

    // We could use auto to omit these rather convoluted types in the
    // declaration, but as this tutorial aims to be clear and easy to grasp,
    // they're included.
    parser<vector<int>> parser = parseLispList();
    ftl::either<error,vector<int>> res = parser.run(cin);

    while(!res) {
        // error has the single method message() to get a string describing what
        // the parser expected to find, but didn't.
        cout << "expected " << res.left().message() << endl;

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
We start by creating an instance of our parser, and then we run it on `std::cin` so we can try giving it various input and see how it reacts. This is not ideal, but it works as a test. In any case, after we've run it, we simply _keep_ running it until we get an input that parses. We do this using either's error-handling related interface, which among other things provides us with the ability to do the whole `while(!res)` thing.

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

