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
#ifndef FTL_LAZYT_TESTS_H
#define FTL_LAZYT_TESTS_H

#include <string>
#include <ftl/maybe.h>
#include <ftl/lazy_trans.h>
#include "base.h"

test_set lazyt_tests{
	std::string("lazy_trans"),
	{
		std::make_tuple(
			std::string("functor::map"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using lazyM = ftl::lazyT<ftl::maybe<int>>;

				auto a = ftl::applicative<lazyM>::pure(1);
				auto b = [](int x){ return float(x)/4.f; } % a;

				return ***b == .25f;
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {
				using lazyM = ftl::lazyT<ftl::maybe<int>>;

				auto x = ftl::applicative<lazyM>::pure(10);

				return ***x == 10;
			})
		),
		std::make_tuple(
			std::string("applicative::apply"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;
				using lazyF = lazyT<function<int,int>>;

				ftl::function<int,int,int> f = [](int x, int y){ return x+y; };
				lazyF x{
					inplace_tag(),
					[](int x){ return monad<lazy<int>>::pure(2*x); }
				};

				lazyF y{
					inplace_tag(),
					[](int x){ return monad<lazy<int>>::pure(x/2); }};

				auto z = f % x * y;

				return *(*z)(6) == 15;
			})
		),
		std::make_tuple(
			std::string("monad::bind"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;
				using lazyM = lazyT<maybe<int>>;

				lazyM a = aPure<lazyM>()(3);
				auto b = a >>= [](int x) {
					return aPure<lazyM>()(x*2);
				};

				return ***b == 6;
			})
		),
	}
};

#endif




