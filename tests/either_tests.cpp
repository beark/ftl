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
#include <ftl/either.h>
#include "either_tests.h"

static_assert(std::is_trivial<ftl::Left<int>>::value, "Left container should be trivial when encapsulating trivial types");
static_assert(std::is_trivial<ftl::Right<int>>::value, "Right container should be trivial when encapsulating trivial types");

test_set either_tests{
	std::string("either"),
	{
		std::make_tuple(
			std::string("Left type tests"),
			std::function<bool()>([]() -> bool {
				auto e1 = ftl::make_left<int>(10);
				auto e2 = ftl::make_left<int>(10);

				return e1 == e2 && !(e1 != e2);
			})
		),
		std::make_tuple(
			std::string("Preserves Eq[L]"),
			std::function<bool()>([]() -> bool {
				auto e1 = ftl::make_left<int>(10);
				auto e2 = ftl::make_left<int>(10);

				return e1 == e2 && !(e1 != e2);
			})
		),
		std::make_tuple(
			std::string("Preserves Eq[R]"),
			std::function<bool()>([]() -> bool {
				auto e1 = ftl::make_right<int>(10);
				auto e2 = ftl::make_right<int>(10);

				return e1 == e2 && !(e1 != e2);
			})
		),
		std::make_tuple(
			std::string("Copy assignable back and forth"),
			std::function<bool()>([]() -> bool {
				auto e1 = ftl::make_right<int>(10);
				auto e2 = ftl::make_left<int>(5);
				auto e3 = ftl::make_right<int>(15);

				ftl::either<int,int> ex(e1);

				ex = e2;

				ftl::either<int,int> ey(ex);

				ex = e1;
				ex = e3;

				return ex == e3 && ey == e2;
			})
		),
		std::make_tuple(
			std::string("Basic pattern matching"),
			std::function<bool()>([]() -> bool {
				auto e = ftl::make_right<int>(std::string("test"));
				std::string s("test");

				return e.match(
					[s](ftl::Right<std::string> x) { return x.val == s; },
					[](ftl::Left<int>){ return false; }
				);
			})
		),
		std::make_tuple(
			std::string("Effectful pattern matching[R]"),
			std::function<bool()>([]() -> bool {
				auto e = ftl::make_right<std::string>(std::string("test"));

				e.match(
					[](ftl::Right<std::string>& x) { *x += " test"; },
					[](ftl::Left<std::string>&){  }
				);

				return e.match(
					[](ftl::Right<std::string> x) { return *x == "test test"; },
					[](ftl::Left<std::string>){ return false; }
				);
			})
		),
		std::make_tuple(
			std::string("Effectful pattern matching[L]"),
			std::function<bool()>([]() -> bool {
				auto e = ftl::make_left<std::string>(std::string("test"));

				e.match(
					[](ftl::Right<std::string>& x) { *x += " fail"; },
					[](ftl::Left<std::string>& l){ *l += " test";  }
				);

				return e.match(
					[](ftl::Right<std::string>) { return false; },
					[](ftl::Left<std::string> l){ return *l == "test test"; }
				);
			})
		),
		std::make_tuple(
			std::string("functor::map[R&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				auto e = ftl::make_right<int>(10);
				ftl::either<int,std::string> e2 =
					[](int){ return std::string("test"); } % e;

				return e2.match(
					[](ftl::Right<std::string> r){ return *r == std::string("test"); },
					[](ftl::Left<int>){ return false; }
				);
			})
		),
		std::make_tuple(
			std::string("functor::map[a->b,R&&]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;
				auto f = [](NoCopy&& n) { return n.property; };

				auto e = f % make_right<char>(NoCopy(2));

				return e.match(
					[](ftl::Right<int> x){ return *x == 2; },
					[](Left<char>){ return false; }
				);
			})
		),
		std::make_tuple(
			std::string("functor::map[a->b,L&]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				auto e = ftl::make_left<int>(10);
				ftl::either<int,std::string> e2 =
					[](int){ return std::string("test"); } % e;

				return e2.match(
					[](ftl::Right<std::string>){ return false; },
					[](ftl::Left<int> l){ return *l == 10; }
				);
			})
		),
		std::make_tuple(
			std::string("functor::map[a->b,L&&]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;
				auto f = [](NoCopy&& n) { return n.property; };

				auto e = f % make_left<NoCopy>('a');

				return e.match(
					[](Right<int>){ return false; },
					[](Left<char> c){ return *c == 'a'; }
				);
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {
				auto e = ftl::applicative<ftl::either<std::string,float>>::pure(12.f);

				return e.match(
					[](ftl::Left<std::string>){ return false; },
					[](ftl::Right<float> x){ return fequal(*x, 12.f); }
				);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[R,R]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using ftl::operator*;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto e = fn % ftl::make_right<int>(1) * ftl::make_right<int>(1);

				return e.match(
					[](ftl::Left<int>){ return false; },
					[](ftl::Right<int> r) { return r == 2; }
				);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[L,R]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using ftl::operator*;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto e = fn % ftl::make_left<int>(1) * ftl::make_right<int>(1);

				return e.match(
					[](ftl::Right<int>){ return false; },
					[](ftl::Left<int> l){ return *l == 1; }
				);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[R,L]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using ftl::operator*;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto e = fn % ftl::make_right<int>(1) * ftl::make_left<int>(1);

				return e.match(
					[](ftl::Right<int>){ return false; },
					[](ftl::Left<int> l){ return *l == 1; }
				);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[L,L]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using ftl::operator*;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto e = fn % ftl::make_left<int>(1) * ftl::make_left<int>(1);

				return e.match(
					[](ftl::Left<int> l){ return *l == 1; },
					[](ftl::Right<int>){ return false; }
				);
			})
		),
		std::make_tuple(
			std::string("monad::bind[R,->R]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				auto fn = [](int x){ return ftl::make_right<int>(x+1); };

				auto e = ftl::make_right<int>(1) >>= fn;

				return e.match(
					[](ftl::Left<int>){ return false; },
					[](ftl::Right<int> r){ return *r == 2; }
				);
			})
		),
		std::make_tuple(
			std::string("monad::bind[L,->R]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				auto fn = [](int x){ return ftl::make_right<int>(x+1); };

				auto e = ftl::make_left<int>(1) >>= fn;

				return e.match(
					[](ftl::Left<int> l){ return *l == 1; },
					[](ftl::Right<int>){ return false; }
				);
			})
		),
		std::make_tuple(
			std::string("monad::bind[R,->L]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator>>=;

				auto fn = [](int x){ return ftl::make_left<int>(x+1); };

				auto e = ftl::make_right<int>(1) >>= fn;

				return e.match(
					[](ftl::Left<int> l){ return *l == 2; },
					[](ftl::Right<int>){ return false; }
				);
			})
		),
		std::make_tuple(
			std::string("monad::join[R<R>]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto e = make_right<int>(make_right<int>(2));

				return monad<either<int,int>>::join(e) == make_right<int>(2);
			})
		),
		std::make_tuple(
			std::string("monad::join[R<L>]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto e = make_right<int>(make_left<int>(2));

				return monad<either<int,int>>::join(e) == make_left<int>(2);
			})
		),
		std::make_tuple(
			std::string("monad::join[L<_>]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto e = make_left<either<int,int>>(2);

				return monad<either<int,int>>::join(e) == make_left<int>(2);
			})
		)
	}
};

