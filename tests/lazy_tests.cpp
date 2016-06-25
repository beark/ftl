/*
 * Copyright (c) 2013, 2016 Björn Aili
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
#include <string>
#include <ftl/lazy.h>
#include "lazy_tests.h"
#include <iostream>

test_set lazy_tests{
	std::string("lazy"),
	{
		std::make_tuple(
			std::string("assignment"),
			[] {

				ftl::lazy<int> l1{[](){ return 1; }};
				ftl::lazy<int> l2{[](){ return 5; }};
				auto l3(l2);
				l2 = l1;

				TEST_ASSERT(*l1 == *l2);
				TEST_ASSERT(*l3 == 5);
			}
		),
		std::make_tuple(
			std::string("operator->"),
			[] {

				ftl::lazy<std::string> l1{[](){ return std::string("blah"); }};

				TEST_ASSERT(l1->size() == 4);
				TEST_ASSERT(l1->at(0) == 'b');
			}
		),
		std::make_tuple(
			std::string("mutable reference in deferred computation"),
			[] {
				std::string s("a");
				auto l = ftl::defer(
					[](std::string& x, std::string y) {
						x += "b";
						return x+y;
					},
					std::ref(s),
					std::string("c")
				);

				TEST_ASSERT(s == std::string("a"));
				TEST_ASSERT(*l == std::string("abc"));
				TEST_ASSERT(s == std::string("ab"));
			}
		),
		std::make_tuple(
			std::string("Shared computations are performed once only"),
			[] {
				ftl::lazy<int> l1([](){ return 0; });
				auto l2(l1);

				if(!(
						l1.status() == l2.status()
						&& l1.status() == ftl::value_status::deferred
				))
				{
					TEST_ASSERT(false);
				}

				int x = *l1;
				TEST_ASSERT(l1.status() == l2.status());
				TEST_ASSERT(l1.status() == ftl::value_status::ready);
				TEST_ASSERT(*l1 == x);
			}
		),
		std::make_tuple(
			std::string("monoid::append"),
			[] {
				using ftl::operator^;

				auto l1 = ftl::defer([](int x){return ftl::sum(x); }, 1);
				auto l2(l1);
				auto l3 = l1 ^ l2;

				TEST_ASSERT(static_cast<int>(*l3) == 2);
			}
		),
		std::make_tuple(
			std::string("preserves eq"),
			[] {
				using ftl::operator%;

				auto l1 = ftl::defer([](int x){ return x; }, 1);
				auto l2(l1);
				auto l3 = [](int x){ return x+1; } % l1;

				TEST_ASSERT(l1 == l2);
				TEST_ASSERT(l1 != l3);
			}
		),
		std::make_tuple(
			std::string("eq is lazy"),
			[] {
				using ftl::operator%;

				auto l1 = ftl::defer([](int x){ return x; }, 1);
				auto l2(l1);
				auto l3 = [](int x){ return x+1; } % l1;

				auto r1 = l1 == l3;
				auto r2 = l1 != l2;

				TEST_ASSERT(r1.status() == ftl::value_status::deferred);
				TEST_ASSERT(r2.status() == ftl::value_status::deferred);
				TEST_ASSERT(!r1);
				TEST_ASSERT(!r2);
			}
		),
		std::make_tuple(
			std::string("preserves lt and gt"),
			[] {
				using ftl::operator%;

				auto l1 = ftl::defer([](int x){ return x; }, 1);
				auto l2 = [](int x){ return x+1; } % l1;

				TEST_ASSERT(l1 < l2);
				TEST_ASSERT(l2 > l1);
				TEST_ASSERT(!(l2 < l1));
				TEST_ASSERT(!(l1 > l2));
			}
		),
		std::make_tuple(
			std::string("lt and gt are lazy"),
			[] {
				using ftl::operator%;

				auto l1 = ftl::defer([](int x){ return x; }, 1);
				auto l2(l1);
				auto l3 = [](int x){ return x+1; } % l1;

				auto r1 = l1 < l3;
				auto r2 = l1 > l2;

				TEST_ASSERT(r1.status() == ftl::value_status::deferred);
				TEST_ASSERT(r2.status() == ftl::value_status::deferred);
				TEST_ASSERT(r1);
				TEST_ASSERT(!r2);
			}
		),
		std::make_tuple(
			std::string("applicative::pure"),
			[] {

				auto l = ftl::applicative<ftl::lazy<int>>::pure(10);

				TEST_ASSERT(*l == 10);
			}
		),
		std::make_tuple(
			std::string("applicative::apply"),
			[] {
				using namespace ftl;

				function<int(int,int)> fn = [](int x, int y){ return x+y; };

				auto l = fn
					% applicative<lazy<int>>::pure(1)
					* applicative<lazy<int>>::pure(2);

				TEST_ASSERT(*l == 3);
			}
		),
		std::make_tuple(
			std::string("monad::bind"),
			[] {
				using namespace ftl;

				auto f = [](int x){ return lazy<float>{[x](){ return float(x)/2.f; }}; };
				auto l1 = applicative<lazy<int>>::pure(1);
				auto l2 = l1 >>= f;

				TEST_ASSERT(*l2 == .5f);
			}
		)
	}
};

