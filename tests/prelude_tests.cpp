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
#include <ftl/prelude.h>
#include <ftl/maybe.h>
#include "prelude_tests.h"

int curry_me(int x, int y) {
	return x+y;
}

// Test make_curried_n with arbitrarily large function.
struct _curry5 : public ftl::make_curried_n<5,_curry5> {
    template<typename P1, typename P2, typename P3, typename P4, typename P5>
    auto operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const
    -> decltype(p1+p2+p3+p4+p5) {
        return p1+p2+p3+p4+p5;
    }

    using ftl::make_curried_n<5,_curry5>::operator();
} curry5;

test_set prelude_tests{
	std::string("prelude"),
	{
		std::make_tuple(
			std::string("identity function object"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto m = ftl::id % ftl::value(10);

				return m && *m == 10;
			})
		),
		std::make_tuple(
			std::string("const_ function object"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;

				auto m = ftl::const_(42) % ftl::value(3);

				return m && *m == 42;
			})
		),
		std::make_tuple(
			std::string("Eq<Identity>"),
			std::function<bool()>([]() -> bool {

				ftl::Identity<int> x{10};
				ftl::Identity<int> y{12};

				return x == x && x != y;
			})
		),
		std::make_tuple(
			std::string("Orderable<Identity>"),
			std::function<bool()>([]() -> bool {

				ftl::Identity<int> x{10};
				ftl::Identity<int> y{12};
				ftl::Identity<int> z{4};

				return x > z && x < y && x >= x && x <= x;
			})
		),
		std::make_tuple(
			std::string("Functor<Identity>"),
			std::function<bool()>([]() -> bool {

				ftl::Identity<int> x{10};

				auto y = ftl::fmap([](int x){ return x/2; }, x);

				return y.val == 5;
			})
		),
		std::make_tuple(
			std::string("Applicative<Identity>"),
			std::function<bool()>([]() -> bool {

				ftl::Identity<int> x{10};
				auto f = [](int x, int y){ return x+y; };

				auto y = ftl::fmap(ftl::curry<2>(f), x);
				auto z = ftl::aapply(y, x);

				return z.val == 20;
			})
		),
		std::make_tuple(
			std::string("Monad<Identity>::join"),
			std::function<bool()>([]() -> bool {
				using ftl::Identity;

				Identity<int> x{10};
				auto y = Identity<Identity<int>>{x};
				auto z = ftl::monad<Identity<int>>::join(y);

				return x == z;
			})
		),
		std::make_tuple(
			std::string("Monad<Identity>::bind"),
			std::function<bool()>([]() -> bool {
				using ftl::Identity;

				Identity<int> x{10};
				auto f = [](int x){ return Identity<int>{x/2}; };

				return (x >>= f).val == 5;
			})
		),
		std::make_tuple(
			std::string("tuple_apply[&]"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x, int y){ return x+y; };
				auto t = std::make_tuple(4,4);

				auto r = ftl::tuple_apply(f,t);

				return r == 8;
			})
		),
		std::make_tuple(
			std::string("curried tuple_apply"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x, int y){ return x+y; };
				auto t = std::make_tuple(4,4);

				auto tup_ap = ftl::tuple_apply(f);
				auto r = tup_ap(t);

				return r == 8;
			})
		),
		std::make_tuple(
			std::string("tuple_apply[&&]"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x, int y){ return x+y; };

				auto r = ftl::tuple_apply(f,std::make_tuple(4,4));

				return r == 8;
			})
		),
		std::make_tuple(
			std::string("currying regular functions"),
			std::function<bool()>([]() -> bool {
				auto f = ftl::curry(curry_me);
				return f(2)(2) == f(2,2) && f(2,2) == curry_me(2,2);
			})
		),
		std::make_tuple(
			std::string("currying std::function"),
			std::function<bool()>([]() -> bool {
				std::function<int(int,int)> f = [](int x, int y) { return x+y; };
				auto g = ftl::curry(f);

				return g(2)(2) == g(2,2) && g(2,2) == f(2,2);
			})
		),
		std::make_tuple(
			std::string("currying generic function object"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x, int y, int z){ return x+y+z; };
				auto g = ftl::curry(f);

				return g(2)(2)(2) == g(2,2)(2)
					&& g(3,3)(3) == g(3,3,3)
					&& g(2,3,4) == f(2,3,4);
			})
		),
		std::make_tuple(
			std::string("currying n-ary function"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x, int y, int z){ return x+y+z; };
				auto g = ftl::curry<3>(f);

				return g(2)(2)(2) == g(2,2)(2)
					&& g(3,3)(3) == g(3,3,3)
					&& g(2,3,4) == f(2,3,4);
			})
		),
		std::make_tuple(
			std::string("curried n-ary function object"),
			std::function<bool()>([]() -> bool {
				return curry5(1)(2,3)(4,5)  == curry5(1,2)(3,4)(5)
					&& curry5(1,2)(3)(4)(5) == curry5(1)(2)(3,4)(5)
					&& curry5(1,2,3,4,5)    == curry5(1)(2)(3)(4)(5);
			})
		),
		std::make_tuple(
			std::string("compose[...,R(*)(Ps...)]"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x){ return 2*x; };
				auto g = [](int x){ return float(x)/3.f; };
				auto h = ftl::compose(g, f, curry_me);

				return h(2,2) == 8.f/3.f;
			})
		),
		std::make_tuple(
			std::string("compose[...,function<R,Ps...>]"),
			std::function<bool()>([]() -> bool {
				auto f = [](int x){ return 2*x; };
				auto g = [](int x){ return float(x)/3.f; };
				auto h = ftl::compose(g, f, ftl::curry(curry_me));

				return h(2,2) == 8.f/3.f;
			})
		),
		std::make_tuple(
			std::string("flip[function<R,A,B>]"),
			std::function<bool()>([]() -> bool {
				ftl::function<int(int,int)> f = [](int x, int y){ return x/y; };
				auto g = ftl::flip(f);

				return g(2,4) == 2;
			})
		)
	}
};

