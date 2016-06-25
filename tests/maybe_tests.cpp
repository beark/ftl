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
#include <ftl/maybe.h>
#include <ftl/type_functions.h>
#include "maybe_tests.h"

#include <iostream>

test_set maybe_tests{
	std::string("maybe"),
	{
		std::make_tuple(
			std::string("Preserves Eq"),
			[] {
				auto e1 = ftl::just(10);
				auto e2 = ftl::just(10);

				TEST_ASSERT(e1 == e2);
			}
		),
		std::make_tuple(
			std::string("Preserves Orderable"),
			[] {
				auto e1 = ftl::just(10);
				auto e2 = ftl::just(12);
				ftl::maybe<int> e3 = ftl::nothing;

				TEST_ASSERT(e1 < e2);
				TEST_ASSERT(e3 < e1);
				TEST_ASSERT(e2 > e1);
				TEST_ASSERT(e1 > e3);
			}
		),
		std::make_tuple(
			std::string("Copy assignable back and forth"),
			[] {
				auto m1 = ftl::just(10);
				ftl::maybe<int> m2 = ftl::nothing;
				auto m3 = ftl::just(15);

				ftl::maybe<int> mx(m1);

				mx = m2;

				ftl::maybe<int> my(mx);

				mx = m1;
				mx = m3;

				TEST_ASSERT(mx == m3);
				TEST_ASSERT(my == m2);
			}
		),
		std::make_tuple(
			std::string("ForwardIterable"),
			[] {
				using namespace ftl;

				static_assert(ForwardIterable<maybe<int>>(), "");

				auto m1 = just(10);
				maybe<int> m2 = nothing;

				for(auto& x : m1) {
					x += 2;
				}

				for(auto& x : m2) {
					x += 2;
				}

				TEST_ASSERT(m1 == just(12));
				TEST_ASSERT(m2 == nothing);
			}
		),
		std::make_tuple(
			std::string("ForwardIterable[const]"),
			[] {
				using namespace ftl;

				static_assert(ForwardIterable<const ftl::maybe<int>>(), "");

				const auto m1 = just(10);
				const maybe<int> m2 = nothing;

				for(auto& x : m1) {
					TEST_ASSERT(x == 10);
				}

				for(auto& x : m2) {
					// Using x to silence unused variable warnings
					TEST_ASSERT(false && x > 0);
				}
			}
		),
		std::make_tuple(
			std::string("Pattern matching"),
			[] {
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

				TEST_ASSERT(r1 == 10);
				TEST_ASSERT(r2 == -1);
			}
		),
		std::make_tuple(
			std::string("Functor<maybe>"),
			[] {
				auto m1 = ftl::just(10);
				ftl::maybe<int> m2 = ftl::nothing;

				auto r1 = ftl::fmap([](int x){ return x+1; }, m1);
				auto r2 = ftl::fmap([](int x){ return x+1; }, m2);

				r1.match(
					[](int x){ TEST_ASSERT(x == 11); },
					[](ftl::nothing_t){ TEST_ASSERT(false); }
				);

				TEST_ASSERT(r2 == ftl::nothing);
			}
		),
		std::make_tuple(
			std::string("Applicative<maybe>::pure"),
			[] {
				auto m1 = ftl::aPure<ftl::maybe<int>>()(4);
				auto m2 = ftl::aPure<ftl::maybe<int>>()(8);

				TEST_ASSERT(m1.unsafe_get<int>() == 4);
				TEST_ASSERT(m2.unsafe_get<int>() == 8);
			}
		),
		std::make_tuple(
			std::string("Applicative<maybe>::apply"),
			[] {
				using namespace ftl;

				auto f = [](int x, int y){ return x+y; };

				auto m1 = just(12);
				auto m2 = just(5);
				maybe<int> m3 = nothing;

				auto mf = fmap(curry(f), m1);

				auto r1 = aapply(mf, m2);
				auto r2 = aapply(mf, m3);

				r1.match(
					[](int x){ TEST_ASSERT(x == 17); },
					[](nothing_t){ TEST_ASSERT(false); }
				);

				TEST_ASSERT(r2 == nothing);
			}
		),
		std::make_tuple(
			std::string("Monad<maybe>::bind"),
			[] {
				using namespace ftl;
				using ftl::operator<<=;

				auto f1 = [](int x){ return just(x/2); };
				auto f2 = [](int) -> maybe<int> { return nothing; };

				auto m1 = just(10);
				maybe<int> m2 = nothing;

				auto r1 = f1 <<= m1;
				auto r2 = f1 <<= m2;
				auto r3 = f2 <<= m1;
				auto r4 = f2 <<= m2;

				TEST_ASSERT(r1.is<int>());
				TEST_ASSERT(r1.unsafe_get<int>() == 5);
				TEST_ASSERT(r2 == nothing);
				TEST_ASSERT(r3 == nothing);
				TEST_ASSERT(r4 == nothing);
			}
		),
		std::make_tuple(
			std::string("monoid::mappend[id]"),
			[] {
				using namespace ftl;

				auto m1 = just(sum(10));
				maybe<sum_monoid<int>> m2 = nothing;

				auto r1 = m1 ^ m2;
				auto r2 = m2 ^ m1;

				TEST_ASSERT(r1 == m1);
				TEST_ASSERT(r2 == m1);
			}
		),
		std::make_tuple(
			std::string("monoid::mappend"),
			[] {
				using namespace ftl;

				auto m1 = just(sum(10));
				auto m2 = just(sum(5));

				auto r1 = m1 ^ m2;
				auto r2 = m2 ^ m1;

				TEST_ASSERT(r1 == just(sum(15)));
				TEST_ASSERT(r2 == just(sum(15)));
			}
		),
		std::make_tuple(
			std::string("foldable::foldl"),
			[] {
				using namespace ftl;

				auto m1 = just(10);
				maybe<int> m2 = nothing;
				auto f = [](int x, int y){ return x+y; };

				TEST_ASSERT( (foldl(f, 4, m1) == 14) );
				TEST_ASSERT( (foldl(f, 4, m2) == 4) );
			}
		),
		std::make_tuple(
			std::string("foldable::foldr"),
			[] {
				using namespace ftl;

				auto m1 = just(10);
				maybe<int> m2 = nothing;
				auto f = [](int x, int y){ return x+y; };

				TEST_ASSERT( (foldr(f, 4, m1) == 14) );
				TEST_ASSERT( (foldr(f, 4, m2) == 4) );
			}
		),
		/* gcc5 ICE: in nothrow_spec_p
		std::make_tuple(
			std::string("foldable::foldMap"),
			[] {
				using namespace ftl;

				auto m1 = just(2);
				maybe<int> m2 = nothing;

				TEST_ASSERT( (foldMap(prod<int>, m1) == 2) );
				TEST_ASSERT( (foldMap(prod<int>, m2) == 1) );
			}
		),
		*/
		std::make_tuple(
			std::string("foldable::fold"),
			[] {
				using namespace ftl;

				auto m1 = just(prod(2));
				maybe<prod_monoid<int>> m2 = nothing;

				TEST_ASSERT(fold(m1) == 2);
				TEST_ASSERT(fold(m2) == 1);
			}
		)
	}
};

