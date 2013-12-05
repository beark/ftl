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
#ifndef FTL_ORDERABLE_H
#define FTL_ORDERABLE_H

#include "../type_traits.h"

namespace ftl {
	/**
	 * \defgroup orderable Orderable
	 *
	 * \ref eq and \ref orderablepg concepts
	 *
	 * \code
	 *   #include <ftl/concepts/orderable.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref type_traits
	 */

	/**
	 * \page eq Eq
	 *
	 * Any type that can be compared for both equality and inequality.
	 *
	 * More formally, there must exist an `operator==` and an `operator!=` that
	 * both accept two objects of the type in question and returns either a
	 * `bool` or something contextually convertible to the same.
	 *
	 * Further, both comparison operators must be
	 * - Commutative: `a == b` implies `b == a`
	 * - Transitive: `a == b` and `b == c` implies `a == c`
	 *
	 * \see \ref orderable (module)
	 */

	/**
	 * \page orderablepg Orderable
	 *
	 * Anything that can be ordered in some strict sense.
	 *
	 * In essence, any type that defines the operators `<` and `>`, as
	 * well as implements the concept \ref eq.
	 *
	 * Note that the 'less than' and 'greater than' operators must evaluate
	 * to something contextually convertible to `bool` as well as satisfy the
	 * following:
	 * - `a < b` implies `b > a`
	 * - `a > b` implies `b < a`
	 * - `a > b` and `a < b` both implies `a != b`
	 *
	 * \see \ref orderable (module)
	 */

	/**
	 * Predicate to check for \ref eq instances.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   void foo() {
	 *       static_assert(Eq<T>(), "foo: T is not an instance of Eq");
	 *
	 *       // Do interesting comparisons with Ts
	 *   }
	 * \endcode
	 *
	 * \ingroup orderable
	 */
	template<typename E>
	constexpr bool Eq() noexcept {
		return has_eq<E>::value && has_neq<E>::value;
	}

	/**
	 * Predicate to check for \ref orderablepg instances.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   void foo() {
	 *       static_assert(Ord<T>(), "foo: T is not an instance of Ord");
	 *
	 *       // Order various Ts
	 *   }
	 * \endcode
	 *
	 * \ingroup orderable
	 */
	template<typename Ord>
	constexpr bool Orderable() noexcept {
		return Eq<Ord>() && has_lt<Ord>::value && has_gt<Ord>::value;
	}

}

#endif


