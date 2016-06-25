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
#include <string>
#include <list>
#include <ftl/ord.h>
#include "ord_tests.h"

test_set ord_tests{
	std::string("ord"),
	{
		std::make_tuple(
			std::string("compare[lt]"),
			[] {
				auto o = ftl::compare(1,2);

				TEST_ASSERT(o == ftl::ord::Lt);
			}
		),
		std::make_tuple(
			std::string("compare[eq]"),
			[] {
				auto o = ftl::compare(1,1);

				TEST_ASSERT(o == ftl::ord::Eq);
			}
		),
		std::make_tuple(
			std::string("compare[gt]"),
			[] {
				auto o = ftl::compare(2,1);

				TEST_ASSERT(o == ftl::ord::Gt);
			}
		),
		std::make_tuple(
			std::string("getComparator"),
			[] {
				auto cmp = ftl::getComparator<int>();

				TEST_ASSERT( (cmp(2,1) == ftl::ord::Gt) );
			}
		),
		std::make_tuple(
			std::string("comparing[method]"),
			[] {
				auto cmp = ftl::comparing(&std::list<int>::size);

				TEST_ASSERT( (cmp(std::list<int>{1,2,3},std::list<int>{3,2}) == ftl::ord::Gt) );
			}
		),
		std::make_tuple(
			std::string("comparing[fn]"),
			[] {
				using std::string;
				ftl::function<int(const string&)> f{
					[](const string& s){ return std::stoi(s); }
				};

				auto cmp = ftl::comparing(f);

				TEST_ASSERT( (cmp(std::string("10"),std::string("5")) == ftl::ord::Gt) );
			}
		),
		std::make_tuple(
			std::string("monoid::append"),
			[] {
				using ftl::ord;
				using ftl::operator^;

				ord lt{ord::Lt};
				ord eq{ord::Eq};
				ord gt{ord::Gt};

				TEST_ASSERT((lt ^ gt) == ord::Lt);
				TEST_ASSERT((lt ^ eq) == ord::Lt);
				TEST_ASSERT((eq ^ lt) == ord::Lt);
				TEST_ASSERT((eq ^ gt) == ord::Gt);
				TEST_ASSERT((eq ^ eq) == ord::Eq);
				TEST_ASSERT((gt ^ lt) == ord::Gt);
				TEST_ASSERT((gt ^ eq) == ord::Gt);
			}
		)
	}
};

