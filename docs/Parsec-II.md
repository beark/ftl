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
While very straight forward, we can be a lot more clever than that. To save ourselves a lot of trouble implementing all sorts of functions or concepts or whatnot for combining parsers (remember, it's a parser _combinator_ library), we could just use a transformer.
```cpp
    template<typename T>
    using parser = ftl::eitherT<error,ftl::function<T,std::istream&>>;
```
And that's it. Done. Two lines of code and we have a fully functional monad instance and monoidal alternative instance, just like that. Well, technically, the first type alias was already a monad (though not a monoidal alternative), but it wouldn't have worked the way users of our library expects. For one, when creating a new parser that depends on the result of one sequenced before it, it would have to accept an `either<error,T>` as parameter instead of just `T` and would have had to explicitly handle the case where the either was an error. This conflicts with the point of a library: to save work for users.

Oh right, there's the matter of letting clients run parsers too (ideally, they should be unaware of the underlying implementation of `parser<T>`). No problem.
```cpp
template<typename T>
ftl::either<error,T> run(parser<T> p, std::istream& is) {
    // Because p is really just a function wrapped in a transformer, we only
    // have to "dereference" it to use it as the base type instead.
    return (*p)(is);
}
```

### Error data type
The `error` data type is also fairly trivial and should explain itself. In fact, the main purpose of it is simply to provide semantic information to users of the library. `ftl::either<std::string,T>` does not convey as much information as `ftl::either<error,T>`.
```cpp
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
One thing left: looking at the `eitherT` interface, we realise that it is only a monoidal alternative instance if its `L` type is a monoid. So, `error` should be a monoid then. 

```cpp
// Monoid instances need to be in the ftl namespace
namespace ftl {
    template<>
    struct monoid<error> {
        // The identity element is of course an empty error, else we'd
        // break one of the monoid laws.
        static error id() {
            return error(std::string(""));
        }

        // Appending is... well, appending
        static error append(const error& e1, const error& e2) {
            if(e1.message().empty())
                return error(e2.message());
            if(e2.message().empty())
                return error(e1.message());

            return error(e1.message() + " or " + e2.message());
        }

        static constexpr bool instance = true;
    };
}

```
Alright, all of the data types are essentially done. That's hardly even an effort, so far.

2 - Building The Basic Building Blocks
--------------------------------------
We won't go through all of the building blocks here, because most of them are quite obvious once you get the hang of it. Also, these functions aren't actually that relevant to FTL as a library, though they might serve as an introduction to how to build a good combinator based library. There are also a couple of convenience functions used throughout, the implementation of which are quite trivial and not all that interesting.

### anyChar
Easiest one is first. It shouldn't even require an explanation, assuming you're familiar with the `std::istream` interface.
```cpp
parser<char> anyChar() {
    return parser<char>([](std::istream& s) {
        char ch;
        if(s.get(ch)) {
            // Yield simply creates an either<error,T> from a T. Basically, a
            // shorter version of ftl::make_right.
            return yield(ch);
        }

        // Similar to yield, but for make_left
        return fail<char>(error("any character"));
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

        // Then we simply run p until it results in an error
        auto r = (*p)(s);
        while(r) {
            oss << *r;
            r = (*p)(s);
        }

        return yield(oss.str());
    });
}
```

3 - Final Words
---------------
That concludes the parser combinator tutorials. If you think this seems like a pretty neat way of creating parsers, you should totally check out [the real thing](http://www.haskell.org/haskellwiki/Parsec). It's much cooler. You can of course also check out the complete source of the tutorial [here](../examples/parser_combinator/parser_combinator.h) and [here](../examples/parser_combinator/parser_combinator.cpp).

