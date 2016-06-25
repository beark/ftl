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
#include <ftl/forward_list.h>
#include "fwdlist_tests.h"

test_set fwdlist_tests{
	std::string("forward_list"),
	{
		std::make_tuple(
			std::string("concatMap[&]"),
			[] {
				std::forward_list<int> l{1,2,3,4};

				auto l2 = ftl::concatMap(
					[](int x){
						return std::forward_list<int>{2*x, 2*x-1};
					},
					l
				);

				TEST_ASSERT( (l2 == std::forward_list<int>{2,1,4,3,6,5,8,7}) );
			}
		),
		std::make_tuple(
			std::string("concatMap[&&]"),
			[] {

				auto l = ftl::concatMap(
					[](int x){
						return std::forward_list<int>{2*x, 2*x-1};
					},
					std::forward_list<int>{2,3,4}
				);

				TEST_ASSERT( (l == std::forward_list<int>{4,3,6,5,8,7}) );
			}
		),
		std::make_tuple(
			std::string("monoid::id"),
			[] {

				TEST_ASSERT(ftl::monoid<std::forward_list<float>>::id().empty());
			}
		),
		std::make_tuple(
			std::string("monoid::append[&,&]"),
			[] {
				using ftl::operator^;

				auto l1 = std::forward_list<int>{1,2};
				auto l2 = std::forward_list<int>{2,3};

				TEST_ASSERT( ((l1 ^ l2) == std::forward_list<int>{1,2,2,3}) );
			}
		),
		std::make_tuple(
			std::string("monoid::append[&,&&]"),
			[] {
				using ftl::operator^;

				auto l1 = std::forward_list<int>{1,2};
				auto l2 = std::forward_list<int>{2,3};

				TEST_ASSERT( ((l1 ^ std::move(l2)) == std::forward_list<int>{1,2,2,3}) );
			}
		),
		std::make_tuple(
			std::string("monoid::append[&&,&]"),
			[] {
				using ftl::operator^;

				auto l1 = std::forward_list<int>{1,2};
				auto l2 = std::forward_list<int>{2,3};

				TEST_ASSERT( ((std::move(l1) ^ l2) == std::forward_list<int>{1,2,2,3}) );
			}
		),
		std::make_tuple(
			std::string("monoid::append[&&,&&]"),
			[] {
				using ftl::operator^;

				auto l1 = std::forward_list<int>{1,2};
				auto l2 = std::forward_list<int>{2,3};

				TEST_ASSERT( ((std::move(l1) ^ std::move(l2)) == std::forward_list<int>{1,2,2,3}) );
			}
		),
		std::make_tuple(
			std::string("functor::map[a->b,&]"),
			[] {
				using ftl::operator%;

				auto f = [](int x){ return float(x)+1.f; };
				auto l = std::forward_list<int>{1,2,3};
				auto l2 = f % l;

				TEST_ASSERT( (l2 == std::forward_list<float>{2.f,3.f,4.f}) );
			}
		),
		std::make_tuple(
			std::string("functor::map[a->b,&&]"),
			[] {
				using ftl::operator%;

				auto f = [](int x){ return float(x)+1.f; };
				auto l = f % std::forward_list<int>{1,2,3};

				TEST_ASSERT( (l == std::forward_list<float>{2.f,3.f,4.f}) );
			}
		),
		std::make_tuple(
			std::string("functor::map[a->a,&&]"),
			[] {
				using ftl::operator%;

				auto f = [](int x){ return x+1; };
				auto l = f % std::forward_list<int>{1,2,3};

				TEST_ASSERT( (l == std::forward_list<int>{2,3,4}) );
			}
		),
		std::make_tuple(
			std::string("applicative::pure"),
			[] {

				auto l = ftl::applicative<std::forward_list<int>>::pure(2);

				TEST_ASSERT(l == std::forward_list<int>{2});
			}
		),
		std::make_tuple(
			std::string("applicative::apply"),
			[] {
				using ftl::operator*;

				std::forward_list<ftl::function<int(int)>> lf{
					[](int x){ return x-1; },
					[](int x){ return x+1; }
				};

				std::forward_list<int> l = lf * std::forward_list<int>{1,2,3};

				TEST_ASSERT( (l == std::forward_list<int>{0,1,2,2,3,4}) );
			}
		),
		std::make_tuple(
			std::string("monad::bind"),
			[] {
				using ftl::operator>>=;

				std::forward_list<int> l{1,2,3};

				auto f = [](int x){ return std::forward_list<int>{x,x+1}; };

				TEST_ASSERT( ((l >>= f) == std::forward_list<int>{1,2,2,3,3,4}) );
			}
		),
		std::make_tuple(
			std::string("foldable::foldl"),
			[] {
				using namespace ftl;

				std::forward_list<int> l{1,2,3};
				auto f = [](int x, int y){ return x+y; };

				TEST_ASSERT( (foldl(f, 0, l) == 6) );
			}
		),
		std::make_tuple(
			std::string("foldable::foldr"),
			[] {
				using namespace ftl;

				std::forward_list<float> l{4.f,4.f,2.f};
				auto f = [](float x, float y){ return x/y; };

				TEST_ASSERT( (foldr(f, 16.f, l) == .125f) );
			}
		),
		std::make_tuple(
			std::string("foldable::fold"),
			[] {
				using namespace ftl;

				std::forward_list<prod_monoid<int>> l{prod(2),prod(3),prod(2)};

				TEST_ASSERT(fold(l) == 12);
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[3,3]"),
			[] {
				using namespace ftl;

				std::forward_list<int> l1{1,2,3};
				std::forward_list<int> l2{2,2,2};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				TEST_ASSERT( (l3 == std::forward_list<int>{3,4,5}) );
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[2,3]"),
			[] {
				using namespace ftl;

				std::forward_list<int> l1{1,2};
				std::forward_list<int> l2{2,2,2};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				TEST_ASSERT( (l3 == std::forward_list<int>{3,4}) );
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[3,2]"),
			[] {
				using namespace ftl;

				std::forward_list<int> l1{1,2,3};
				std::forward_list<int> l2{2,2};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				TEST_ASSERT( (l3 == std::forward_list<int>{3,4}) );
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[0,3]"),
			[] {
				using namespace ftl;

				std::forward_list<int> l1{};
				std::forward_list<int> l2{2,2};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				TEST_ASSERT(l3 == std::forward_list<int>{});
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[3,0]"),
			[] {
				using namespace ftl;

				std::forward_list<int> l1{1,2,3};
				std::forward_list<int> l2{};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				TEST_ASSERT(l3 == std::forward_list<int>{});
			}
		),
		std::make_tuple(
			std::string("zippable::zip[3,3]"),
			[] {
				using namespace ftl;
				using namespace std;

				using ltup = forward_list<tuple<int,float>>;

				forward_list<int> l1{1,2,3};
				forward_list<float> l2{3.f,2.f,1.f};

				auto l3 = ftl::zip(l1, l2);

				TEST_ASSERT( (l3 == ltup{ make_tuple(1,3.f), make_tuple(2,2.f), make_tuple(3,1.f) }) );
			}
		)
	}
};

