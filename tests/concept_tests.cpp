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
#include <ftl/maybe.h>
#include <ftl/vector.h>
#include <ftl/list.h>
#include <type_traits>
#include "concept_tests.h"

static_assert(std::is_trivial<ftl::sum_monoid<int>>::value, "sum_monoid of trivial base type should be trivial");
static_assert(std::is_trivial<ftl::prod_monoid<int>>::value, "prod_monoid of trivial base type should be trivial");
static_assert(std::is_trivial<ftl::any>::value, "any should be trivial");
static_assert(std::is_trivial<ftl::all>::value, "all should be trivial");

test_set concept_tests{
	std::string("concepts"),
	{
		std::make_tuple(
			std::string("Monoid: curried mappend"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(sum(2));
				auto m2 = just(sum(2));

				auto m3 = mappend % m1 * m2;

				return m3 == just(sum(4));
			})
		),
		std::make_tuple(
			std::string("Functor: curried fmap"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = [](int x){ return x+1; };
				auto fm = fmap(f);

				auto m = fm(just(2));

				return m == just(3) && m == fmap(f)(just(2));
			})
		),
		std::make_tuple(
			std::string("Functor: fmap [member fn]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				struct test {
					test(int x) : x(x) {}

					int foo() {
						return x + 3;
					}

					int x;
				};

				auto r = &test::foo % just(test{3});

				return r.template unsafe_get<int>() == 6;
			})
		),
		std::make_tuple(
			std::string("Applicative: curried aapply"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto mf = just(function<int(int)>([](int x){
						return x+1;
				}));

				auto ap = aapply(mf);

				return ap(just(2)) == just(3)
					&& aapply(mf)(just(2)) == just(3);
			})
		),
		std::make_tuple(
			std::string("Monad: curried mbind"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m = just(2);
				auto cbind = mbind(m);
				auto f = [](int x){ return just(x+1); };

				return cbind(f) == just(3);
			})
		),
		std::make_tuple(
			std::string("Monad: >>="),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(2);

				auto f1 = [](int x){ return just(float(x)*0.5f); };
				auto f2 = [](float x){ return just(int(x*3.33f)); };

				return
					((m1 >>= f1) >>= f2) == just(3);
			})
		),
		std::make_tuple(
			std::string("Monad: >>= [member fn]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				struct foo {
					explicit foo(int x) : x(x) {}

					maybe<int> bar() const {
						return just(2*x);
					}

					int x;
				};

				auto m = just(foo{3});

				return
					(m >>= &foo::bar) == just(6);
			})
		),
		std::make_tuple(
			std::string("Monad: >>"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(1);
				auto m2 = just(2);
				maybe<int> m3 = nothing;

				return (m1 >> m2) == just(2) && (m3 >> m1) == maybe<int>{nothing};
			})
		),
		std::make_tuple(
			std::string("Monad: <<"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(1);
				auto m2 = just(2);
				maybe<int> m3 = nothing;

				return (m1 << m2) == just(1) && (m1 << m3) == maybe<int>{nothing};
			})
		),
		std::make_tuple(
			std::string("Monad: <<="),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(0.f);
				auto m2 = just(2.f);
				auto f = [](float x){
					return x == 0 ? nothing : just(8.f / x);
				};

				return (f <<= f <<= m1) == nothing
					&& (f <<= f <<= m2) == just(2.f);
			})
		),
		std::make_tuple(
			std::string("Monad: mixed, non-trivial sequence"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto plusOne = [](int x){ return just(x+1); };
				auto mulTwo = [](int x){ return just(2*x); };

				auto mOne = just(1);

				auto m1 = ((mOne >>= plusOne) >>= mulTwo) << (mulTwo <<= mOne);

				return m1 == just(4);
			})
		),
		std::make_tuple(
			std::string("Foldable: curried foldMap"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto foldmap = foldMap(&sum<int>);

				std::vector<int> v{3,3,4};

				return foldmap(v) == 10;
			})
		),
		std::make_tuple(
			std::string("Foldable: curried foldr"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto foldr2 = foldr(std::plus<int>());
				auto foldr1 = foldr2(0);

				std::vector<int> v{3,3,4};

				return
					foldr(std::plus<int>(), 0, v) == 10
					&& foldr2(0, v) == 10
					&& foldr1(v) == 10
					&& foldr(std::plus<int>())(0)(v) == 10;
			})
		),
		std::make_tuple(
			std::string("Foldable: curried foldl"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = [](int x, int y){ return x+y; };

				auto foldl2 = foldl(f);
				auto foldl1 = foldl(f, 0);

				std::vector<int> v{3,3,4};

				return
					foldl(f, 0, v) == 10
					&& foldl2(0, v) == 10
					&& foldl1(v) == 10
					&& foldl(f, 0)(v) == 10;
			})
		),
		std::make_tuple(
			std::string("Foldable: foldl associativity"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto rCons = [](std::list<int> xs, int x){
					xs.push_front(x);
					return xs;
				};

				std::list<int> l{2,3,4};

				return foldl(rCons, std::list<int>{}, l)
					== std::list<int>{4,3,2};
			})
		),
		std::make_tuple(
			std::string("Zippable: curried zipWith"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = [](int x, int y){ return x+y; };

				auto zipWithF = zipWith(f);

				std::vector<int> v1{3,3,4};
				std::vector<int> v2{1,3,5};

				auto zipWithFV1 = zipWith(f, v1);

				std::vector<int> expected{4,6,9};

				return zipWithF(v1, v2) == expected
					&& zipWithFV1(v2) == expected;
			})
		),
		std::make_tuple(
			std::string("Zippable: zip"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::vector<int> v1{3,3,4};
				std::vector<int> v2{1,3,5,6};

				std::vector<std::tuple<int,int>> expected1{
					std::make_tuple(3,1),
					std::make_tuple(3,3),
					std::make_tuple(4,5)
				};

				std::vector<std::tuple<int,int>> expected2{
					std::make_tuple(1,3),
					std::make_tuple(3,3),
					std::make_tuple(5,4)
				};

				return zip(v1, v2) == expected1
					&& zip(v2, v1) == expected2;
			})
		),
		std::make_tuple(
			std::string("fmap(fold, v)"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				std::vector<std::vector<sum_monoid<int>>> v{
					{sum(1), sum(2)},
					{sum(3), sum(4)}
				};

				auto r = fmap(fold, v);

				return r == std::vector<sum_monoid<int>>{sum(3), sum(7)};
			})
		)
	}
};

