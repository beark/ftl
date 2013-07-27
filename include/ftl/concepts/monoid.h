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
#ifndef FTL_MONOID_H
#define FTL_MONOID_H

#include <type_traits>

namespace ftl {

	/**
	 * \page monoidpg Monoid
	 *
	 * Concept encapsulating the mathematical construct of the same name.
	 *
	 * Mathematically, a monoid is any set \c S, for which there is an
	 * associated binary operation, •, and where there exists
	 * an element \c id of \c S such that the following laws hold:
	 *
	 * \li **Right identity law**
	 *     \code
	 *       a • id = a
	 *     \endcode
	 * \li **Left identity law**
	 *     \code
	 *       id • a = a
	 *     \endcode
	 * \li **Law of associativity**
	 *     \code
	 *       a • (b • c) = (a • b) • c
	 *     \endcode
	 *
	 * Note, however, that in FTL, the binary monoid operation is denoted either
	 * by `monoid<instance>::append` or by `ftl::operator^`. This is due to the
	 * limited selection of overloadable operators in C++.
	 *
	 * For the actual interface instances need to implement, refer to the
	 * documentation of  `ftl::monoid`.
	 *
	 * \see \ref monoid (module)
	 */

	/**
	 * \defgroup monoid Monoid
	 *
	 * Functions and interfaces relating to the \ref monoidpg concept.
	 *
	 * \code
	 *   #include <ftl/monoid.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - <type_traits>
	 */

	/**
	 * \interface monoid
	 *
	 * \brief Concrete monoid interface.
	 *
	 * For any type to be an instance of the monoid concept, it must specialise
	 * this interface.
	 *
	 * \ingroup monoid
	 */
	template<typename M>
	struct monoid {
		/**
		 * Get the identity element for a given monoid.
		 *
		 * Which element to be used depends on what operation the monoid
		 * performs. For example, when using the sum_monoid to give numbers a
		 * monoid implementation, 0 is used. This is because
		 * \code
		 *   a + 0 = a
		 *   0 + a = a
		 * \endcode
		 * and + is the definition of append / `operator^` for sums.
		 */
		static M id();

		/**
		 * The monoid operation itself.
		 *
		 * While implementations should follow the general style of this
		 * interface, they are also allowed to accept for instance
		 * \c const \c M&, but never a non-const reference or other type
		 * that would allow mutation of either \c m1 or \c m2.
		 */
		static M append(M m1, M m2);

		/**
		 * Used for compile time checks.
		 *
		 * Any actual instance must define this as true.
		 */
		static constexpr bool instance = false;
	};

	/**
	 * Convenience operator to ease use of append.
	 *
	 * This default implementation should work for any type that properly
	 * implements the monoid interface.
	 *
	 * \ingroup monoid
	 */
	template<
		typename M,
		typename = typename std::enable_if<monoid<M>::instance>::type>
	M operator^ (const M& m1, const M& m2) {
		return monoid<M>::append(m1, m2);
	}

	/**
	 * \overload
	 *
	 * \ingroup monoid
	 */
	template<
		typename M,
		typename = typename std::enable_if<monoid<M>::instance>::type>
	M operator^ (M&& m1, M&& m2) {
		return monoid<M>::append(std::move(m1), std::move(m2));
	}

	/**
	 * Implementation of monoid for numbers, interpreted as sums.
	 *
	 * The reason for wrapping numbers in this struct when using the monoid
	 * concept is twofold:
	 * \li To keep the combining monoid \c operator^ from interfering with
	 *     any such operator defined for the plain type
	 * \li To allow for secondary or tertiary interpretations, such as products.
	 *
	 * The behaviour of the sum monoid is simple:
	 * \code
	 *   id() => N(0)
	 *   operator^ => N::operator+
	 * \endcode
	 *
	 * \tparam N Any integer or floating point primitive type, \em or any 
	 *           type that implements `operator+` (in such a way that it does
	 *           not violate the monoid laws) and can be constructed from the
	 *           literal `0`.
	 *
	 * \ingroup monoid
	 */
	template<typename N>
	struct sum_monoid {

		constexpr sum_monoid(N num)
		noexcept(std::is_nothrow_copy_constructible<N>::value)
			: n(num) {}

		constexpr operator N () const noexcept {
			return n;
		}

		constexpr sum_monoid operator+ (const sum_monoid& n2) const
		noexcept(std::is_nothrow_move_assignable<N>::value) {
			return sum_monoid(n + n2.n);
		}

