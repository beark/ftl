#include <iostream>
#include <limits>
#include <vector>
#include "parser_combinator/parser_combinator.h"

int string2int(const std::string& str) {
	int result = 0;
	for(auto nc : str) {
		result *= 10;
		result += nc - '0';
	}

	return result;
}

template<typename T>
parser<T> option(parser<T> p, T t) {
	return p || ftl::monad<parser>::pure(t);
}

parser<int> parseNatural() {
	using ftl::operator%;

	return string2int % many1(oneOf("0123456789"));
}

parser<std::string> whitespace() {
	return many1(oneOf(" \t\r\n"));
}

std::vector<int> cons(int n, std::vector<int> v) {
	v.insert(v.begin(), n);
	return v;
}

parser<std::vector<int>> parseList() {
	using namespace ftl;

	return curry(cons)
		% (parseNatural())
		* option(
			whitespace() >> lazy(parseList),
			std::vector<int>());
}

parser<std::vector<int>> parseLispList() {
	using namespace ftl;
	return parseChar('(') >> parseList() << parseChar(')');
}

int main(int arc, char** argv) {

	using std::string;

	auto parser = parseLispList();
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

