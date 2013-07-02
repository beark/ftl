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
#ifndef FTL_LIST_TESTS_H
#define FTL_LIST_TESTS_H

#include <ftl/list.h>
#include "base.h"

test_set list_tests{
	std::string("list"),
	{
		std::make_tuple(
			std::string("concatMap[&]"),
			std::function<bool()>([]() -> bool {
				std::list<int> l{1,2,3,4};

				auto l2 = ftl::concatMap(
					[](int x){
						return std::list<int>{2*x, 2*x-1};
					},
					l
				);

				return l2 == std::list<int>{2,1,4,3,6,5,8,7};
			})
		),
		std::make_tuple(
			std::string("concatMap[&&]"),
			std::function<bool()>([]() -> bool {

				auto l = ftl::concatMap(
					[](int x){
						return std::list<int>{2*x, 2*x-1};
					},
					std::list<int>{2,3,4}
				);

				return l == std::list<int>{4,3,6,5,8,7};
			})
		),
		std::make_tuple(
			std::string("monoid::id"),
			std::function<bool()>([]() -> bool {

				return ftl::monoid<std::list<float>>::id().empty();
			})
		),
		std::make_tuple(
			std::string("monoid::append[&,&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;

				auto l1 = std::list<int>{1,2};
				auto l2 = std::list<int>{2,3};

				return (l1 ^ l2) == std::list<int>{1,2,2,3};
			})
		),
		std::make_tuple(
			std::string("monoid::append[&,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;

				auto l1 = std::list<int>{1,2};
				auto l2 = std::list<int>{2,3};

				return (l1 ^ std::move(l2)) == std::list<int>{1,2,2,3};
			})
		),
		std::make_tuple(
			std::string("monoid::append[&&,&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;

				auto l1 = std::list<int>{1,2};
				auto l2 = std::list<int>{2,3};

				return (std::move(l1) ^ l2) == std::list<int>{1,2,2,3};
			})
		),
		std::make_tuple(
			std::string("monoid::append[&&,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;

				auto l1 = std::list<int>{1,2};
				auto l2 = std::list<int>{2,3};

				return
					(std::move(l1) ^ std::move(l2)) == std::list<int>{1,2,2,3};
			})
		)
	}
};

#endif



