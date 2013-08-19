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
#include <ftl/prelude.h>
#include <ftl/maybe.h>
#include "prelude_tests.h"

int curry_me(int x, int y) {
	return x+y;
}

test_set prelude_tests{
	std::string("prelude"),
	{
		std::make_tuple(
			std::string("identity function object"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto m = ftl::id % ftl::value(10);

				return m && *m == 10;
			})
		),
		std::make_tuple(
			std::string("currying regular functions"),
			std::function<bool()>([]() -> bool {
				auto f = ftl::curry(curry_me);
				return f(2)(2) == f(2,2) && f(2,2) == curry_me(2,2);
			})
		),
		std::make_tuple(
			std::string("currying std::function"),
			std::function<bool()>([]() -> bool {
				std::function<int(int,int)> f = [](int x, int y) { return x+y; };
				auto g = ftl::curry(f);

				return g(2)(2) == g(2,2) && g(2,2) == f(2,2);
			})
		),
		std::make_tuple(
			std::string("currying generic function object"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x, int y, int z){ return x+y+z; };
				auto g = ftl::curry(f);

				return g(2)(2)(2) == g(2,2)(2)
					&& g(3,3)(3) == g(3,3,3)
					&& g(2,3,4) == f(2,3,4);
			})
		),
		std::make_tuple(
			std::string("compose[...,R(*)(Ps...)]"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x){ return 2*x; };
				auto g = [](int x){ return float(x)/3.f; };
				auto h = ftl::compose(g, f, curry_me);

				return h(2,2) == 8.f/3.f;
			})
		),
		std::make_tuple(
			std::string("compose[...,function<R,Ps...>]"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x){ return 2*x; };
				auto g = [](int x){ return float(x)/3.f; };
				auto h = ftl::compose(g, f, ftl::curry(curry_me));

				return h(2,2) == 8.f/3.f;
			})
		),
		std::make_tuple(
			std::string("flip[function<R,A,B>]"),
			std::function<bool()>([]() -> bool {
				ftl::function<int,int,int> f = [](int x, int y){ return x/y; };
				auto g = ftl::flip(f);

				return g(2,4) == 2;
			})
		)
	}
};

