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
#include <ftl/sum_type.h>
#include "sum_type_tests.h"

template<typename T>
struct Just {
	explicit constexpr Just(const T& t) noexcept : value(t) {}
	explicit constexpr Just(T&& t) noexcept : value(std::move(t)) {}

	T& operator*() {
		return value;
	}

	constexpr const T& operator*() const {
		return value;
	}

private:
	T value;
};

struct Nothing {};

template<typename T>
using maybe = ftl::sum_type<Just<T>,Nothing>;

template<typename T>
maybe<ftl::plain_type<T>> just(T&& t) {
	using ftl::plain_type;

	return maybe<plain_type<T>>{
		ftl::constructor<Just<plain_type<T>>>(), std::forward<T>(t)
	};
}

test_set sum_type_tests{
	std::string("sum_type"),
	{
		std::make_tuple(
			std::string("Construct using select"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				int x = 1;

				sum_type<int,int*> y{constructor<int>(), 12};
				sum_type<int,int*> z{constructor<int*>(), &x};

				return true;
			})
		),
		std::make_tuple(
			std::string("Get by index"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				sum_type<int,char> x{constructor<int>(), 10};
				sum_type<int,char> y{constructor<char>(), 'b'};

				auto s1 = get<0>(x);
				auto s2 = get<1>(y);

				return s1 == 10 && s2 == 'b';
			})
		),
		std::make_tuple(
			std::string("Match case_ expressions"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				sum_type<int,char> x{constructor<int>(), 10};
				sum_type<int,char> y{constructor<char>(), 'b'};

				auto s1 = x.match(
					[](case_<int> x){ return x*2; },
					[](case_<char>){ return 0; }
				);

				auto s2 = y.match(
					[](case_<int> x){ return x*2; },
					[](case_<char>){ return 0; }
				);

				return s1 == 20 && s2 == 0;
			})
		),
		std::make_tuple(
			std::string("Match expressions"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				struct A {};
				struct B {};
				struct C {};

				sum_type<A,B,C> x{constructor<A>()};
				sum_type<A,B,C> y{constructor<B>()};
				sum_type<A,B,C> z{constructor<C>()};

				auto s1 = x.match(
					[](A){ return 0; },
					[](B){ return 1; },
					[](C){ return 2; }
				);

				auto s2 = y.match(
					[](A){ return 0; },
					[](B){ return 1; },
					[](C){ return 2; }
				);

				auto s3 = z.match(
					[](A){ return 0; },
					[](B){ return 1; },
					[](C){ return 2; }
				);

				return s1 == 0 && s2 == 1 && s3 == 2;
			})
		),
		std::make_tuple(
			std::string("Maybe mockup"),
			std::function<bool()>([]() -> bool {
				using namespace ftl;

				auto x = just(12);
				auto y = maybe<int>{constructor<Nothing>()};

				auto s1 = x.match(
					[](Just<int> x){ return *x; },
					[](Nothing){ return 0; }
				);

				return s1 == 12;
			})
		),
	}
};

