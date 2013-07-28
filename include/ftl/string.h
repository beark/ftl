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
#ifndef FTL_STRING_H
#define FTL_STRING_H

#include <string>
#include "concepts/monoid.h"

namespace ftl {

	/**
	 * \defgroup string String
	 *
	 * Concept implementations for the standard string classes.
	 *
	 * \par Dependencies
	 * - <string>
	 * - \ref monoid
	 */

	/**
	 * Monoid instance for the standard string classes.
	 *
	 * Behaviour:
	 * \code
	 *   id()         <=> ""
	 *   append(a, b) <=> a + b
	 * \endcode
	 *
	 * Satisfies the monoid laws because:
	 * \code
	 *   a ^ id()    <=> a + ""      <=> a
	 *   id() ^ a    <=> "" + a      <=> a
	 *   a ^ (b ^ c) <=> a + (b + c) <=> (a + b) + c <=> (a ^ b) ^ c
	 * \endcode
	 *
	 * \ingroup string
	 */
	template<typename...Ts>
	struct monoid<std::basic_string<Ts...>> {

		static std::basic_string<Ts...> id() {
			return std::basic_string<Ts...>{};
		}

		static std::basic_string<Ts...> append(
				const std::basic_string<Ts...>& s1,
				const std::basic_string<Ts...>& s2) {

			return s1 + s2;
		}

		static std::basic_string<Ts...> append(
				std::basic_string<Ts...>&& s1,
				const std::basic_string<Ts...>& s2) {

			s1 += s2;
			return s1;
		}

		static std::basic_string<Ts...> append(
				const std::basic_string<Ts...>& s1,
				std::basic_string<Ts...>&& s2) {

			s2.insert(s2.begin(), s1.begin(), s1.end());
			return s2;
		}

		static std::basic_string<Ts...> append(
				std::basic_string<Ts...>&& s1,
				std::basic_string<Ts...>&& s2) {

			std::move(s2.begin(), s2.end(), std::back_inserter(s1));
			return s1;
		}

		static constexpr bool instance = true;

	};

}

#endif

