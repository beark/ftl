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
#ifndef FTL_ORD_TESTS_H
#define FTL_ORD_TESTS_H

#include <string>
#include <ftl/ord.h>
#include "base.h"

test_set ord_tests{
	std::string("ord"),
	{
		std::make_tuple(
			std::string("compare[lt]"),
			std::function<bool()>([]() -> bool {
				auto o = ftl::compare(1,2);

				return o == ftl::ord::Lt;
			})
		),
		std::make_tuple(
			std::string("compare[eq]"),
			std::function<bool()>([]() -> bool {
				auto o = ftl::compare(1,1);

				return o == ftl::ord::Eq;
			})
		),
		std::make_tuple(
			std::string("compare[gt]"),
			std::function<bool()>([]() -> bool {
				auto o = ftl::compare(2,1);

				return o == ftl::ord::Gt;
			})
		),
		std::make_tuple(
			std::string("getComparator"),
			std::function<bool()>([]() -> bool {
				auto cmp = ftl::getComparator<int>();

				return cmp(2,1) == ftl::ord::Gt;
			})
		),
		std::make_tuple(
			std::string("comparing[method]"),
			std::function<bool()>([]() -> bool {
				auto cmp = ftl::comparing(&std::string::size);

				return cmp(std::string("ab"),std::string("b")) == ftl::ord::Gt;
			})
		),
		std::make_tuple(
			std::string("comparing[fn]"),
			std::function<bool()>([]() -> bool {
				using std::string;
				ftl::function<int,const string&> f{
					[](const string& s){ return std::stoi(s); }
				};

				auto cmp = ftl::comparing(f);

				return cmp(std::string("10"),std::string("5")) == ftl::ord::Gt;
			})
		),
		std::make_tuple(
			std::string("monoid::append"),
			std::function<bool()>([]() -> bool {
				using ftl::ord;
				using ftl::operator^;

				ord lt{ord::Lt};
				ord eq{ord::Eq};
				ord gt{ord::Gt};

				return (lt ^ gt) == ord::Lt
					&& (lt ^ eq) == ord::Lt
					&& (eq ^ lt) == ord::Lt
					&& (eq ^ gt) == ord::Gt
					&& (eq ^ eq) == ord::Eq
					&& (gt ^ lt) == ord::Gt
					&& (gt ^ eq) == ord::Gt;
			})
		)
	}
};

#endif

