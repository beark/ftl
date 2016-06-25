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
#include <ftl/vector.h>
#include <list>
#include "vector_tests.h"

test_set vector_tests{
	std::string("vector"),
	{
		std::make_tuple(
			std::string("concatMap[&]"),
			[] {
				std::vector<int> v{1,2,3,4};

				auto v2 = ftl::concatMap(
					[](int x){
						return std::vector<int>{2*x, 2*x-1};
					},
					v
				);

				TEST_ASSERT( (v2 == std::vector<int>{2,1,4,3,6,5,8,7}) );
			}
		),
		std::make_tuple(
			std::string("concatMap[&&]"),
			[] {

				auto v = ftl::concatMap(
					[](int x){
						return std::vector<int>{2*x, 2*x-1};
					},
					std::vector<int>{2,3,4}
				);

				TEST_ASSERT( (v == std::vector<int>{4,3,6,5,8,7}) );
			}
		),
		std::make_tuple(
			std::string("monoid::id"),
			[] {

				TEST_ASSERT(ftl::monoid<std::vector<float>>::id().empty());
			}
		),
		std::make_tuple(
			std::string("monoid::append[&,&]"),
			[] {
				using ftl::operator^;

				auto v1 = std::vector<int>{1,2};
				auto v2 = std::vector<int>{2,3};

				TEST_ASSERT( ((v1 ^ v2) == std::vector<int>{1,2,2,3}) );
			}
		),
		std::make_tuple(
			std::string("monoid::append[&,&&]"),
			[] {
				using ftl::operator^;

				auto v1 = std::vector<int>{1,2};
				auto v2 = std::vector<int>{2,3};

				TEST_ASSERT( ((v1 ^ std::move(v2)) == std::vector<int>{1,2,2,3}) );
			}
		),
		std::make_tuple(
			std::string("monoid::append[&&,&]"),
			[] {
				using ftl::operator^;

				auto v1 = std::vector<int>{1,2};
				auto v2 = std::vector<int>{2,3};

				TEST_ASSERT( ((std::move(v1) ^ v2) == std::vector<int>{1,2,2,3}) );
			}
		),
		std::make_tuple(
			std::string("monoid::append[&&,&&]"),
			[] {
				using ftl::operator^;

				auto v1 = std::vector<int>{1,2};
				auto v2 = std::vector<int>{2,3};

				TEST_ASSERT( ((std::move(v1) ^ std::move(v2)) == std::vector<int>{1,2,2,3}) );
			}
		),
		std::make_tuple(
			std::string("functor::map[a->b,&"),
			[] {
				using ftl::operator%;

				auto f = [](int x){ return float(x)+1.f; };
				auto v = std::vector<int>{1,2,3};
				auto v2 = f % v;

				TEST_ASSERT( (v2 == std::vector<float>{2.f,3.f,4.f}) );
			}
		),
		std::make_tuple(
			std::string("functor::map[a->b,&&"),
			[] {
				using ftl::operator%;

				auto f = [](int x){ return float(x)+1.f; };
				auto v = f % std::vector<int>{1,2,3};

				TEST_ASSERT( (v == std::vector<float>{2.f,3.f,4.f}) );
			}
		),
		std::make_tuple(
			std::string("functor::map[a->a,&&"),
			[] {
				using ftl::operator%;

				auto f = [](int x){ return x+1; };
				auto v = f % std::vector<int>{1,2,3};

				TEST_ASSERT( (v == std::vector<int>{2,3,4}) );
			}
		),
		std::make_tuple(
			std::string("applicative::pure"),
			[] {

				auto v = ftl::applicative<std::vector<int>>::pure(2);

				TEST_ASSERT(v == std::vector<int>{2});
			}
		),
		std::make_tuple(
			std::string("applicative::apply"),
			[] {
				using ftl::operator*;

				std::vector<ftl::function<int(int)>> vf{
					[](int x){ return x-1; },
					[](int x){ return x+1; }
				};

				std::vector<int> v = vf * std::vector<int>{1,2,3};

				TEST_ASSERT( (v == std::vector<int>{0,1,2,2,3,4}) );
			}
		),
		std::make_tuple(
			std::string("monad::bind[&,->vector]"),
			[] {
				using ftl::operator>>=;

				std::vector<int> v{1,2,3};

				auto f = [](int x){ return std::vector<int>{x,x+1}; };

				TEST_ASSERT( ((v >>= f) == std::vector<int>{1,2,2,3,3,4}) );
			}
		),
		std::make_tuple(
			std::string("monad::bind[&,->list]"),
			[] {
				using ftl::operator>>=;

				std::vector<int> v{1,2,3,4};

				auto f = [](int x){
					return x % 2 == 0 ? std::list<int>{x} : std::list<int>{};
				};

				TEST_ASSERT( ((v >>= f) == std::vector<int>{2,4}) );
			}
		),
		std::make_tuple(
			std::string("monad::bind[&&,->maybe]"),
			[] {
				using ftl::operator>>=;

				auto f = [](int x){
					return x % 2 == 0 ? std::list<int>{x} : std::list<int>{};
				};

				TEST_ASSERT( ((std::vector<int>{1,2,3,4} >>= f) == std::vector<int>{2,4}) );
			}
		),
		std::make_tuple(
			std::string("foldable::foldl"),
			[] {
				using namespace ftl;

				std::vector<int> v{1,2,3};
				auto f = [](int x, int y){ return x+y; };


				TEST_ASSERT( (foldl(f, 0, v) == 6) );
			}
		),
		std::make_tuple(
			std::string("foldable::foldr"),
			[] {
				using namespace ftl;

				std::vector<float> v{4.f,4.f,2.f};
				auto f = [](float x, float y){ return x/y; };


				TEST_ASSERT( (foldr(f, 16.f, v) == .125f) );
			}
		),
		std::make_tuple(
			std::string("foldable::fold"),
			[] {
				using namespace ftl;

				std::vector<prod_monoid<int>> v{prod(2),prod(3),prod(2)};


				TEST_ASSERT(fold(v) == 12);
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[3,3]"),
			[] {
				using namespace ftl;

				std::vector<int> v1{1,2,3};
				std::vector<int> v2{2,2,2};

				auto v3 = zipWith([](int x, int y){ return x + y; }, v1, v2);

				TEST_ASSERT( (v3 == std::vector<int>{3,4,5}) );
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[2,3]"),
			[] {
				using namespace ftl;

				std::vector<int> v1{1,2};
				std::vector<int> v2{2,2,2};

				auto v3 = zipWith([](int x, int y){ return x + y; }, v1, v2);

				TEST_ASSERT( (v3 == std::vector<int>{3,4}) );
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[3,2]"),
			[] {
				using namespace ftl;

				std::vector<int> v1{1,2,3};
				std::vector<int> v2{2,2};

				auto v3 = zipWith([](int x, int y){ return x + y; }, v1, v2);

				TEST_ASSERT( (v3 == std::vector<int>{3,4}) );
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[0,3]"),
			[] {
				using namespace ftl;

				std::vector<int> v1{};
				std::vector<int> v2{2,2};

				auto v3 = zipWith([](int x, int y){ return x + y; }, v1, v2);

				TEST_ASSERT(v3 == std::vector<int>{});
			}
		),
		std::make_tuple(
			std::string("zippable::zipWith[3,0]"),
			[] {
				using namespace ftl;

				std::vector<int> v1{1,2,3};
				std::vector<int> v2{};

				auto v3 = zipWith([](int x, int y){ return x + y; }, v1, v2);

				TEST_ASSERT(v3 == std::vector<int>{});
			}
		),
		std::make_tuple(
			std::string("zippable::zip[3,3]"),
			[] {
				using namespace ftl;

				using zvec = std::vector<std::tuple<int,float>>;

				std::vector<int> v1{1,2,3};
				std::vector<float> v2{3.f,2.f,1.f};

				auto v3 = zip(v1, v2);

				auto t1 = std::make_tuple(1, 3.f);
				auto t2 = std::make_tuple(2, 2.f);
				auto t3 = std::make_tuple(3, 1.f);

				TEST_ASSERT( (v3 == zvec{t1, t2, t3}) );
			}
		)
	}
};

