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
#include <ftl/forward_list.h>
#include "fwdlist_tests.h"

test_set fwdlist_tests{
	std::string("forward_list"),
	{
		std::make_tuple(
			std::string("concatMap[&]"),
			std::function<bool()>([]() -> bool {
				std::forward_list<int> l{1,2,3,4};

				auto l2 = ftl::concatMap(
					[](int x){
						return std::forward_list<int>{2*x, 2*x-1};
					},
					l
				);

				return l2 == std::forward_list<int>{2,1,4,3,6,5,8,7};
			})
		),
		std::make_tuple(
			std::string("concatMap[&&]"),
			std::function<bool()>([]() -> bool {

				auto l = ftl::concatMap(
					[](int x){
						return std::forward_list<int>{2*x, 2*x-1};
					},
					std::forward_list<int>{2,3,4}
				);

				return l == std::forward_list<int>{4,3,6,5,8,7};
			})
		),
		std::make_tuple(
			std::string("monoid::id"),
			std::function<bool()>([]() -> bool {

				return ftl::monoid<std::forward_list<float>>::id().empty();
			})
		),
		std::make_tuple(
			std::string("monoid::append[&,&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;

				auto l1 = std::forward_list<int>{1,2};
				auto l2 = std::forward_list<int>{2,3};

				return (l1 ^ l2) == std::forward_list<int>{1,2,2,3};
			})
		),
		std::make_tuple(
			std::string("monoid::append[&,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;

				auto l1 = std::forward_list<int>{1,2};
				auto l2 = std::forward_list<int>{2,3};

				return (l1 ^ std::move(l2)) == std::forward_list<int>{1,2,2,3};
			})
		),
		std::make_tuple(
			std::string("monoid::append[&&,&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;

				auto l1 = std::forward_list<int>{1,2};
				auto l2 = std::forward_list<int>{2,3};

				return (std::move(l1) ^ l2) == std::forward_list<int>{1,2,2,3};
			})
		),
		std::make_tuple(
			std::string("monoid::append[&&,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator^;

				auto l1 = std::forward_list<int>{1,2};
				auto l2 = std::forward_list<int>{2,3};

				return
					(std::move(l1) ^ std::move(l2)) == std::forward_list<int>{1,2,2,3};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->b,&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto f = [](int x){ return float(x)+1.f; };
				auto l = std::forward_list<int>{1,2,3};
				auto l2 = f % l;

				return l2 == std::forward_list<float>{2.f,3.f,4.f};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->b,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto f = [](int x){ return float(x)+1.f; };
				auto l = f % std::forward_list<int>{1,2,3};

				return l == std::forward_list<float>{2.f,3.f,4.f};
			})
		),
		std::make_tuple(
			std::string("functor::map[a->a,&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto f = [](int x){ return x+1; };
				auto l = f % std::forward_list<int>{1,2,3};

				return l == std::forward_list<int>{2,3,4};
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {

				auto l = ftl::applicative<std::forward_list<int>>::pure(2);

				return l == std::forward_list<int>{2};
			})
		),
		std::make_tuple(
			std::string("applicative::apply"),
			std::function<bool()>([]() -> bool {
				using ftl::operator*;

				std::forward_list<ftl::function<int,int>> lf{
					[](int x){ return x-1; },
					[](int x){ return x+1; }
				};

				std::forward_list<int> l = lf * std::forward_list<int>{1,2,3};

				return l == std::forward_list<int>{0,1,2,2,3,4};
			})
		),
		std::make_tuple(
			std::string("monad::bind"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				std::forward_list<int> l{1,2,3};

				auto f = [](int x){ return std::forward_list<int>{x,x+1}; };

				return (l >>= f) == std::forward_list<int>{1,2,2,3,3,4};
			})
		),
		std::make_tuple(
			std::string("foldable::foldl"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::forward_list<int> l{1,2,3};
				auto f = [](int x, int y){ return x+y; };


				return foldl(f, 0, l) == 6;
			})
		),
		std::make_tuple(
			std::string("foldable::foldr"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::forward_list<float> l{4.f,4.f,2.f};
				auto f = [](float x, float y){ return x/y; };


				return foldr(f, 16.f, l) == .125f;
			})
		),
		std::make_tuple(
			std::string("foldable::fold"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::forward_list<prod_monoid<int>> l{prod(2),prod(3),prod(2)};


				return fold(l) == 12;
			})
		)
	}
};

