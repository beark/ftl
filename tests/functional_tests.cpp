/*
 * Copyright (c) 2013 Björn Aili
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
#include <ftl/functional.h>
#include <ftl/ord.h>
#include "functional_tests.h"

test_set functional_tests{
	std::string("functional"),
	{
		std::make_tuple(
			std::string("function allows curried calling"),
			std::function<bool()>([]() -> bool {

				ftl::function<int(int,int)> f =
					[](int x, int y){ return x + y; };

				ftl::function<int(int,int,int)> g =
					[](int x, int y, int z){ return x + y + z; };

				return f(1)(2) == 3 && g(1)(2,3) == 6;
			})
		),
		std::make_tuple(
			std::string("functor<function>::map"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				ftl::function<int(int)> unary = [](int x){ return 2*x; };
				ftl::function<int(int,int)> binary = [](int x, int y){ return x + y; };

				auto f = [](int x){ return float(x)/3.f; };

				return (f % unary)(2) == 4.f/3.f
					&& (f % binary)(2,2) == 4.f/3.f
					&& (unary % binary)(2,2) == 8;
				;
			})
		),
		std::make_tuple(
			std::string("applicative<function>::pure"),
			std::function<bool()>([]() -> bool {

				auto f = ftl::applicative<ftl::function<int(int)>>::pure(10);

				return f(-1) == 10 && f(1) == 10 && f(100) == 10;
			})
		),
		std::make_tuple(
			std::string("applicative<function>::apply"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using ftl::applicative;
				using ftl::operator*;

				function<int(int)> f = [](int x){ return 2*x; };
				auto f_ = applicative<function<function<int(int)>(int)>>::pure(f);

				auto g = f_ * f;

				return g(1) == 4;
			})
		),
		std::make_tuple(
			std::string("monad<function>::bind"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using ftl::operator>>=;

				function<int(int)> f = [](int x){ return 2*x; };
				auto g = [](int x){
					return function<float(int)>{
						[x](int y){ return float(x+y)*1.5f; }
					};
				};

				auto h = f >>= g;

				return h(1) == 4.5f;
			})
		),
		std::make_tuple(
			std::string("monoid<function>::append"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using ftl::operator^;

				auto f = ftl::comparing(&std::string::size);
				auto g = ftl::getComparator<std::string>();

				return (f ^ g)(std::string("aa"), std::string("ab")) == ftl::ord::Lt;
			})
		),
		std::make_tuple(
			std::string("functor<std::function>::map"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				std::function<int(int)> unary = [](int x){ return 2*x; };
				std::function<int(int,int)> binary = [](int x, int y){ return x + y; };

				auto f = [](int x){ return float(x)/3.f; };

				return (f % unary)(2) == 4.f/3.f
					&& (f % binary)(2,2) == 4.f/3.f;
				;
			})
		),
		std::make_tuple(
			std::string("applicative<std::function>::pure"),
			std::function<bool()>([]() -> bool {

				auto f = ftl::applicative<std::function<int(int)>>::pure(10);

				return f(-1) == 10 && f(1) == 10 && f(100) == 10;
			})
		),
		std::make_tuple(
			std::string("applicative<std::function>::apply"),
			std::function<bool()>([]() -> bool {
				using std::function;
				using ftl::applicative;
				using ftl::operator*;

				function<int(int)> f = [](int x){ return 2*x; };
				auto f_ = applicative<function<function<int(int)>(int)>>::pure(f);

				auto g = f_ * f;

				return g(1) == 4;
			})
		),
		std::make_tuple(
			std::string("monad<std::function>::bind"),
			std::function<bool()>([]() -> bool {
				using std::function;
				using ftl::operator>>=;

				function<int(int)> f = [](int x){ return 2*x; };
				auto g = [](int x){
					return function<float(int)>{
						[x](int y){ return float(x+y)*1.5f; }
					};
				};

				auto h = f >>= g;

				return h(1) == 4.5f;
			})
		),
		std::make_tuple(
			std::string("monoid<std::function>::append"),
			std::function<bool()>([]() -> bool {
				using std::function;
				using ftl::operator^;

				function<ftl::sum_monoid<int>(int)> f{
					[](int x){ return ftl::sum(1+x); }
				};

				function<ftl::sum_monoid<int>(int)> g{
					[](int x){ return ftl::sum(2+x); }
				};

				return static_cast<int>((f ^ g)(2)) == 7;
			})
		)
	}
};

