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
#include <ftl/tuple.h>
#include "tuple_tests.h"

test_set tuple_tests{
	std::string("tuple"),
	{
		std::make_tuple(
			std::string("monoid::id"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				return monoid<std::tuple<sum_monoid<int>,prod_monoid<float>>>::id()
					== std::make_tuple(sum(0), prod(1.f));
			})
		),
		std::make_tuple(
			std::string("monoid::append"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto t1 = std::make_tuple(sum(2), prod(2));
				auto t2 = std::make_tuple(sum(1), prod(3));

				return (t1 ^ t2) == std::make_tuple(sum(3), prod(6));
			})
		),
		std::make_tuple(
			std::string("functor::map"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto f = [](int x){ return x+1; };
				auto t = f % std::make_tuple(2, 3.f, true);

				return t == std::make_tuple(3, 3.f, true);
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {

				auto t = ftl::applicative<std::tuple<int,ftl::sum_monoid<int>>>::pure(2);

				return t == std::make_tuple(2, ftl::sum(0));
			})
		),
		std::make_tuple(
			std::string("applicative::apply"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;
				using std::make_tuple;

				auto tf = make_tuple([](int x){ return 2*x; }, sum(3), prod(2));
				auto t = tf * make_tuple(3, sum(2), prod(3));


				return t == make_tuple(6, sum(5), prod(6));
			})
		)
	}
};