		N n;
	};

	/**
	 * Convenience function to concisely create new sums.
	 *
	 * \ingroup monoid
	 */
	template<typename N>
	constexpr sum_monoid<N> sum(N num)
	noexcept(std::is_nothrow_constructible<sum_monoid<N>,N>::value) {
		return sum_monoid<N>(num);
	}

	/*
	 * Actual implementation of monoid for sums.
	 *
	 * The identity is 0 and the combining operation is +.
	 *
	 * \ingroup monoid
	 */
	template<typename N>
	struct monoid<sum_monoid<N>> {
		static constexpr sum_monoid<N> id()
		noexcept(std::is_nothrow_constructible<sum_monoid<N>,N>::value) {
			return sum(N(0));
		}

		static constexpr sum_monoid<N> append(
				const sum_monoid<N>& n1,
				const sum_monoid<N>& n2) {

			return n1 + n2;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Implementation of monoid for numbers, when interpreted as products.
	 *
	 * The reason behind this struct is exactly the same as with sum_monoid.
	 *
	 * The behaviour of the product monoid is simple:
	 * \code
	 *   id() => N(1)
	 *   operator^ => N::operator*
	 * \endcode
	 *
	 * \tparam N Any integer or floating point primitive type, \em or a type
	 *           that implements `operator*` (in a way that does not violate the
	 *           monoid laws) and can be constructed from the literal `1`.
	 *
	 * \ingroup monoid
	 */
	template<typename N>
	struct prod_monoid {
		constexpr prod_monoid(N num)
		noexcept(std::is_nothrow_copy_constructible<N>::value)
			: n(num) {}

		constexpr prod_monoid operator* (const prod_monoid& m)
		noexcept(std::is_nothrow_move_constructible<prod_monoid>::value) {
			return prod_monoid(n*m.n);
		}

		constexpr operator N () noexcept {
			return n;
		}

		N n;
	};

	/**
	 * Convenience function to concisely create new products.
	 *
	 * \ingroup monoid
	 */
	template<typename N>
	constexpr prod_monoid<N> prod(N n)
	noexcept(std::is_nothrow_constructible<prod_monoid<N>,N>::value) {
		return prod_monoid<N>(n);
	}

	/*
	 * Actual implementation of monoid for products.
	 *
	 * \ingroup monoid
	 */
	template<typename N>
	struct monoid<prod_monoid<N>> {
		static constexpr prod_monoid<N> id()
		noexcept(std::is_nothrow_constructible<prod_monoid<N>,N>::value) {
			return prod(N(1));
		}

		static constexpr prod_monoid<N> append(
				const prod_monoid<N>& n1,
				const prod_monoid<N>& n2) {

			return n1 * n2;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Wrapper for booleans to make them a monoid.
	 *
	 * This particular version of bools as monoids means that:
	 * \code
	 *   monoid<any>::id() <=> false
	 *   monoid<any>::append() <=> ||
	 * \endcode
	 *
	 * \ingroup monoid
	 */
	struct any {
		/// Construct from bool
		constexpr any(bool bl) noexcept : b(bl) {}

		/// Cast back to bool
		constexpr operator bool() noexcept {
			return b;
		}

		bool b = false;
	};

	/*
	 * Monoid implementation for any.
	 *
	 * \ingroup monoid
	 */
	template<>
	struct monoid<any> {
		/// Return `false`
		static constexpr any id() noexcept {
			return false;
		}

		/// Return `a1 || a2`
		static constexpr any append(any a1, any a2) noexcept {
			return a1.b || a2.b;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Wrapper for booleans to get a monoid implementation.
	 *
	 * This particular version of bools as monoids means that:
	 * \code
	 *   monoid<any>::id() <=> true
	 *   monoid<any>::append() <=> &&
	 * \endcode
	 *
	 * \ingroup monoid
	 */
	struct all {
		/// Construct from a bool
		constexpr all(bool bl) noexcept : b(bl) {}

		/// Cast back to bool
		constexpr operator bool() noexcept {
			return b;
		}

		bool b = true;
	};

	/*
	 * Monoid implementation for bools as an AND-operation.
	 *
	 * \ingroup monoid
	 */
	template<>
	struct monoid<all> {
		/// Returns `true`
		static constexpr all id() noexcept {
			return true;
		}

		/// Returns `a1 && a2`
		static constexpr all append(all a1, all a2) noexcept {
			return a1 && a2;
		}

		static constexpr bool instance = true;
	};

}

#endif

