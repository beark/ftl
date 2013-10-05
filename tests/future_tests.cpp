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
#include <string>
#include <ftl/future.h>
#include "future_tests.h"

test_set future_tests{
	std::string("future"),
	{
		std::make_tuple(
			std::string("functor::map"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto fb = [](int x) { return std::to_string(x); } %
					std::async(std::launch::async, []() { return 1; });

				return fb.get() == std::string("1");
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {

				auto f = ftl::applicative<std::future<int>>::pure(10);

				return f.get() == 10;
			})
		),
		std::make_tuple(
			std::string("applicative::apply"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using ftl::operator*;

				ftl::function<int(int,int)> fn = [](int x, int y){ return x+y; };

				auto f = fn
					% std::async(std::launch::async, [](){ return 1; })
					* std::async(std::launch::async, [](){ return 1; });

				return f.get() == 2;
			})
		),
		std::make_tuple(
			std::string("monad::bind"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				auto f = std::async(std::launch::async, [](){ return 1; });
				auto fn = [](int x) -> std::future<int> {
					return std::async(
						std::launch::deferred,
						[x]() -> int { return x+1; }
					);
				};

				auto g = ftl::monad<std::future<int>>::bind(std::move(f), fn);

				return g.get() == 2;
			})
		),
		std::make_tuple(
			std::string("monad::join"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				auto f = std::async(std::launch::deferred, [](){
					return std::async(std::launch::deferred, [](){
						return 1;
					});
				});


				return ftl::monad<std::future<int>>
					::join(std::move(f)).get() == 1;
			})
		),
		std::make_tuple(
			std::string("monoid::append"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;

				auto f =
					std::async(std::launch::async, [](){ return ftl::sum(1); })
					^
					std::async(std::launch::async, [](){ return ftl::sum(1); });

				return static_cast<int>(f.get()) == 2;
			})
		),
	}
};

