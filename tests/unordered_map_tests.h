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
#ifndef FTL_UNORDERED_MAP_TESTS_H
#define FTL_UNORDERED_MAP_TESTS_H

#include <ftl/unordered_map.h>
#include "base.h"

test_set unordered_map_tests{
	std::string("unordered_map"),
	{
		std::make_tuple(
			std::string("functor::map[a->a,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using std::make_pair;

				auto f = [](int x){ return x+1; };
				auto s = f % std::unordered_map<int,int>{
					make_pair(0, 1),
					make_pair(1, 2),
					make_pair(2, 3)
				};

				return s == std::unordered_map<int,int>{
					make_pair(0, 2),
					make_pair(1, 3),
					make_pair(2, 4)
				};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->b,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using std::make_pair;

				auto f = [](int x){ return float(x)*1.5f; };
				auto s1 = std::unordered_map<int,int>{
					make_pair(0,1),
					make_pair(1,2),
					make_pair(2,3)
				};
				auto s = f % s1;

				return s == std::unordered_map<int,float>{
					make_pair(0,1.f*1.5f),
					make_pair(1,2.f*1.5f),
					make_pair(2,3.f*1.5f)
				};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->a,&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using std::make_pair;

				auto f = [](int x){ return x+1; };
				auto s1 = std::unordered_map<int,int>{
					make_pair(0,1),
					make_pair(1,2),
					make_pair(2,3)
				};
				auto s = f % s1;

				return s == std::unordered_map<int,int>{
					make_pair(0,2),
					make_pair(1,3),
					make_pair(2,4)
				};
			})
		)
	}
};

#endif






