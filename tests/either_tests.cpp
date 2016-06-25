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
#include <ftl/either.h>
#include "either_tests.h"

static_assert(std::is_trivial<ftl::Left<int>>::value, "Left container should be trivial when encapsulating trivial types");
static_assert(std::is_trivial<ftl::Right<int>>::value, "Right container should be trivial when encapsulating trivial types");

test_set either_tests{
	std::string("either"),
	{
		std::make_tuple(
			std::string("Preserves Eq[L]"),
			[] {
				auto e1 = ftl::make_left<int>(10);
				auto e2 = ftl::make_left<int>(10);

				TEST_ASSERT(e1 == e2)
				TEST_ASSERT(!(e1 != e2));
			}
		),
		std::make_tuple(
			std::string("Preserves Eq[R]"),
			[] {
				auto e1 = ftl::make_right<int>(10);
				auto e2 = ftl::make_right<int>(10);

				TEST_ASSERT(e1 == e2);
				TEST_ASSERT(!(e1 != e2));
			}
		),
		std::make_tuple(
			std::string("Copy assignable back and forth"),
			[] {
				auto e1 = ftl::make_right<int>(10);
				auto e2 = ftl::make_left<int>(5);
				auto e3 = ftl::make_right<int>(15);

				ftl::either<int,int> ex(e1);

				ex = e2;

				ftl::either<int,int> ey(ex);

				ex = e1;
				ex = e3;

				TEST_ASSERT(ex == e3);
				TEST_ASSERT(ey == e2);
			}
		),
		std::make_tuple(
			std::string("Basic pattern matching"),
			[] {
				auto e = ftl::make_right<int>(std::string("test"));
				std::string s("test");

				e.match(
					[s](ftl::Right<std::string> x) { TEST_ASSERT(x.val == s); },
					[](ftl::Left<int>){ TEST_ASSERT(false); }
				);
			}
		),
		std::make_tuple(
			std::string("Effectful pattern matching[R]"),
			[] {
				auto e = ftl::make_right<std::string>(std::string("test"));

				e.match(
					[](ftl::Right<std::string>& x) { *x += " test"; },
					[](ftl::Left<std::string>&){  }
				);

				e.match(
					[](ftl::Right<std::string> x) { TEST_ASSERT(*x == "test test"); },
					[](ftl::Left<std::string>){ TEST_ASSERT(false); }
				);
			}
		),
		std::make_tuple(
			std::string("Effectful pattern matching[L]"),
			[] {
				auto e = ftl::make_left<std::string>(std::string("test"));

				e.match(
					[](ftl::Right<std::string>& x) { *x += " fail"; },
					[](ftl::Left<std::string>& l){ *l += " test";  }
				);

				e.match(
					[](ftl::Right<std::string>) { TEST_ASSERT(false); },
					[](ftl::Left<std::string> l){ TEST_ASSERT(*l == "test test"); }
				);
			}
		),
		std::make_tuple(
			std::string("functor::map[R&]"),
			[] {
				using ftl::operator%;
				auto e = ftl::make_right<int>(10);
				ftl::either<int,std::string> e2 =
					[](int){ return std::string("test"); } % e;

				e2.match(
					[](ftl::Right<std::string> r){ TEST_ASSERT(*r == std::string("test")); },
					[](ftl::Left<int>){ TEST_ASSERT(false); }
				);
			}
		),
		std::make_tuple(
			std::string("functor::map[a->b,R&&]"),
			[] {
				using namespace ftl;
				auto f = [](NoCopy&& n) { return n.property; };

				auto e = f % make_right<char>(NoCopy(2));

				e.match(
					[](ftl::Right<int> x){ TEST_ASSERT(*x == 2); },
					[](Left<char>){ TEST_ASSERT(false); }
				);
			}
		),
		std::make_tuple(
			std::string("functor::map[a->b,L&]"),
			[] {
				using ftl::operator%;
				auto e = ftl::make_left<int>(10);
				ftl::either<int,std::string> e2 =
					[](int){ return std::string("test"); } % e;

				e2.match(
					[](ftl::Right<std::string>){ TEST_ASSERT(false); },
					[](ftl::Left<int> l){ TEST_ASSERT(*l == 10); }
				);
			}
		),
		std::make_tuple(
			std::string("functor::map[a->b,L&&]"),
			[] {
				using namespace ftl;
				auto f = [](NoCopy&& n) { return n.property; };

				auto e = f % make_left<NoCopy>('a');

				e.match(
					[](Right<int>){ TEST_ASSERT(false); },
					[](Left<char> c){ TEST_ASSERT(*c == 'a'); }
				);
			}
		),
		std::make_tuple(
			std::string("applicative::pure"),
			[] {
				auto e = ftl::applicative<ftl::either<std::string,float>>::pure(12.f);

				e.match(
					[](ftl::Left<std::string>){ TEST_ASSERT(false); },
					[](ftl::Right<float> x){ TEST_ASSERT(fequal(*x, 12.f)); }
				);
			}
		),
		std::make_tuple(
			std::string("applicative::apply[R,R]"),
			[] {
				using ftl::operator%;
				using ftl::operator*;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto e = fn % ftl::make_right<int>(1) * ftl::make_right<int>(1);

				e.match(
					[](ftl::Left<int>){ TEST_ASSERT(false); },
					[](ftl::Right<int> r) { TEST_ASSERT(r == 2); }
				);
			}
		),
		std::make_tuple(
			std::string("applicative::apply[L,R]"),
			[] {
				using ftl::operator%;
				using ftl::operator*;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto e = fn % ftl::make_left<int>(1) * ftl::make_right<int>(1);

				e.match(
					[](ftl::Right<int>){ TEST_ASSERT(false); },
					[](ftl::Left<int> l){ TEST_ASSERT(*l == 1); }
				);
			}
		),
		std::make_tuple(
			std::string("applicative::apply[R,L]"),
			[] {
				using ftl::operator%;
				using ftl::operator*;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto e = fn % ftl::make_right<int>(1) * ftl::make_left<int>(1);

				e.match(
					[](ftl::Right<int>){ TEST_ASSERT(false); },
					[](ftl::Left<int> l){ TEST_ASSERT(*l == 1); }
				);
			}
		),
		std::make_tuple(
			std::string("applicative::apply[L,L]"),
			[] {
				using ftl::operator%;
				using ftl::operator*;
				auto fn = [](int x){ return [x](int y){ return x+y; }; };
				auto e = fn % ftl::make_left<int>(1) * ftl::make_left<int>(1);

				e.match(
					[](ftl::Left<int> l){ TEST_ASSERT(*l == 1); },
					[](ftl::Right<int>){ TEST_ASSERT(false); }
				);
			}
		),
		std::make_tuple(
			std::string("monad::bind[R,->R]"),
			[] {
				using ftl::operator>>=;

				auto fn = [](int x){ return ftl::make_right<int>(x+1); };

				auto e = ftl::make_right<int>(1) >>= fn;

				e.match(
					[](ftl::Left<int>){ TEST_ASSERT(false); },
					[](ftl::Right<int> r){ TEST_ASSERT(*r == 2); }
				);
			}
		),
		std::make_tuple(
			std::string("monad::bind[L,->R]"),
			[] {
				using ftl::operator>>=;

				auto fn = [](int x){ return ftl::make_right<int>(x+1); };

				auto e = ftl::make_left<int>(1) >>= fn;

				e.match(
					[](ftl::Left<int> l){ TEST_ASSERT(*l == 1); },
					[](ftl::Right<int>){ TEST_ASSERT(false); }
				);
			}
		),
		std::make_tuple(
			std::string("monad::bind[R,->L]"),
			[] {
				using ftl::operator>>=;

				auto fn = [](int x){ return ftl::make_left<int>(x+1); };

				auto e = ftl::make_right<int>(1) >>= fn;

				e.match(
					[](ftl::Left<int> l){ TEST_ASSERT(*l == 2); },
					[](ftl::Right<int>){ TEST_ASSERT(false); }
				);
			}
		),
		std::make_tuple(
			std::string("monad::join[R<R>]"),
			[] {
				using namespace ftl;

				auto e = make_right<int>(make_right<int>(2));

				TEST_ASSERT((monad<either<int,int>>::join(e) == make_right<int>(2)));
			}
		),
		std::make_tuple(
			std::string("monad::join[R<L>]"),
			[] {
				using namespace ftl;

				auto e = make_right<int>(make_left<int>(2));

				TEST_ASSERT((monad<either<int,int>>::join(e) == make_left<int>(2)));
			}
		),
		std::make_tuple(
			std::string("monad::join[L<_>]"),
			[] {
				using namespace ftl;

				auto e = make_left<either<int,int>>(2);

				TEST_ASSERT((monad<either<int,int>>::join(e) == make_left<int>(2)));
			}
		)
	}
};

