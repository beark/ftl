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
#include <iostream>
#include "sum_type_tests.h"
#include "either_tests.h"
#include "maybe_tests.h"
#include "future_tests.h"
#include "lazy_tests.h"
#include "ord_tests.h"
#include "functional_tests.h"
#include "prelude_tests.h"
#include "maybet_tests.h"
#include "eithert_tests.h"
#include "lazyt_tests.h"
#include "list_tests.h"
#include "vector_tests.h"
#include "fwdlist_tests.h"
#include "tuple_tests.h"
#include "memory_tests.h"
#include "string_tests.h"
#include "set_tests.h"
#include "map_tests.h"
#include "unordered_map_tests.h"
#include "concept_tests.h"

bool run_test_set(test_set& ts, std::ostream& os) {
	os << "Running test set '" << std::get<0>(ts) << "'...";

	int nsuc = 0, nfail = 0;

	for(const auto& t : std::get<1>(ts)) {
		try {
			if(!std::get<1>(t)()) {
				if(nfail == 0)
					os << std::endl;

				os << std::get<0>(t) << ": fail" << std::endl;
				++nfail;
			}
			else
				++nsuc;
		}
		catch(...) {
			os << "Unexpected exception raised while running '"
				<< std::get<0>(t) << "'" << std::endl;

			throw;
		}
	}

	os << nsuc << "/" << std::get<1>(ts).size() << " passed" << std::endl;

	return nfail == 0;
}

int main(int, char**) {

	bool flawless = true;

	flawless &= run_test_set(prelude_tests, std::cout);
	flawless &= run_test_set(sum_type_tests, std::cout);
	flawless &= run_test_set(either_tests, std::cout);
	flawless &= run_test_set(eithert_tests, std::cout);
	flawless &= run_test_set(maybe_tests, std::cout);
	flawless &= run_test_set(maybet_tests, std::cout);
	flawless &= run_test_set(future_tests, std::cout);
	flawless &= run_test_set(lazy_tests, std::cout);
	flawless &= run_test_set(lazyt_tests, std::cout);
	flawless &= run_test_set(ord_tests, std::cout);
	flawless &= run_test_set(functional_tests, std::cout);
	flawless &= run_test_set(list_tests, std::cout);
	flawless &= run_test_set(vector_tests, std::cout);
	flawless &= run_test_set(fwdlist_tests, std::cout);
	flawless &= run_test_set(tuple_tests, std::cout);
	flawless &= run_test_set(memory_tests, std::cout);
	flawless &= run_test_set(string_tests, std::cout);
	flawless &= run_test_set(set_tests, std::cout);
	flawless &= run_test_set(map_tests, std::cout);
	flawless &= run_test_set(unordered_map_tests, std::cout);
	flawless &= run_test_set(concept_tests, std::cout);

	if(!flawless)
		return -1;

	return 0;
}
