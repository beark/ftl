#include <sstream>
#include <ftl/either.h>
#include <ftl/functional.h>

#ifndef PARSER_GEN_H
#define PARSER_GEN_H

/**
 * Error reporting class.
 * 
 * We could have used a string directly, but this thin wrapper allows us to
 * actually instantiate \c ftl::either<string,error>, whereas it's impossible
 * to instantiate \c ftl::either<string,string>.
 */
class error {
public:
	// Default versions of c-tors default, copy, and move are acceptable

	/// Construct from string error message
	explicit error(const std::string& msg) : e(msg) {}
	explicit error(std::string&& msg) : e(std::move(msg)) {}

	/// Access the error message
	std::string message() const {
		return e;
	}

private:
	std::string e;
};

/// Convenience function to reduce template gibberish
template<typename T>
ftl::either<T,error> fail(const std::string& s) {
	return ftl::either<T,error>(error(s));
}

/// Convenience function to reduce template gibberish
template<typename T>
ftl::either<T,error> yield(const T& t) {
	return ftl::either<T,error>(t);
}

// Forward declarations required for later friend declarations, sigh
template<typename T> class parser;

template<typename T>
parser<T> operator|| (parser<T>, parser<T>);

template<typename T>
parser<T> lazy(ftl::function<parser<T>>);

template<typename T>
parser<T> lazy(parser<T>(*)());

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
public:
	/* The basic library set of building blocks must be friended,
	 * as they all use private parts (c-tor).
	 */
	friend parser lazy<>(ftl::function<parser>);
	friend parser lazy<>(parser(*)());
	friend parser<char> anyChar();
	friend parser<char> parseChar(char);
	friend parser<char> notChar(char);
	friend parser<char> oneOf(std::string);
	friend parser<std::string> many(parser<char>);
	friend parser<std::string> many1(parser<char>);
	friend parser operator|| <>(parser, parser);
	friend class ftl::monad<parser>;

	using value_type = T;

	// Default set of c-tors is fine

	/**
	 * Run the parser, reading characters from some input stream.
	 */
	ftl::either<T,error> run(std::istream& s) const {
		return runP(s);
	}

private:
	using fn_t = ftl::function<ftl::either<T,error>,std::istream&>;
	explicit parser(fn_t f) : runP(f) {}

	fn_t runP;
};

namespace ftl {
	/**
	 * Monad instance for parsers.
	 *
	 * Also gives us Applicative and Functor.
	 */
	template<>
	struct monad<parser> {

		/**
		 * Consume no input, yield a.
		 */
		template<typename A>
		static parser<A> pure(A a) {
			return parser<A>{
				[a](std::istream& stream) {
					return yield(a);
				}
			};
		}

		/**
		 * Maps a function to the result of a parser.
		 *
		 * Can be a very useful combinator, f.ex. to apply smart constructors
		 * to the result of another parser.
		 */
		template<
				typename F,
				typename A,
				typename B = typename decayed_result<F(A)>::type>
		static parser<B> map(F f, parser<A> p) {
			return parser<B>([f,p](std::istream& s) {
				auto r = p.run(s);
				if(r) {
					return yield(f(*r));
				}
				else {
					return fail<B>(r.right().message());
				}
			});
		}

		/**
		 * Run two parsers in sequence, mapping output of p to f.
		 */
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
					return fail<B>(r.right().message());
				}
			});
		}

		// Yes, parser is an instance of monad (and applicative, and functor)
		static constexpr bool instance = true;
	};
}

/**
 * Combinator to try parsers in sequence.
 *
 * First tries to run \c p1, and if that fails, tries p2. If both fail, a parse
 * error is returned.
 */
template<typename T>
parser<T> operator|| (parser<T> p1, parser<T> p2) {
	return parser<T>{[p1,p2](std::istream& strm) {
		auto r = p1.run(strm);
		if(r) {
			return r;
		}
		else {
			auto r2 = p2.run(strm);
			if(r2) {
				return r2;
			}
			else {
				std::ostringstream oss;
				oss << r.right().message() << " or " << r2.right().message();
				return fail<T>(oss.str());
			}
		}
	}};
}

/* What follows is a basic set of blocks that a user of the library can
 * combine with the various combinators available (operator||, monad instance,
 * applicative instance, functor instance).
 */

/**
 * Parses any one character.
 *
 * This parser can only fail if the end of stream has been reached.
 */
parser<char> anyChar();

/**
 * Parses one specific character.
 *
 * This parser will fail if the next character in the stream is not equal
 * to \c c.
 */
parser<char> parseChar(char c);

/**
 * Parses any character except c.
 *
 * This parser will fail if the next character does not equal c.
 */
parser<char> notChar(char c);

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

/**
 * Greedily parses 1 or more of p.
 *
 * This parser will fail if the first attempt at parsing p fails.
 */
parser<std::string> many1(parser<char> p);

/**
 * Lazily run the parser generated by f
 *
 * This is useful e.g. if you want a parser to recurse.
 */
template<typename T>
parser<T> lazy(ftl::function<parser<T>> f) {
	return parser<T>([f](std::istream& is) {
		return f().run(is);
	});
}

/// \overload
template<typename T>
parser<T> lazy(parser<T>(*f)()) {
	return parser<T>([f](std::istream& is) {
			return f().run(is);
	});
}

#endif

