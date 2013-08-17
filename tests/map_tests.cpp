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
#include <ftl/map.h>
#include "map_tests.h"

test_set map_tests{
	std::string("map"),
	{
		std::make_tuple(
			std::string("functor::map[a->a,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using std::make_pair;

				auto f = [](int x){ return x+1; };
				auto s = f % std::map<int,int>{
					make_pair(0, 1),
					make_pair(1, 2),
					make_pair(2, 3)
				};

				return s == std::map<int,int>{
					make_pair(0, 2),
					make_pair(1, 3),
					make_pair(2, 4)
				};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->a,&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using std::make_pair;

				auto f = [](int x){ return x+1; };
				auto s1 = std::map<int,int>{
					make_pair(0,1),
					make_pair(1,2),
					make_pair(2,3)
				};
				auto s = f % s1;

				return s == std::map<int,int>{
					make_pair(0,2),
					make_pair(1,3),
					make_pair(2,4)
				};
			})
		),
		std::make_tuple(
			std::string("foldable::foldl"),
			std::function<bool()>([]() -> bool {
				using std::make_pair;

				auto s = std::map<int,int>{
					make_pair(0, 1),
					make_pair(1, 2),
					make_pair(2, 3),
				};

				auto f = [](float x, int y){ return x + float(y); };

				return ftl::foldl(f, 0.5f, s) == .5f + 1.f + 2.f + 3.f;
			})
		),
		std::make_tuple(
			std::string("foldable::foldr"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;
				using std::make_pair;

				std::map<int,float> s{
					make_pair(0, 2.f),
					make_pair(1, 4.f),
					make_pair(2, 5.f)
				};
				auto f = [](float x, float y){ return x/y; };


				return foldr(f, 16.f, s) == .15625f;
			})
		),
		std::make_tuple(
			std::string("foldable::fold"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;
				using std::make_pair;

				std::map<int,prod_monoid<int>> m{
					make_pair(0, prod(2)),
					make_pair(1, prod(3)),
					make_pair(2, prod(4))
				};

				return fold(m) == 24;
			})
		)
	}
};

