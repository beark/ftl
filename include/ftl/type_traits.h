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
#ifndef FTL_TYPE_TRAITS_H
#define FTL_TYPE_TRAITS_H

namespace ftl {
	/**
	 * \defgroup typetraits Type Traits
	 *
	 * Collection of useful type traits.
	 *
	 * The main difference between this module and \ref typelevel is that
	 * this module is concerned with finding out particular properties of types,
	 * \ref typelevel is concerned with _modifying_ types.
	 *
	 * \code
	 *   #include <ftl/type_traits.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * N/A
	 */

	namespace _dtl {
		struct no {};

		template<typename T>
		static bool test_eq(decltype(std::declval<T>() == std::declval<T>())*);

		template<typename T>
		static no test_eq(...);

		template<typename T>
		static bool test_neq(decltype(std::declval<T>() != std::declval<T>())*);

		template<typename T>
		static no test_neq(...);

		template<typename T>
		static bool test_lt(decltype(std::declval<T>() < std::declval<T>())*);

		template<typename T>
		static no test_lt(...);

		template<typename T>
		static bool test_gt(decltype(std::declval<T>() > std::declval<T>())*);

		template<typename T>
		static no test_gt(...);
	}

	/**
	 * Type trait to test for `operator==`
	 *
	 * \ingroup typetraits
	 */
	template<typename T>
	struct has_eq {
		/**
		 * Compile time value to compare to.
		 *
		 * Is `true` if there exists an `operator==(T,T)`, regardless of
		 * return type. Otherwise `false`.
		 */
		static constexpr bool value = 
			!std::is_same<
				_dtl::no,
				decltype(_dtl::test_eq<T>(nullptr))
			>::value;
	};

	/**
	 * Test a type for `operator!=`
	 *
	 * \ingroup typetraits
	 */
	template<typename T>
	struct has_neq {
		/**
		 * Compile time constant representing the result of the test.
		 *
		 * Set to `true` for `T`s for which there exists an `operator!=(T,T)`.
		 */
		static constexpr bool value = 
			!std::is_same<
				_dtl::no,
				decltype(_dtl::test_neq<T>(nullptr))
			>::value;
	};


	/**
	 * Test a type for `operator<`
	 *
	 * \ingroup typetraits
	 */
	template<typename T>
	struct has_lt {
		static constexpr bool value =
			!std::is_same<
				_dtl::no,
				decltype(_dtl::test_lt<T>(nullptr))
			>::value;
	};

	/**
	 * Test a type for `operator>`
	 *
	 * \ingroup typetraits
	 */
	template<typename T>
	struct has_gt {
		static constexpr bool value =
			!std::is_same<
				_dtl::no,
				decltype(_dtl::test_gt<T>(nullptr))
			>::value;
	};
}

#endif

