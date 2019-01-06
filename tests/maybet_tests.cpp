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
#include <ftl/maybe_trans.h>
#include <ftl/vector.h>
#include <ftl/functional.h>
#include "maybet_tests.h"

test_set maybet_tests{
	std::string("maybe_trans"),
	{
		std::make_tuple(
			std::string("functor::map[value]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using ftl::function;
				using ftl::just;
				using mf = ftl::maybeT<function<int(int)>>;

				auto f = ftl::applicative<mf>::pure(1);
				auto g = [](int x){ return float(x)/4.f; } % f;

				return (*g)(3) == just(0.25f);
			})
		),
		std::make_tuple(
			std::string("functor::map[nothing]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;

				mf f{ftl::inplace_tag(), [](int){ return ftl::nothing<int>(); }};
				auto g = [](int x){ return float(x)/4.f; } % f;

				return (*g)(3) == ftl::nothing<float>();
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;

				auto f = ftl::applicative<mf>::pure(10);

				return (*f)(50) == ftl::just(10);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[value,value]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator%;
				using ftl::operator*;

				ftl::function<int(int,int)> f = [](int x, int y){ return x+y; };
				mf x{ftl::inplace_tag(), [](int x){ return ftl::just(2*x); }};
				mf y{ftl::inplace_tag(), [](int x){ return ftl::just(x/2); }};

				auto z = f % x * y;

				return (*z)(6) == ftl::just(15);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[nothing,value]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator%;
				using ftl::operator*;

				ftl::function<int(int,int)> f = [](int x, int y){ return x+y; };
				mf x{ftl::inplace_tag(), [](int){ return ftl::nothing<int>(); }};
				mf y{ftl::inplace_tag(), [](int x){ return ftl::just(x/2); }};

				auto z = f % x * y;

				return (*z)(6) == ftl::nothing<int>();
			})
		),
		std::make_tuple(
			std::string("applicative::apply[value,nothing]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator%;
				using ftl::operator*;

				ftl::function<int(int,int)> f = [](int x, int y){ return x+y; };
				mf x{ftl::inplace_tag(), [](int x){ return ftl::just(2*x); }};
				mf y{ftl::inplace_tag(), [](int){ return ftl::nothing<int>(); }};

				auto z = f % x * y;

				return (*z)(6) == ftl::nothing<int>();
			})
		),
		std::make_tuple(
			std::string("applicative::apply[nothing,nothing]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator%;
				using ftl::operator*;

				ftl::function<int(int,int)> f = [](int x, int y){ return x+y; };
				mf x{ftl::inplace_tag(), [](int){ return ftl::nothing<int>(); }};
				mf y{ftl::inplace_tag(), [](int){ return ftl::nothing<int>(); }};

				auto z = f % x * y;

				return (*z)(6) == ftl::nothing<int>();
			})
		),
		std::make_tuple(
			std::string("monad::bind[value,->value]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator>>=;

				mf f{ftl::inplace_tag(), [](int x){ return ftl::just(x); }};
				auto g = f >>= [](int x){
					return ftl::maybeT<function<float(int)>>{
						ftl::inplace_tag(),
						[x](int y){ return ftl::just(float(x+y)/4.f); }
					};
				};
				return (*g)(2) == ftl::just(1.f);
			})
		),
		std::make_tuple(
			std::string("monad::bind[nothing,->value]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator>>=;

				mf f{ftl::inplace_tag(), [](int){ return ftl::nothing<int>(); }};
				auto g = f >>= [](int x){
					return ftl::maybeT<function<float(int)>>{
						ftl::inplace_tag(),
						[x](int y){ return ftl::just(float(x+y)/4.f); }
					};
				};
				return (*g)(2) == ftl::nothing<float>();
			})
		),
		std::make_tuple(
			std::string("monad::bind[value,->nothing]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator>>=;

				mf f{ftl::inplace_tag(), [](int x){ return ftl::just(x); }};
				auto g = f >>= [](int){
					return ftl::maybeT<function<float(int)>>{
						ftl::inplace_tag(),
						[](int){ return ftl::nothing<float>(); }
					};
				};
				return (*g)(2) == ftl::nothing<float>();
			})
		),
		std::make_tuple(
			std::string("monad::bind[nothing,->nothing]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator>>=;

				mf f{ftl::inplace_tag(), [](int){ return ftl::nothing<int>(); }};
				auto g = f >>= [](int){
					return ftl::maybeT<function<float(int)>>{
						ftl::inplace_tag(),
						[](int){ return ftl::nothing<float>(); }
					};
				};
				return (*g)(2) == ftl::nothing<float>();
			})
		),
		std::make_tuple(
			std::string("monad::bind[lift]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator>>=;

				mf f{ftl::inplace_tag(), [](int x){ return ftl::just(x); }};
				auto g = f >>= [](int x){
					return function<float(int)>{
						[x](int y){ return float(x+y)/4.f; }
					};
				};
				return (*g)(2) == ftl::just(1.f);
			})
		),
		std::make_tuple(
			std::string("monad::bind[lift&&]"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using mf = ftl::maybeT<function<int(int)>>;
				using ftl::operator>>=;

				mf f{ftl::inplace_tag(), [](int x){ return ftl::just(x); }};
				auto g = std::move(f) >>= [](int x){
					return function<float(int)>{
						[x](int y){ return float(x+y)/4.f; }
					};
				};
				return (*g)(2) == ftl::just(1.f);
			})
		),
