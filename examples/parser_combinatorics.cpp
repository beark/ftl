#include <iostream>
#include <limits>
#include <vector>
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

parser<std::string> spaces() {
	return many1(oneOf(" \t\r\n"));
}

std::vector<int> singleton(int n) {
	std::vector<int> v; v.push_back(n);
	return std::move(v);
}

std::vector<int> concat(std::vector<int> v1, std::vector<int> v2) {
	v1.insert(v1.end(), v2.begin(), v2.end());
	return v1;
}

parser<std::vector<int>> parseList() {
	using namespace ftl;

	return curry(concat)
		% (singleton % parseInt())
		* option(
			spaces() >> lazy(parseList),
			std::vector<int>());
}

parser<std::vector<int>> parseSexpr() {
	using namespace ftl;
	return parseChar('(') >> parseList() << parseChar(')');
}

int main(int arc, char** argv) {

	using std::string;

	auto parser = parseSexpr();
	auto res = parser.run(std::cin);

	while(!res) {
		std::cout << "expected " << res.right().message() << std::endl;

		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		res = parser.run(std::cin);
	}

	for(auto e : *res) {
		std::cout << e << ", ";
	}

	std::cout << std::endl;

	return 0;
}

