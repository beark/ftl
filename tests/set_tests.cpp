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
#include <ftl/set.h>
#include "set_tests.h"

test_set set_tests{
	std::string("set"),
	{
		std::make_tuple(
			std::string("monoid:id"),
			std::function<bool()>([]() -> bool {

				return ftl::monoid<std::set<int>>::id()
					== std::set<int>{};
			})
		),
		std::make_tuple(
			std::string("monoid:append"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;
				using std::set;

				auto s1 = set<int>{1,2};
				auto s2 = set<int>{2,3,4};
				auto s3 = set<int>{3,4,5,6,7};

				auto s = std::move(s2) ^ std::move(s3) ^ s1;
				return s == set<int>{1,2,3,4,5,6,7};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->a,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto f = [](int x){ return x+1; };
				auto s = f % std::set<int>{1,2,3};

				return s == std::set<int>{2,3,4};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->a,&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto f = [](int x){ return x+1; };
				auto s1 = std::set<int>{1,2,3};
				auto s = f % s1;

				return s == std::set<int>{2,3,4};
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto s = ftl::applicative<std::set<int>>::pure(1);

				return s == std::set<int>{1};
			})
		),
		std::make_tuple(
			std::string("monad::bind"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				auto s = std::set<int>{0,1,2};
				auto s2 = s >>= [](int x){ return std::set<int>{x,2*x}; };

				return s2 == std::set<int>{0,1,2,4};
			})
		),
		std::make_tuple(
			std::string("monad::join"),
			std::function<bool()>([]() -> bool {

				auto s = std::set<std::set<int>>{
					std::set<int>{1,2,3},
					std::set<int>{3,4,5}
				};

				auto s2 = ftl::monad<std::set<int>>::join(s);

				return s2 == std::set<int>{1,2,3,4,5};
			})
		),
		std::make_tuple(
			std::string("foldable::foldl"),
			std::function<bool()>([]() -> bool {

				auto s = std::set<int>{2,3,4};

				auto f = [](float x, int y){ return x + float(y); };

				return ftl::foldl(f, 0.5f, s) == .5f + 2.f + 3.f + 4.f;
			})
		),
		std::make_tuple(
			std::string("foldable::foldr"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::set<float> s{4.f,5.f,2.f};
				auto f = [](float x, float y){ return x/y; };


				return foldr(f, 16.f, s) == .15625f;
			})
		),
		std::make_tuple(
			std::string("foldable::fold"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::set<prod_monoid<int>> l{prod(2),prod(3),prod(4)};

				return fold(l) == 24;
			})
		)
	}
};

