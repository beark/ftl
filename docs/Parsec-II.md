Parser Combinator Part II: Parser Generator Library
===================================================

After [part I](Prasec-I.md), it is time to have a look at how to make a library that implements and exposes the various functional concepts introduced in FTL. In this case, that means creating the library we previously used.

First thing's first: unlike the first tutorial, this one will move a bit faster and will not always explain the rationale behind every decision and step. The idea is not to teach you how to code&mdash;not even in a functional style&mdash;but how to use the FTL, after all. However, the steps described herein roughly correspond to the author's process of designing the library in the first place.

On that note, readers already familiar with Haskell may have noticed a more than passing similarity between this parser combinator library and [Parsec](http://www.haskell.org/haskellwiki/Parsec). This is no coincidence. The parser combinator library featured in these two tutorials is essentially a much simplified ~~rip-off~~ homage to Parsec. It's used because the author of FTL feels that Parsec is a very good showcase of the powers of applicative and monadic code.

Now that that's out of the way, let's get started.

1 - Defining the Data Types
---------------------------
### Parser Data Type
The primary data type of the library deserves to go first, don't you think? To arrive at a first draft, we ask ourselves, "What is a parser?" One answer would be, "Something that can be run on an input stream, the result of which is either a parse error or a representation of whatever it was we wanted to parse." Turning that into code is quite easy:
```cpp
template<typename T>
using parser = ftl::function<ftl::either<error,T>,std::istream&>;
```
But this definition is a bit naïve; it gives us no encapsulation, no way of gauranteeing internal consistency (because a user of the library could provide _any_ function they like and call it a parser). To mitigate these issues as much as we can, we declare the parser type as a thin wrapper instead.
```cpp
template<typename T>
class parser {
public:
    using value_type = T;

    parser() = delete;
    parser(const parser&) = default;
    parser(parser&&) = default;
    ~parser() = default;

    ftl::either<error,T> run(std::istream& is) const;

private:
    explicit parser(ftl::function<ftl::either<error,T>,std::istream&> f)
    : runP(f) {}

    ftl::function<ftl::either<error,T>,std::istream&> runP;
};
```
This looks better. We've prevented users from constructing nullary parsers, as well as using arbitrary functions as parsers. Instead, parsers must now be built by combining the building blocks we design. Of course, making all constructors private except copy/move means that&mdash;as we add them&mdash;our library of combinator blocks must be made friends of the `parser` data type. If that is not desirable, one could opt to create e.g. a `make_parser` function that is friends with `parser` but part of a private or anonymous namespace instead.

### Error data type
The `error` data type is even more trivial and should explain itself:
```cpp
/**
 * Error reporting class.
 * 
 * We could have used a string directly, but this thin wrapper conveys more
 * semantic information to users of the library.
 */
class error {
public:
    // Default versions of c-tors default, copy, and move are acceptable,
    // so no need to define any

    /// Construct from string error message
    explicit error(const std::string& msg) : e(msg) {}
    explicit error(std::string&& msg) : e(std::move(msg)) {}

    /// Access the error message
    std::string message() const noexcept {
        return e;
    }

private:
    std::string e;
};
```
And that's it. All of the data types are essentially done. That's hardly even an effort, so far.

2 - The Mighty Monad
-----------------------
As you may recall from part I, parsers are monads. In other words, we should make our newly defined `parser<T>` data type a monad. Looking at the [monad](Monad.md) documentation, that doesn't look _too_ hard. We start by just copying the skeleton interface and substitute `parser<T>` for all `M<T>`.
```cpp
template<>
struct monad<parser> {
    template<typename T>
    static parser<T> pure(const T&);

    template<
        typename F,
        typename A,
        typename B = ...>
    static parser<B> map(F, const parser<A>& pa);

    template<
        typename F,
        typename A,
        typename B = ...>
    static parser<B> bind(const parser<A>&, F);

    static constexpr bool instance = true;
};
```
Alright, time to implement the details. `pure` seems simple enough, it's just supposed to bring a given value into the context of parsers. Or in other words, it should create a parser that always succeeds, consumes no input, and results in whatever we gave `pure`. Here we go:
```cpp
static parser<T> pure(T t) {
    return parser<T>{
        [t](std::istream& istrm) {
            return ftl::make_right<error>(t);
        }
    };
}
```
Looks good, though on closer inspection, we realise the whole `ftl::make_right<error>` call is a bit clunky. The template parameter will always be `error` for `make_right`, so a convenience function to save repetition might be in order
```cpp
template<typename T>
auto yield(T&& t) -> decltype(ftl::make_right<error>(std::forward<T>(t))) {
    return ftl::make_right<error>(std::forward<T>(t));
}
```
Cool, now we can just call `yield(some_value)` to get an `ftl::either<error,some_type>`.

Moving on, we get to the implementation of `map`. It's supposed to map a function into the context of our monad. That is, the function we're mapping should be applied in the context of `parser`. What could this mean? Just looking at its conceptualised type&mdash;something like `parser<B> map(Function<B(A)>, parser<A>)`&mdash;should give us a pretty good idea. In fact, after that, it almost writes itself:
```cpp
template<
        typename F,
        typename A,
        typename B = typename decayed_result<F(A)>::type>
static parser<B> map(F f, parser<A> p) {
    // Return type is obvious and naturally we want to capture the parameters
    return parser<B>([f,p](std::istream& s) {
        // To get a B, we must apply f to an A, and to get an A, we must run p
        auto r = p.run(s);

        // The only quirk is that parsers can fail, but that is easily handled
        if(r) {
            return yield(f(*r));
        }
        else {
            return ftl::make_left<B>(r.left());
        }
    });
}
```
Note: because the lambda must capture by copy anyway (if we copy by reference, either of `f` or `p` could "die" before the parser is run), we opt to make those copies at the point of invocation, hence why `f` and `p` are pass-by-value. 

Now for the trickiest part: `bind`. Let's just dig right in, explanations can wait.
```cpp
template<
        typename F,
        typename A,
        typename B = typename decayed_result<F(A)>::type::value_type>
static parser<B> bind(parser<A> p, F f) {
    return parser<B>([p,f](std::istream& strm) {
        auto r = p.run(strm);
        if(r) {
            parser<B> p2 = f(*r);
            return p2.run(strm);
        }

        else {
            return ftl::make_left<B>(r.left());
        }
    });
}
```
The template parameter `B` might look a bit confusing, but it's not really that complicated. By definition of the monad concept, the result of `F` is supposed to be a parser of some type&mdash;say, `B`. So, because we had the foresight of providing a `value_type` member in `parser`, we can find out what `B` that is supposed to be, and this way guarantee that our `bind` only accepts `F`s that actually result in a `parser`.

As for the implmentation itself, it might not be obvious this is how it should be done until we think about what `bind` is supposed to do. By its definition, it is supposed to return a new "computation" that somehow represents sequencing the two "computations" given as its input, with the result of the first one acting as input to the second one. In the case of parsers, "computing" would of course mean _running_ the parser. So, that's what we do: return a new parser that simply runs first `p`, then uses its result to produce the second parser and run that too. Any error encountered aborts the "computations".

Done, we have our monad instance.

3 - Building The Basic Building Blocks
--------------------------------------
We won't go through all of the building blocks here, because most of them are quite obvious once you get the hang of it. Also, these functions aren't actually that relevant to FTL as a library, though they might serve as an introduction to how to build a good combinator based library.

### anyChar
Easiest one is first. It shouldn't even require an explanation, assuming you're familiar with the std::istream interface.
```cpp
parser<char> anyChar() {
    return parser<char>([](std::istream& s) {
        char ch;
        if(s.get(ch)) {
            return yield(ch);
        }

        return ftl::make_left<char>(error("any character"));
    });
}
```
With `s.get(ch)`, the parser attempts to read one byte&mdash;any byte&mdash;from the input stream. If the input stream has at least one character to read, we simply `yield` it. Otherwise, either the stream has reached end-of-file, or some other bad thing has happened, so we return what the parser _expected_, but could not find.

### oneOf
Surely we can do something slightly more advanced than that. How about parsing any one out of a number of characters? Actually, not that hard.
```cpp
parser<char> oneOf(std::string str) {
    return parser<char>{[str](std::istream& s) {
        if(s) {
            char peek = s.peek();
            auto pos = str.find(peek);
            if(pos != std::string::npos) {
                s.get();
                return yield(peek);
            }
        }

        std::ostringstream oss;
        oss << "one of \"" << str << "\"";
        return fail<char>(oss.str());
    }};
}
```
Very similar in concept, except this time we peek at the next character in the stream, and if it's in `str`, we `yield` it.

### many
Now it's getting interesting. `many` should parse `p` zero or more times.
```cpp
parser<std::string> many(parser<char> p) {
    return parser<string>([p](std::istream& s) {
        // A stream to put the characters in, as we parse them
        std::ostringstream oss;

        // Then we simply parse p until it results in an error
        auto r = p.run(s);
        while(r) {
            oss << *r;
            r = p.run(s);
        }

        return yield(oss.str());
    });
}
```

### operator|
One final combinator: OR-ing two parsers together. As you may know, this combinator is not actually a stand-alone one, it's part of the monoidal alternative concept&mdash;or `monoidA`. Implementing it is similar to implementing `monad`, except quite a bit easier.

```cpp
template<>
struct monoidA<parser> {
    /// Generic fail parser; always fails, never consume input
    template<typename T>
    static parser<T> fail() {
        return parser<T>{
            [](std::istream&) {
                return make_left<T>("Unknown parse error.");
            }
        };
    }

    template<typename T>
    static parser<T> orDo(parser<T> p1, parser<T> p2) {
        return parser<T>{[p1,p2](std::istream& is) {
            // Attempt to run p1
            auto r = p1.run(is);
            if(r) // Success, short-circuit and yield value!
                return r;

            else {
                // Ok, p1 failed, so try p2
                auto r2 = p2.run(is);
                if(r2)
                    return r2;

                else {
                    // Both failed, tell the world what we expected
                    std::ostringstream oss;
                    oss << r.left().message()
                        << " or " << r2.left().message();
                    return make_left<T>(oss.str());
                }
            }
        }};
    }

    static constexpr bool instance = true;
};
```

4 - Final Words
---------------
That concludes the parser combinator tutorials. If you think this seems like a pretty neat way of creating parsers, you should totally check out [the real thing](http://www.haskell.org/haskellwiki/Parsec). It's much cooler. You can of course also check out the complete source of the tutorial [here](../examples/parser_combinator/parser_combinator.h) and [here](../examples/parser_combinator/parser_combinator.cpp).

