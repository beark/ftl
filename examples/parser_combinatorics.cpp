#include <iostream>
#include <limits>
#include "parser_combinator/parser_combinator.h"

int string2int(const std::string& str) {
	char sign = str[0];
	int result = 0;
	for(size_t i = 1; i < str.size(); ++i) {
		result *= 10;
		result += str[i] - '0';
	}

	if(sign == '-')
		return -result;

	return result;
}

parser<std::string> parseQuotedString() {
	using namespace ftl;

	return parseChar('"')
		>> many(notChar('"'))
		<< parseChar('"');
}

template<typename T>
parser<T> option(parser<T> p, T t) {
	return p || ftl::monad<parser>::template pure<T>(t);
}

parser<char> parseSign() {
	return oneOf("+-");
}

parser<std::string> parseN(char sign) {
	using namespace ftl;

	return many1(oneOf("0123456789")) >>= [sign](std::string str) {
			str.insert(str.begin(), sign);
			return monad<parser>::template pure<std::string>(str);
		};
}

parser<int> parseInt() {
	using namespace ftl;

	return string2int % (option<char>(parseSign(), '+') >>= parseN);
}

int main(int arc, char** argv) {

	using std::string;

	auto parser = parseInt();
	auto res = parser.run(std::cin);

	while(!res) {
		std::cout << "expected " << res.right().message() << std::endl;

		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		res = parser.run(std::cin);
	}

	std::cout << *res + 12 << std::endl;

	return 0;
}

