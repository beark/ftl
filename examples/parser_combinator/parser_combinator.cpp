#include "parser_combinator.h" 
#include <sstream>

using namespace ftl;

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
		while(r.template is<Right<char>>()) {
			oss << *get<Right<char>>(r);
			r = (*p)(s);
		}

		return yield(oss.str());
	});
}

std::string prepend(char c, std::string s) {
	s.insert(s.begin(), c);
	return s;
}

parser<std::string> many1(parser<char> p) {
	return curry(prepend) % p * many(p);
}

