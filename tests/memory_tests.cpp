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
#include <ftl/memory.h>
#include "memory_tests.h"

test_set memory_tests{
	std::string("memory"),
	{
		std::make_tuple(
			std::string("monoid::id"),
			[] {
				using namespace ftl;
				using sptr = std::shared_ptr<sum_monoid<int>>;

				auto p = monoid<sptr>::id();

				TEST_ASSERT(p == nullptr);
			}
		),
		std::make_tuple(
			std::string("monoid::append"),
			[] {
				using namespace ftl;
				using sptr = std::shared_ptr<sum_monoid<int>>;

				auto p1 = monoid<sptr>::id();
				auto p2 = std::make_shared<sum_monoid<int>>(sum(2));
				auto p3 = std::make_shared<sum_monoid<int>>(sum(2));

				auto pr = p1 ^ p2 ^ p1 ^ p3 ^ p1;

				TEST_ASSERT(*pr == sum(4));
			}
		),
		std::make_tuple(
			std::string("functor::map"),
			[] {
				using namespace ftl;

				auto p = std::make_shared<int>(3);
				auto pr = [](int x){ return -x; } % p;

				TEST_ASSERT(*pr == -3);
			}
		),
		std::make_tuple(
			std::string("applicative::pure"),
			[] {
				using namespace ftl;

				auto p = applicative<std::shared_ptr<int>>::pure(2);

				TEST_ASSERT(*p == 2);
			}
		),
		std::make_tuple(
			std::string("applicative::apply[*,*]"),
			[] {
				using namespace ftl;

				auto f = function<int(int,int)>(
					[](int x, int y){ return x-y; }
				);

				auto p1 = applicative<std::shared_ptr<int>>::pure(2);
				auto p2 = applicative<std::shared_ptr<int>>::pure(3);

				auto pr = f % p1 * p2;

				TEST_ASSERT(*pr == -1);
			}
		),
		std::make_tuple(
			std::string("applicative::apply[nullptr,*]"),
			[] {
				using namespace ftl;

				auto f = function<int(int,int)>(
					[](int x, int y){ return x-y; }
				);

				std::shared_ptr<int> p1 = nullptr;
				auto p2 = applicative<std::shared_ptr<int>>::pure(2);

				auto pr = f % p1 * p2;

				TEST_ASSERT(pr == nullptr);
			}
		),
		std::make_tuple(
			std::string("applicative::apply[*,nullptr]"),
			[] {
				using namespace ftl;

				auto f = function<int(int,int)>(
					[](int x, int y){ return x-y; }
				);

				auto p1 = applicative<std::shared_ptr<int>>::pure(2);
				std::shared_ptr<int> p2 = nullptr;

				auto pr = f % p1 * p2;

				TEST_ASSERT(pr == nullptr);
			}
		),
		std::make_tuple(
			std::string("applicative::apply[nullptr,nullptr]"),
			[] {
				using namespace ftl;

				auto f = function<int(int,int)>(
					[](int x, int y){ return x-y; }
				);

				std::shared_ptr<int> p1 = nullptr;
				std::shared_ptr<int> p2 = nullptr;

				auto pr = f % p1 * p2;

				TEST_ASSERT(pr == nullptr);
			}
		),
		std::make_tuple(
			std::string("monad::bind[&,->&]"),
			[] {
				using namespace ftl;

				auto p = std::make_shared<int>(1);
				auto f = [](int x){ return std::make_shared<float>(float(x)/2.f); };

				auto pr = p >>= f;

				TEST_ASSERT(*pr == .5f);
			}
		),
		std::make_tuple(
			std::string("monad::bind[nullptr,->&]"),
			[] {
				using namespace ftl;

				std::shared_ptr<int> p{};
				auto f = [](int x){ return std::make_shared<float>(float(x)/2.f); };

				auto pr = p >>= f;

				TEST_ASSERT(pr == nullptr);
			}
		),
		std::make_tuple(
			std::string("monad::bind[&,->nullptr]"),
			[] {
				using namespace ftl;

				auto p = std::make_shared<int>(1);
				auto f = [](int){ return std::shared_ptr<float>{}; };

				auto pr = p >>= f;

				TEST_ASSERT(pr == nullptr);
			}
		),
		std::make_tuple(
			std::string("monad::bind[nullptr,->nullptr]"),
			[] {
				using namespace ftl;

				std::shared_ptr<int> p{};
				auto f = [](int){ return std::shared_ptr<float>{}; };

				auto pr = p >>= f;

				TEST_ASSERT(pr == nullptr);
			}
		),
		std::make_tuple(
			std::string("foldable::foldl[&]"),
			[] {
				using namespace ftl;

				auto p = std::make_shared<int>(2);

				TEST_ASSERT( (foldl([](int x, int y){ return x+y; }, 1, p) == 3) );
			}
		),
		std::make_tuple(
			std::string("foldable::foldl[nullptr]"),
			[] {
				using namespace ftl;

				std::shared_ptr<int> p{};

				TEST_ASSERT( (foldl([](int x, int y){ return x+y; }, 1, p) == 1) );
			}
		),
		std::make_tuple(
			std::string("foldable::foldr[&]"),
			[] {
				using namespace ftl;

				auto p = std::make_shared<int>(2);

				TEST_ASSERT( (foldr([](int x, int y){ return x+y; }, 1, p) == 3) );
			}
		),
		std::make_tuple(
			std::string("foldable::foldr[nullptr]"),
			[] {
				using namespace ftl;

				std::shared_ptr<int> p{};

				TEST_ASSERT( (foldr([](int x, int y){ return x+y; }, 1, p) == 1) );
			}
		)
	}
};

