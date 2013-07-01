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
#ifndef FTL_MAYBE_TESTS_H
#define FTL_MAYBE_TESTS_H

#include <ftl/maybe.h>
#include "base.h"

test_set maybe_tests{
	std::string("maybe"),
	{
		std::make_tuple(
			std::string("Preserves Eq"),
			std::function<bool()>([]() -> bool {
				auto e1 = ftl::value(10);
				auto e2 = ftl::value(10);

				return e1 == e2;
			})
		),
		std::make_tuple(
			std::string("Copy assignable back and forth"),
			std::function<bool()>([]() -> bool {
				auto m1 = ftl::value(10);
				auto m2 = ftl::maybe<int>{};
				auto m3 = ftl::value(15);

				ftl::maybe<int> mx(m1);

				mx = m2;

				ftl::maybe<int> my(mx);

				mx = m1;
				mx = m3;

				return mx == m3 && my == m2;
			})
		),
		std::make_tuple(
			std::string("Method access works"),
			std::function<bool()>([]() -> bool {
				auto m = ftl::value(std::string("test"));
				std::string s("test");

				return m->size() == s.size();
			})
		),
		std::make_tuple(
			std::string("Method access throws on `nothing'"),
			std::function<bool()>([]() -> bool {
				auto m = ftl::maybe<std::string>{};
				try {
					m->size();
				}
				catch(std::logic_error& e) {
					return true;
				}

				return false;
			})
		),
		std::make_tuple(
			std::string("monoid::id"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				return monoid<maybe<sum_monoid<int>>>::id() == nothing;
			})
		),
		std::make_tuple(
			std::string("monoid::append[value,value]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = value(sum(2));
				auto m2 = value(sum(3));

				return (m1 ^ m2) == value(sum(5));
			})
		),
		std::make_tuple(
			std::string("monoid::append[value,nothing]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = value(sum(2));
				maybe<sum_monoid<int>> m2;

				return (m1 ^ m2) == m1;
			})
		),
		std::make_tuple(
			std::string("monoid::append[nothing,value]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybe<sum_monoid<int>> m1;
				auto m2 = value(sum(3));

				return (m1 ^ m2) == m2;
			})
		),
		std::make_tuple(
			std::string("monoid::append[nothing,nothing]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybe<sum_monoid<int>> m1;
				maybe<sum_monoid<int>> m2;

				return (m1 ^ m2) == nothing;
			})
		),
		std::make_tuple(
			std::string("functor::map[value] on lvalues"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto m = ftl::value(10);
				ftl::maybe<std::string> m2 = [](int){ return std::string("test"); } % m;

				return *m2 == std::string("test");
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {
				auto m = ftl::applicative<ftl::maybe<float>>::pure(12.f);

				return *m == 12.f;
			})
		),
		std::make_tuple(
			std::string("applicative::apply[value,value]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator %;
				using ftl::operator *;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto m = fn % ftl::value(1) * ftl::value(1);

				return *m == 2;
			})
		),
		std::make_tuple(
			std::string("applicative::apply[nothing,value]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator %;
				using ftl::operator *;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto m = fn % ftl::maybe<int>{} * ftl::value(1);

				return m.isNothing();
			})
		),
		std::make_tuple(
			std::string("applicative::apply[value,nothing]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator %;
				using ftl::operator *;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto m = fn % ftl::value(1) * ftl::maybe<int>{};

				return m.isNothing();
			})
		),
		std::make_tuple(
			std::string("applicative::apply[nothing,nothing]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator %;
				using ftl::operator *;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto m = fn % ftl::maybe<int>{} * ftl::maybe<int>{};

				return m.isNothing();
			})
		),
		std::make_tuple(
			std::string("monad::bind[value,->value]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator %;
				using ftl::operator *;

				auto fn = [](int x){ return ftl::value(x+1); };

				auto m = ftl::value(1) >>= fn;

				return *m == 2;
			})
		),
		std::make_tuple(
			std::string("monad::bind[nothing,->value]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator %;
				using ftl::operator *;

				auto fn = [](int x){ return ftl::value(x+1); };

				auto m = ftl::maybe<int>{} >>= fn;

				return m.isNothing();
			})
		),
		std::make_tuple(
			std::string("monad::bind[value,->nothing]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator %;
				using ftl::operator *;

				auto fn = [](int){ return ftl::maybe<int>{}; };

				auto m = ftl::value(1) >>= fn;

				return m.isNothing();
			})
		),
		std::make_tuple(
			std::string("monoidA::fail"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				return monoidA<maybe<int>>::fail() == nothing;
			})
		),
		std::make_tuple(
			std::string("monoidA::orDo[value,value]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto a = value(1);
				auto b = value(2);

				return (a | b) == value(1);
			})
		),
		std::make_tuple(
			std::string("monoidA::orDo[value,nothing]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto a = value(1);
				maybe<int> b;

				return (a | b) == value(1);
			})
		),
		std::make_tuple(
			std::string("monoidA::orDo[nothing,value]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybe<int> a;
				auto b = value(2);

				return (a | b) == value(2);
			})
		),
		std::make_tuple(
			std::string("monoidA::orDo[nothing,nothing]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybe<int> a;
				maybe<int> b;

				return (a | b) == nothing;
			})
		)
	}
};

#endif