#if !defined(_MSC_VER)  // error with VS2017 compiler because of the | operator (orDo)
		std::make_tuple(
			std::string("monoidA::orDo[value,value]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = monad<maybeT<std::vector<int>>>::pure(2);
				auto m2 = monad<maybeT<std::vector<int>>>::pure(4);

				auto m3 = m1 | m2;

				return *m1 == *m3;
			})
		),
		std::make_tuple(
			std::string("monoidA::orDo[value,nothing]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto m1 = monad<maybeT<std::vector<int>>>::pure(2);
				maybeT<std::vector<int>> m2;

				auto m3 = m1 | m2;

				return *m1 == *m3;
			})
		),
		std::make_tuple(
			std::string("monoidA::orDo[nothing,value]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybeT<std::vector<int>> m1;
				auto m2 = monad<maybeT<std::vector<int>>>::pure(2);

				auto m3 = m1 | m2;

				return *m2 == *m3;
			})
		),
		std::make_tuple(
			std::string("monoidA::orDo[nothing,nothing]"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybeT<std::vector<int>> m1;
				maybeT<std::vector<int>> m2;
				auto m3 = m1 | m2;

				return *m3 == *(maybeT<std::vector<int>>{});
			})
		),
#endif
		std::make_tuple(
			std::string("foldable::foldl"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybeT<std::vector<int>> m1{
					inplace_tag(),
					just(1),
					just(2),
					nothing<int>(),
					just(4)
				};

				auto r = foldl([](int x, int y){ return x+y; }, 0, m1);

				return r == 7;
			})
		),
		std::make_tuple(
			std::string("foldable::foldr"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybeT<std::vector<float>> m1{
					inplace_tag(),
					just(4.f),
					just(4.f),
					nothing<float>(),
					just(2.f)
				};

				auto r = foldr([](float x, float y){ return x/y; }, 16.f, m1);

				return r == .125f;
			})
		),
		std::make_tuple(
			std::string("foldable::fold"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybeT<std::vector<sum_monoid<int>>> m1{
					inplace_tag(),
					just(sum(1)),
					just(sum(2)),
					nothing<sum_monoid<int>>(),
					just(sum(4))
				};

				return fold(m1) == 7;
			})
		)
		/* Crashed gcc, works as it should in clang
		,
		std::make_tuple(
			std::string("foldable::foldMap"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				maybeT<std::vector<int>> m1{
					inplace_tag(),
					value(1),
					value(2),
					maybe<int>{},
					value(4)
				};

				return foldMap(sum<int>, m1) == 7;
			})
		)
		*/
	}
};

