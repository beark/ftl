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
#ifndef FTL_MEMORY_TESTS_H
#define FTL_MEMORY_TESTS_H

#include <ftl/memory.h>
#include "base.h"

test_set memory_tests{
	std::string("memory"),
	{
		std::make_tuple(
			std::string("monoid::id"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;
				using sptr = std::shared_ptr<sum_monoid<int>>;

				auto p = monoid<sptr>::id();

				return p == nullptr;
			})
		),
		std::make_tuple(
			std::string("monoid::append"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;
				using sptr = std::shared_ptr<sum_monoid<int>>;

				auto p1 = monoid<sptr>::id();
				auto p2 = std::make_shared<sum_monoid<int>>(sum(2));
				auto p3 = std::make_shared<sum_monoid<int>>(sum(2));

				auto pr = p1 ^ p2 ^ p1 ^ p3 ^ p1;

				return *pr == sum(4);
			})
		),
		std::make_tuple(
			std::string("functor::map"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto p = std::make_shared<int>(3);
				auto pr = [](int x){ return -x; } % p;

				return *pr == -3;
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto p = applicative<std::shared_ptr<int>>::pure(2);

				return *p == 2;
			})
		),
		std::make_tuple(
			std::string("applicative::apply[*,*]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = function<int,int,int>(
					[](int x, int y){ return x-y; }
				);

				auto p1 = applicative<std::shared_ptr<int>>::pure(2);
				auto p2 = applicative<std::shared_ptr<int>>::pure(3);

				auto pr = f % p1 * p2;

				return *pr == -1;
			})
		),
		std::make_tuple(
			std::string("applicative::apply[nullptr,*]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = function<int,int,int>(
					[](int x, int y){ return x-y; }
				);

				std::shared_ptr<int> p1 = nullptr;
				auto p2 = applicative<std::shared_ptr<int>>::pure(2);

				auto pr = f % p1 * p2;

				return pr == nullptr;
			})
		),
		std::make_tuple(
			std::string("applicative::apply[*,nullptr]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = function<int,int,int>(
					[](int x, int y){ return x-y; }
				);

				auto p1 = applicative<std::shared_ptr<int>>::pure(2);
				std::shared_ptr<int> p2 = nullptr;

				auto pr = f % p1 * p2;

				return pr == nullptr;
			})
		),
		std::make_tuple(
			std::string("applicative::apply[nullptr,nullptr]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = function<int,int,int>(
					[](int x, int y){ return x-y; }
				);

				std::shared_ptr<int> p1 = nullptr;
				std::shared_ptr<int> p2 = nullptr;

				auto pr = f % p1 * p2;

				return pr == nullptr;
			})
		),
		std::make_tuple(
			std::string("monad::bind[&,->&]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto p = std::make_shared<int>(1);
				auto f = [](int x){ return std::make_shared<float>(float(x)/2.f); };

				auto pr = p >>= f;

				return *pr == .5f;
			})
		),
		std::make_tuple(
			std::string("monad::bind[nullptr,->&]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::shared_ptr<int> p{};
				auto f = [](int x){ return std::make_shared<float>(float(x)/2.f); };

				auto pr = p >>= f;

				return pr == nullptr;
			})
		),
		std::make_tuple(
			std::string("monad::bind[&,->nullptr]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto p = std::make_shared<int>(1);
				auto f = [](int){ return std::shared_ptr<float>{}; };

				auto pr = p >>= f;

				return pr == nullptr;
			})
		),
		std::make_tuple(
			std::string("monad::bind[nullptr,->nullptr]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::shared_ptr<int> p{};
				auto f = [](int){ return std::shared_ptr<float>{}; };

				auto pr = p >>= f;

				return pr == nullptr;
			})
		),
		std::make_tuple(
			std::string("foldable::foldl[&]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto p = std::make_shared<int>(2);

				return foldl([](int x, int y){ return x+y; }, 1, p) == 3;
			})
		),
		std::make_tuple(
			std::string("foldable::foldl[nullptr]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::shared_ptr<int> p{};

				return foldl([](int x, int y){ return x+y; }, 1, p) == 1;
			})
		),
		std::make_tuple(
			std::string("foldable::foldr[&]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto p = std::make_shared<int>(2);

				return foldr([](int x, int y){ return x+y; }, 1, p) == 3;
			})
		),
		std::make_tuple(
			std::string("foldable::foldr[nullptr]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::shared_ptr<int> p{};

				return foldr([](int x, int y){ return x+y; }, 1, p) == 1;
			})
		)
	}
};

#endif



