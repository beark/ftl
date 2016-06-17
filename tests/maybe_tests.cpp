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
#include <ftl/type_functions.h>
#include "maybe_tests.h"

#include <iostream>

test_set maybe_tests{
	std::string("maybe"),
	{
		std::make_tuple(
			std::string("Preserves Eq"),
			std::function<bool()>([]() -> bool {
				auto e1 = ftl::just(10);
				auto e2 = ftl::just(10);

				return e1 == e2;
			})
		),
		std::make_tuple(
			std::string("Preserves Orderable"),
			std::function<bool()>([]() -> bool {
				auto e1 = ftl::just(10);
				auto e2 = ftl::just(12);
				ftl::maybe<int> e3 = ftl::nothing;

				return e1 < e2 && e3 < e1
					&& e2 > e1 && e1 > e3;
			})
		),
		std::make_tuple(
			std::string("Copy assignable back and forth"),
			std::function<bool()>([]() -> bool {
				auto m1 = ftl::just(10);
				ftl::maybe<int> m2 = ftl::nothing;
				auto m3 = ftl::just(15);

				ftl::maybe<int> mx(m1);

				mx = m2;

				ftl::maybe<int> my(mx);

				mx = m1;
				mx = m3;

				return mx == m3 && my == m2;
			})
		),
		/*
		std::make_tuple(
			std::string("ForwardIterable"),
			std::function<bool()>([]() -> bool {
				static_assert(ftl::ForwardIterable<ftl::maybe<int>>(), "");

				auto m1 = ftl::just(10);
				auto m2 = ftl::nothing<int>();

				for(auto& x : m1) {
					x += 2;
				}

				for(auto& x : m2) {
					x += 2;
				}

				return m1 == ftl::just(12) && m2 == ftl::nothing<int>();
			})
		),
		std::make_tuple(
			std::string("ForwardIterable[const]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				static_assert(ftl::ForwardIterable<const ftl::maybe<int>>(), "");

				const auto m1 = ftl::just(10);
				const auto m2 = ftl::nothing<int>();

				bool success = false;

				for(auto& x : m1) {
					success = (x == 10);
				}

				for(auto& x : m2) {
					// Using x to silence unused variable warnings
					success = false && x > 0;
				}

				return success;
			})
		),
		*/
		std::make_tuple(
			std::string("Pattern matching"),
			std::function<bool()>([]() -> bool {
				auto m1 = ftl::just(10);
				ftl::maybe<int> m2 = ftl::nothing;

				auto r1 = m1.match(
					[](int x) { return x; },
					[](ftl::nothing_t) { return -1; }
				);

				auto r2 = m2.match(
					[](int x) { return x; },
					[](ftl::nothing_t) { return -1; }
				);

				return r1 == 10 && r2 == -1;
			})
		),
		/*
		std::make_tuple(
			std::string("Functor<maybe>"),
			std::function<bool()>([]() -> bool {
				auto m1 = ftl::just(10);
				auto m2 = ftl::nothing<int>();

				auto r1 = ftl::fmap([](int x){ return x+1; }, m1);
				auto r2 = ftl::fmap([](int x){ return x+1; }, m2);

				return
					r1.match(
						[](int x){ return x; },
						[](ftl::Nothing){ return 0; }
					) == 11
					&& r2.is<ftl::Nothing>();
			})
		),
		std::make_tuple(
			std::string("Applicative<maybe>::pure"),
			std::function<bool()>([]() -> bool {
				auto m1 = ftl::aPure<ftl::maybe<int>>()(4);
				auto m2 = ftl::aPure<ftl::maybe<int>>()(8);

				return m1.unsafe_get<int>() == 4 && m2.unsafe_get<int>() == 8;
			})
		),
		std::make_tuple(
			std::string("Applicative<maybe>::apply"),
			std::function<bool()>([]() -> bool {

				auto f = [](int x, int y){ return x+y; };

				auto m1 = ftl::just(12);
				auto m2 = ftl::just(5);
				auto m3 = ftl::nothing<int>();

				auto mf = ftl::fmap(ftl::curry(f), m1);

				auto r1 = ftl::aapply(mf, m2);
				auto r2 = ftl::aapply(mf, m3);

				return
					r1.match(
						[](int x){ return x; },
						[](ftl::Nothing){ return 0; }
					) == 17
					&& r2.is<ftl::Nothing>();
			})
		),
		std::make_tuple(
			std::string("Monad<maybe>::bind"),
			std::function<bool()>([]() -> bool {
				using ftl::operator<<=;

				auto f1 = [](int x){ return ftl::just(x/2); };
				auto f2 = [](int){ return ftl::nothing<int>(); };

				auto m1 = ftl::just(10);
				auto m2 = ftl::nothing<int>();

				auto r1 = f1 <<= m1;
				auto r2 = f1 <<= m2;
				auto r3 = f2 <<= m1;
				auto r4 = f2 <<= m2;

				return r1.is<int>() && r1.unsafe_get<int>() == 5
					&& r2.is<ftl::Nothing>()
					&& r3.is<ftl::Nothing>()
					&& r4.is<ftl::Nothing>();
			})
		),
		std::make_tuple(
			std::string("monoid::mappend[id]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(sum(10));
				auto m2 = nothing<sum_monoid<int>>();

				auto r1 = m1 ^ m2;
				auto r2 = m2 ^ m1;

				r1.match(
					[](sum_monoid<int> s){ std::cout << s.n << std::endl; },
					[](otherwise){}
				);

				r2.match(
					[](sum_monoid<int> s){ std::cout << s.n << std::endl; },
					[](otherwise){}
				);

				return r1 == m1 && r2 == m1;
			})
		),
		std::make_tuple(
			std::string("monoid::mappend"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(sum(10));
				auto m2 = just(sum(5));

				auto r1 = m1 ^ m2;
				auto r2 = m2 ^ m1;

				return r1 == just(sum(15)) && r2 == just(sum(15));
			})
		),
		std::make_tuple(
			std::string("foldable::foldl"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(10);
				auto m2 = nothing<int>();
				auto f = [](int x, int y){ return x+y; };

				return foldl(f, 4, m1) == 14
					&& foldl(f, 4, m2) == 4;
			})
		),
		std::make_tuple(
			std::string("foldable::foldr"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(10);
				auto m2 = nothing<int>();
				auto f = [](int x, int y){ return x+y; };

				return foldr(f, 4, m1) == 14
					&& foldr(f, 4, m2) == 4;
			})
		),
		*/
		// GCC: internal compiler error: in nothrow_spec_p at cp/except.c:1263
		/*
		std::make_tuple(
			std::string("foldable::foldMap"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(2);
				auto m2 = nothing<int>();


				return foldMap(prod<int>, m1) == 2 && foldMap(prod<int>, m2) == 1;
			})
		),
		*/
		/*
		std::make_tuple(
			std::string("foldable::fold"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = just(prod(2));
				auto m2 = nothing<prod_monoid<int>>();


				return fold(m1) == 2 && fold(m2) == 1;
			})
		)
		*/
	}
};

