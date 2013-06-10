#include "parser_combinator.h" 
#include <sstream>

parser<char> anyChar() {
	return parser<char>([](std::istream& s) {
		char ch;
		if(s.get(ch)) {
			return yield(ch);
		}

		return fail<char>("any character");
	});
}

parser<char> parseChar(char c) {
	return parser<char>{[c](std::istream& s) {
		if(s) {
			char ch = s.peek();
			if(ch == c) {
				// Don't forget to acutally eat a char from the stream too
				s.get();
				return yield(c);
			}
		}

		std::ostringstream oss;
		oss << "'" << c << "'";
		return fail<char>(oss.str());
	}};
}

parser<char> notChar(char c) {
	return parser<char>([c](std::istream& s) {
		if(s) {
			char ch = s.peek();
			if(ch != c) {
				s.get();
				return yield(ch);
			}
		}

		std::ostringstream oss;
		oss << "any character but '" << c << "'";
		return fail<char>(oss.str());
	});
}

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

parser<std::string> many(parser<char> p) {
	return parser<std::string>([p](std::istream& s) {
		auto r = (*p)(s);
		std::ostringstream oss;
		while(r) {
			oss << *r;
			r = (*p)(s);
		}

		return yield(oss.str());
	});
}

parser<std::string> many1(parser<char> p) {
	using ftl::operator>>=;

	// Run p once normally, bind with what's essentially "many"
	return p >>= [p](char t) {
		return parser<std::string>([p,t](std::istream& strm) {
			auto r = (*many(p))(strm);
			if(r) {
				r->insert(r->begin(), t);
				return r;
			}
			else {
				std::string s;
				s.insert(s.begin(), t);
				return yield(s);
			}
		});
	};
}

