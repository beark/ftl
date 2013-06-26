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
#ifndef FTL_EITHERT_TESTS_H
#define FTL_EITHERT_TESTS_H

#include <string>
#include <ftl/either_trans.h>
#include "base.h"

test_set eithert_tests{
	std::string("either_trans"),
	{
		std::make_tuple(
			std::string("functor::map[R]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using ftl::function;
				using ef = ftl::eitherT<std::string,function<int,int>>;

				auto f = ftl::applicative<ef>::pure(1);
				auto g = [](int x){ return float(x)/4.f; } % f;

				return (*g)(3) == ftl::make_right<std::string>(0.25f);
			})
		),
		std::make_tuple(
			std::string("functor::map[L]"),
			std::function<bool()>([]() -> bool {
				using ftl::operator%;
				using ftl::function;
				using ef = ftl::eitherT<std::string,function<int,int>>;

				ef f{
					ftl::inplace_tag(),
					[](int){ return ftl::make_left<int>(std::string("test")); }
				};

				auto g = [](int x){ return float(x)/4.f; } % f;

				return (*g)(3) == ftl::make_left<float>(std::string("test"));
			})
		),
		std::make_tuple(
			std::string("applicative::pure"),
			std::function<bool()>([]() -> bool {
				using ftl::function;
				using ef = ftl::eitherT<float,function<int,int>>;

				auto f = ftl::applicative<ef>::pure(10);

				return (*f)(50) == ftl::make_right<float>(10);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[R,R]"),
			std::function<bool()>([]() -> bool {
				using ef = ftl::eitherT<float,ftl::function<int,int>>;
				using namespace ftl;

				ftl::function<int,int,int> f = [](int x, int y){ return x+y; };
				ef x{inplace_tag(), [](int x){ return make_right<float>(2*x); }};
				ef y{inplace_tag(), [](int x){ return make_right<float>(x/2); }};

				auto z = f % x * y;

				return (*z)(6) == make_right<float>(15);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[L,R]"),
			std::function<bool()>([]() -> bool {
				using ef = ftl::eitherT<float,ftl::function<int,int>>;
				using namespace ftl;

				ftl::function<int,int,int> f = [](int x, int y){ return x+y; };
				ef x{ inplace_tag(), [](int){ return make_left<int>(0.f); }};
				ef y{inplace_tag(), [](int x){ return make_right<float>(x/2); }};

				auto z = f % x * y;

				return (*z)(6) == make_left<int>(0.f);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[R,L]"),
			std::function<bool()>([]() -> bool {
				using ef = ftl::eitherT<float,ftl::function<int,int>>;
				using namespace ftl;

				ftl::function<int,int,int> f = [](int x, int y){ return x+y; };
				ef x{ inplace_tag(), [](int x){ return make_right<float>(2*x); }};
				ef y{inplace_tag(), [](int){ return make_left<int>(0.f); }};

				auto z = f % x * y;

				return (*z)(6) == make_left<int>(0.f);
			})
		),
		std::make_tuple(
			std::string("applicative::apply[L,L]"),
			std::function<bool()>([]() -> bool {
				using ef = ftl::eitherT<float,ftl::function<int,int>>;
				using namespace ftl;

				ftl::function<int,int,int> f = [](int x, int y){ return x+y; };
				ef x{ inplace_tag(), [](int){ return make_left<int>(0.f); }};
				ef y{inplace_tag(), [](int){ return make_left<int>(0.f); }};

				auto z = f % x * y;

				return (*z)(6) == make_left<int>(0.f);
			})
		),
		std::make_tuple(
			std::string("monad::bind[R,->R]"),
			std::function<bool()>([]() -> bool {
				using ef = ftl::eitherT<float,ftl::function<int,int>>;
				using namespace ftl;

				ef f{inplace_tag(), [](int x){ return make_right<float>(x); }};
				auto g = f >>= [](int x){
					return eitherT<float,function<float,int>>{
						inplace_tag(),
						[x](int y){ return make_right<float>(float(x+y)/4.f); }
					};
				};
				return (*g)(2) == make_right<float>(1.f);
			})
		),
		std::make_tuple(
			std::string("monad::bind[L,->R]"),
			std::function<bool()>([]() -> bool {
				using ef = ftl::eitherT<float,ftl::function<int,int>>;
				using namespace ftl;

				ef f{inplace_tag(), [](int){ return make_left<int>(0.f); }};
				auto g = f >>= [](int x){
					return eitherT<float,function<float,int>>{
						inplace_tag(),
						[x](int y){ return make_right<float>(float(x+y)/4.f); }
					};
				};
				return (*g)(2) == make_left<float>(0.f);
			})
		),
		std::make_tuple(
			std::string("monad::bind[R,->L]"),
			std::function<bool()>([]() -> bool {
				using ef = ftl::eitherT<float,ftl::function<int,int>>;
				using namespace ftl;

				ef f{inplace_tag(), [](int x){ return make_right<float>(x); }};
				auto g = f >>= [](int x){
					return eitherT<float,function<float,int>>{
						inplace_tag(),
						[x](int){ return make_left<float>(0.f); }
					};
				};
				return (*g)(2) == make_left<float>(0.f);
			})
		),
		std::make_tuple(
			std::string("monad::bind[L,->L]"),
			std::function<bool()>([]() -> bool {
				using ef = ftl::eitherT<float,ftl::function<int,int>>;
				using namespace ftl;

				ef f{inplace_tag(), [](int){ return make_left<int>(0.f); }};
				auto g = f >>= [](int x){
					return eitherT<float,function<float,int>>{
						inplace_tag(),
						[x](int){ return make_left<float>(0.f); }
					};
				};
				return (*g)(2) == make_left<float>(0.f);
			})
		),
	}
};

#endif




