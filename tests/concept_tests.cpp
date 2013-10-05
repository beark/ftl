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
#include "concept_tests.h"

test_set concept_tests{
	std::string("concepts"),
	{
		std::make_tuple(
			std::string("Monoid: curried mappend"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = value(sum(2));
				auto m2 = value(sum(2));

				auto m3 = mappend % m1 * m2;

				return m3 == value(sum(4));
			})
		),
		std::make_tuple(
			std::string("Functor: curried fmap"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = [](int x){ return x+1; };
				auto fm = fmap(f);

				auto m = fm(value(2));

				return m == value(3);
			})
		),
		std::make_tuple(
			std::string("Applicative: curried aapply"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto mf = value(function<int(int)>([](int x){
						return x+1;
				}));

				auto ap = aapply(mf);

				return ap(value(2)) == value(3);
			})
		),
		std::make_tuple(
			std::string("Monad: curried mbind"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m = value(2);
				auto cbind = mbind(m);
				auto f = [](int x){ return value(x+1); };

				return cbind(f) == value(3);
			})
		),
		std::make_tuple(
			std::string("Monad: >>="),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = value(2);

				auto f1 = [](int x){ return value(float(x)*0.5f); };
				auto f2 = [](float x){ return value(int(x*3.33f)); };

				return
					((m1 >>= f1) >>= f2) == value(3);
			})
		),
		std::make_tuple(
			std::string("Monad: >>"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = value(1);
				auto m2 = value(2);
				maybe<int> m3 = nothing;

				return m1 >> m2 == value(2) && m3 >> m1 == nothing;
			})
		),
		std::make_tuple(
			std::string("Monad: <<"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = value(1);
				auto m2 = value(2);
				maybe<int> m3 = nothing;

				return m1 << m2 == value(1) && m1 << m3 == nothing;
			})
		),
		std::make_tuple(
			std::string("Monad: <<"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = value(1);
				auto m2 = value(2);
				maybe<int> m3 = nothing;

				return m1 << m2 == value(1) && m1 << m3 == nothing;
			})
		),
		std::make_tuple(
			std::string("Monad: mixed, non-trivial sequence"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto plusOne = [](int x){ return value(x+1); };
				auto mulTwo = [](int x){ return value(2*x); };

				maybe<int> mNothing;
				auto mOne = value(1);

				auto m1 = ((mOne >>= plusOne) >>= mulTwo) << (mulTwo <<= mOne);

				return m1 == value(4);
			})
		),
		std::make_tuple(
			std::string("Foldable: curried foldMap"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = [](int x){ return sum(x); };
				auto foldmap = foldMap(f);

				std::vector<int> v{3,3,4};

				return foldmap(v) == 10;
			})
		),
		std::make_tuple(
			std::string("Foldable: curried foldr"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = [](int x, int y){ return x+y; };

				auto foldr2 = foldr(f);
				auto foldr1 = foldr2(0);

				std::vector<int> v{3,3,4};

				return
					foldr(f, 0, v) == 10
					&& foldr2(0, v) == 10
					&& foldr1(v) == 10;
			})
		),
		std::make_tuple(
			std::string("Foldable: curried foldl"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto f = [](int x, int y){ return x+y; };

				auto foldl2 = foldl(f);
				auto foldl1 = foldl2(0);

				std::vector<int> v{3,3,4};

				return
					foldl(f, 0, v) == 10
					&& foldl2(0, v) == 10
					&& foldl1(v) == 10;
			})
		)
	}
};

