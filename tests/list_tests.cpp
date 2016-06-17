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
#include <ftl/list.h>
#include <ftl/vector.h>
#include <ftl/maybe.h>
#include "list_tests.h"

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
			std::string("to_list[vector]"),
			std::function<bool()>([]() -> bool {

				auto v1 = std::vector<int>{1,2,3,4};
				std::vector<int> v2;

				auto r1 = ftl::to_list(v1);
				auto r2 = ftl::to_list(v2);

				return r1 == std::list<int>{1,2,3,4}
					&& r2 == std::list<int>{};
			})
		),
		std::make_tuple(
			std::string("to_list[maybe]"),
			std::function<bool()>([]() -> bool {

				auto m1 = ftl::just(1);
				ftl::maybe<int> m2 = ftl::nothing;

				auto r1 = ftl::to_list(m1);
				auto r2 = ftl::to_list(m2);

				return r1 == std::list<int>{1}
					&& r2 == std::list<int>{};
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
		),
		std::make_tuple(
			std::string("functor::map[a->b,&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto f = [](int x){ return float(x)+.5f; };
				auto l = std::list<int>{1,2,3};
				auto l2 = f % l;

				return l2 == std::list<float>{1.5f, 2.5f, 3.5f};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->b,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto f = [](int x){ return float(x)+.5f; };
				auto l = f % std::list<int>{1,2,3};

				return l == std::list<float>{1.5f, 2.5f, 3.5f};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->a,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto f = [](int x){ return x+1; };
				auto l = f % std::list<int>{1,2,3};

				return l == std::list<int>{2,3,4};
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {

				auto l = ftl::applicative<std::list<int>>::pure(2);

				return l == std::list<int>{2};
			})
		),
		std::make_tuple(
			std::string("applicative::apply"),
			std::function<bool()>([]() -> bool {
				using ftl::operator*;

				std::list<ftl::function<int(int)>> lf{
					[](int x){ return x-1; },
					[](int x){ return x+1; }
				};

				std::list<int> l = lf * std::list<int>{1,2,3};

				return l == std::list<int>{0,1,2,2,3,4};
			})
		),
		std::make_tuple(
			std::string("monad::bind[&,->list]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				std::list<int> l{1,2,3};

				auto f = [](int x){ return std::list<int>{x,x+1}; };

				return (l >>= f) == std::list<int>{1,2,2,3,3,4};
			})
		),
		std::make_tuple(
			std::string("monad::bind[T&&,->list<T>]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				auto f = [](int x){ return std::list<int>{x,x+1}; };

				return (std::list<int>{1,2,3} >>= f)
					== std::list<int>{1,2,2,3,3,4};
			})
		),
		std::make_tuple(
			std::string("monad::bind[&,->maybe]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				std::list<int> l{1,2,3,4};

				auto f = [](int x){
					return x > 2 ? std::vector<int>{x} : std::vector<int>{};
				};

				return (l >>= f) == std::list<int>{3,4};
			})
		),
		std::make_tuple(
			std::string("foldable::foldl"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::list<int> l{1,2,3};
				auto f = [](int x, int y){ return x+y; };

				auto curried1_foldl = foldl(f);
				auto curried2_foldl = foldl(f, 0);

				auto test1 = curried1_foldl(0)(l);
				auto test2 = curried1_foldl(0,l);
				auto test3 = curried2_foldl(l);

				return test1 == test2 && test2 == test3 && test3 == 6;
			})
		),
		std::make_tuple(
			std::string("foldable::foldr"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::list<float> l{4.f,4.f,2.f};
				auto f = [](float x, float y){ return x/y; };


				return foldr(f, 16.f, l) == .125f;
			})
		),
		std::make_tuple(
			std::string("foldable::fold"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::list<prod_monoid<int>> l{prod(2),prod(3),prod(2)};


				return fold(l) == 12;
			})
		),
		std::make_tuple(
			std::string("zippable::zipWith[3,3]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::list<int> l1{1,2,3};
				std::list<int> l2{2,2,2};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				return l3 == std::list<int>{3,4,5};
			})
		),
		std::make_tuple(
			std::string("zippable::zipWith[2,3]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::list<int> l1{1,2};
				std::list<int> l2{2,2,2};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				return l3 == std::list<int>{3,4};
			})
		),
		std::make_tuple(
			std::string("zippable::zipWith[3,2]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::list<int> l1{1,2,3};
				std::list<int> l2{2,2};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				return l3 == std::list<int>{3,4};
			})
		),
		std::make_tuple(
			std::string("zippable::zipWith[0,3]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::list<int> l1{};
				std::list<int> l2{2,2};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				return l3 == std::list<int>{};
			})
		),
		std::make_tuple(
			std::string("zippable::zipWith[3,0]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::list<int> l1{1,2,3};
				std::list<int> l2{};

				auto l3 = ftl::zipWith([](int x, int y){ return x + y; }, l1, l2);

				return l3 == std::list<int>{};
			})
		),
		std::make_tuple(
			std::string("zippable::zip[3,3]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::list<int> l1{1,2,3};
				std::list<float> l2{3.f,2.f,1.f};

				auto l3 = ftl::zip(l1, l2);

				return l3 == std::list<std::tuple<int,float>>{
					std::make_tuple(1,3.f),
					std::make_tuple(2,2.f),
					std::make_tuple(3,1.f)
				};
			})
		)
	}
};

