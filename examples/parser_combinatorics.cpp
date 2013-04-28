#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <limits>
#include <ftl/either.h>
#include <ftl/functional.h>

class error {
public:
	error(const error& err) : e(err.e) {}
	explicit error(const std::string& msg) : e(msg) {}
	~error() {}

	error& operator= (const error& err) {
		e = err.e;
		return *this;
	}

	std::string message() const {
		return e;
	}

private:
	std::string e;
};

template<typename T>
ftl::either<T,error> fail(const std::string& s) {
	return ftl::either<T,error>(error(s));
}

template<typename T, typename S>
class parser {
public:
	using value_type = T;
	using fn_t = ftl::function<ftl::either<T,error>,S&>;

	parser(const parser& p) : run(p.run) {}
	explicit parser(fn_t f) : run(f) {}
	~parser() {}

	ftl::either<T,error> operator()(S& s) const {
		return run(s);
	}

private:
	fn_t run;
};

template<typename S>
parser<char,S> parseChar(char c) {
	return parser<char,S>{[c](S& s) {
		if(s) {
			char r = s.peek();
			if(r == c) {
				s >> r;
				return ftl::either<char,error>(c);
			}
			else {
				std::ostringstream oss;
				oss << "expected '" << c << "', found '" << r << "'";
				return fail<char>(oss.str());
			}
		}
		else {
			std::ostringstream oss;
			oss << "expected '" << c << "', found eof";
			return fail<char>(oss.str());
		}
	}};
}

template<typename S>
parser<char,S> anyChar() {
	return parser<char,S>([](S& s) {
		if(s) {
			char c;
			s >> c;
			return ftl::either<char,error>(c);
		}
		else {
			return fail<char>("expected any character, found eof");
		}
	});
}

template<typename S>
parser<char,S> notChar(char ch) {
	return parser<char,S>([ch](S& s) {
		if(s) {
			char c = s.peek();
			if(ch != c) {
				s >> c;
				return ftl::either<char,error>(c);
			}
			else {
				std::ostringstream oss;
				oss << "expected any character but '" << ch << "', still found it";
				return fail<char>(oss.str());
			}
		}
		else {
			std::ostringstream oss;
			oss << "expected any character but '" << ch << "', found eof";
			return fail<char>(oss.str());
		}
	});
}

template<typename S>
parser<char,S> oneOf(const std::string& str) {
	return parser<char,S>{[str](S& s) {
		if(s) {
			char peek = s.peek();
			auto pos = str.find(peek);
			if(pos != std::string::npos) {
				s >> peek;
				return ftl::either<char,error>(peek);
			}
			else {
				std::ostringstream oss;
				oss << "expected one of \"" << str << "\", found '" << peek << "'";
				return fail<char>(oss.str());
			}
		}
		else {
			std::ostringstream oss;
			oss << "expected one of \"" << str << "\", found eof";
			return fail<char>(oss.str());
		}
	}};
};

template<typename T, typename S>
parser<std::vector<T>,S> many(parser<T,S> p) {
	return parser<std::vector<T>,S>{[p](S& s) {
		auto r = p(s);
		if(r.isLeft()) {
			auto v = many(p)(s);
			if(v.isLeft()) {
				v.left().insert(v.left().begin(), r.left());
				return v;
			}
			else {
				auto vec = std::vector<T>();
				vec.push_back(r.left());
				return ftl::either<std::vector<T>,error>(vec);
			}
		}
		else {
			return ftl::either<std::vector<T>,error>(std::vector<T>());
		}
	}};
}

template<typename S>
parser<std::string,S> many(parser<char,S> p) {
	using std::string;

	return parser<string,S>([p](S& s) {
		auto r = p(s);
		if(r.isLeft()) {
			auto str = many(p)(s);
			if(str.isLeft()) {
				str.left().insert(str.left().begin(), r.left());
				return str;
			}
			else {
				string str;
				str.push_back(r.left());
				return ftl::either<string,error>(str);
			}
		}
		else {
			return ftl::either<string,error>(string());
		}
	});
}

template<typename T, typename S>
parser<std::vector<T>,S> many1(parser<T,S> p) {
	return parser<std::vector<T>,S>([p](S& s) {
			return p(s) >>= [p](T t) {
				auto r = many<T,S>(p)();
				if(r.isLeft()) {
					r.left().insert(r.left().begin(), t);
					return r;
				}
				else {
					std::vector<T> v;
					v.push_back(t);
					return ftl::either<std::vector<T>,error>(v);
				}
			};
	});
}

namespace ftl {
	template<>
	struct monad<parser> {
		template<typename A, typename S>
		static parser<A,S> pure(A a) {
			return parser<A,S>{
				[a](S& stream) {
					return ftl::either<A,error>(a);
				}
			};
		}

		template<
				typename F,
				typename S,
				typename A,
				typename B = typename decayed_result<F(A)>::type>
		static parser<B,S> map(F f, parser<A,S> p) {
			return parser<B,S>([f,p](S& s) {
				auto e = p(s);
				if(e.isLeft()) {
					return ftl::either<B,error>(f(e.left()));
				}
				else {
					return ftl::either<B,error>(e.right());
				}
			});
		}

		template<
				typename F,
				typename S,
				typename A,
				typename B = typename decayed_result<F(A)>::type::value_type>
		static parser<B,S> bind(parser<A,S> p, F f) {
			return parser<B,S>([p,f](S& strm) {

				auto parseResult = p(strm);
				if(parseResult.isLeft()) {
					parser<B,S> p2 = f(parseResult.left());
					return p2(strm);
				}

				else {
					return fail<B>(parseResult.right().message());
				}
			});
		}

		static constexpr bool instance = true;
	};
}

template<typename S>
parser<std::string,S> parseQuotedString() {
	using namespace ftl;

	return parseChar<S>('"')
		>> many<S>(notChar<S>('"'))
		<< parseChar<S>('"');
}

int main(int arc, char** argv) {

	using std::string;

	auto res = parseQuotedString<std::istream>()(std::cin);

	while(res.isRight()) {
		std::cout << res.right().message() << std::endl;

		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		res = parseQuotedString<std::istream>()(std::cin);
	}

	std::cout << res.left() << std::endl;

	return 0;
}

