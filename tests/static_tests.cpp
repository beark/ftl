/*
 * Copyright (c) 2016 Björn Aili
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */

#include <ftl/type_traits.h>
#include <ftl/type_functions.h>
#include <ftl/static.h>
#include <type_traits>

using namespace ftl;

struct fun_ob
{
	int operator() (int) { return 0; }
};

int foo(int)  { return 0; }

void ternary(bool, char, int) {  }

template<typename Sig, typename F>
constexpr bool test_usage(F&&)
{
	return std::is_same<Sig, fn_type<F>>::value;
}

// **** Static assertions of fn_traits ****
static_assert(std::is_same<bool,fn_traits<decltype(ternary)>::template argument_type<0>>::value, "Failed to deduce argument 0");
static_assert(std::is_same<char,fn_traits<decltype(ternary)>::template argument_type<1>>::value, "Failed to deduce argument 1");
static_assert(std::is_same<int,fn_traits<decltype(ternary)>::template argument_type<2>>::value, "Failed to deduce argument 2");

fun_ob bar;

static_assert(test_usage<int(int)>(fun_ob{}), "Failed deducing correct type of rvalue function object.");
static_assert(test_usage<int(int)>(bar), "Failed deducing correct type of lvalue function object.");

const fun_ob baz{};

static_assert(test_usage<int(int)>(baz), "Failed deducing correct type of const function object.");
static_assert(test_usage<int(int)>(foo), "Failed deducing correct type of free function");
static_assert(test_usage<int(int)>(&foo), "Failed deducing correct type of free function pointer");

auto no_capture = [](int){ return 0; };

static_assert(test_usage<int(int)>(no_capture), "Failed deducing correct type of lvalue lambda without capture");

void dummy()
{
	int x = 0;
	auto capture = [x](int){ return x; };

	static_assert(test_usage<int(int)>(capture), "Failed deducing correct type of lvalue lambda with capture");
}

// TODO: With c++17, lambdas can be constexpr and the below should work
// static_assert(test_usage<int(int)>([](int){ return 0; }), "Failed deducing correct type of inline lambda without capture");
// static_assert(test_usage<int(int)>([x](int){ return x; }), "Failed deducing correct type of inline lambda with capture");

// **** Static assertions of index_of ****
static_assert(index_of_v<int, int, char, float> == 0, "Failure in index_of implementation");
static_assert(index_of_v<char, int, char, float> == 1, "Failure in index_of implementation");
static_assert(index_of_v<float, int, char, float> == 2, "Failure in index_of implementation");

// **** Test static_fold behavior ****
constexpr bool and_fn(bool a, bool b) { return a && b; }

static_assert(static_fold<bool, true>::with(and_fn), "Folding a single value should return itself");
static_assert(!static_fold<bool, false>::with(and_fn), "Folding a single value should return itself");
static_assert(static_fold<bool, true, true>::with(and_fn), "Error in static_fold of two values");
static_assert(!static_fold<bool, true, false>::with(and_fn), "Error in static_fold of two values");
static_assert(!static_fold<bool, false, true>::with(and_fn), "Error in static_fold of two values");
static_assert(static_fold<bool, true, true, true>::with(and_fn), "Error in static_fold of three values");
static_assert(!static_fold<bool, true, true, false>::with(and_fn), "Error in static_fold of three values");
static_assert(!static_fold<bool, true, false, true>::with(and_fn), "Error in static_fold of three values");
static_assert(!static_fold<bool, false, true, true>::with(and_fn), "Error in static_fold of three values");
