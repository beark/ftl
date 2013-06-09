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

#ifndef FTL_TESTS_BASE_H
#define FTL_TESTS_BASE_H

#include <ostream>
#include <vector>
#include <tuple>
#include <string>
#include <functional>

using test_t = std::tuple<std::string,std::function<bool()>>;
using test_set = std::tuple<std::string,std::vector<test_t>>;

bool run_test_set(test_set& ts, std::ostream& os) {
	os << "Running test set '" << std::get<0>(ts) << "' ("
		<< std::get<1>(ts).size() << " individual tests)..." << std::endl;

	int nsuc = 0, nfail = 0;

	for(const auto& t : std::get<1>(ts)) {
		try {
			if(!std::get<1>(t)()) {
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

	os << "Set results: " << nfail << " failed, " << nsuc << " succeeded." << std::endl;

	return nfail > 0;
}

#endif

