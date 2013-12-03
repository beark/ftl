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
#include "../prelude.h"

namespace ftl {

	/**
	 * \page monoidpg Monoid
	 *
	 * Concept encapsulating the mathematical construct of the same name.
	 *
	 * Mathematically, a monoid is any set `S`, for which there is an associated
	 * binary operation, •, and where there exists an element `id` of `S` such
	 * that the following laws hold:
	 *
	 * - **Right identity law**
	 *   \code
	 *       a • id = a
	 *   \endcode
	 * - **Left identity law**
	 *   \code
	 *       id • a = a
	 *   \endcode
	 * - **Law of associativity**
	 *   \code
	 *       a • (b • c) = (a • b) • c
	 *   \endcode
	 *
	 * Note, however, that in FTL, the binary monoid operation is denoted by
	 * either of `monoid<aType>::append`, `ftl::mappend`, or `ftl::operator^`.
	 * This is of course due to the limited selection of overloadable operators
	 * in C++.
	 *
	 * For the actual interface instances need to implement, refer to the
	 * documentation of `ftl::monoid`.
	 *
	 * \see \ref monoid (module)
	 */

	/**
	 * \defgroup monoid Monoid
	 *
	 * Functions and interfaces relating to the \ref monoidpg concept.
	 *
	 * \code
	 *   #include <ftl/concepts/monoid.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - <type_traits>
	 * - \ref prelude
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
		 * performs. For example, when using the `sum_monoid` to give numbers a
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
		 * `const &`, but never a non-const reference or other type
		 * that would allow mutation of either `m1` or `m2`.
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
	 * Concepts lite-compatible check for monoid instances.
	 *
	 * Can also be used with SFINAE to "hide" functions and methods from
	 * non-monoid types.
	 *
	 * Example usage:
	 * \code
	 *   // Due to SFINAE, myFunc will not be considered an option for
	 *   // non-monoids.
	 *   template<
	 *       typename M,
	 *       typename = Requires<Monoid<M>()>
	 *   >
	 *   void myFunc(M m1, M m2);
	 * \endcode
	 *
	 * \ingroup monoid
	 */
	template<typename M>
	constexpr bool Monoid() noexcept {
		return monoid<M>::instance;
	}

	/**
	 * Convenience operator to ease use of append.
	 *
	 * The use of this operator is of course optional, but it can make code
	 * considerably cleaner. Another option is to use `ftl::mappend`.
	 *
	 * Example usage:
	 * \code
	 *   void myFunc(const MyMonoid& m1, const MyMonoid& m2) {
	 *       using ftl::operator^;
	 *       std::cout << m1 ^ m2 << std::endl;
	 *   }
	 *
	 *   // Equivalent, operator-less version
	 *   void myFunc2(const MyMonoid&m1, const MyMonoid& m2) {
	 *       std::cout << ftl::monoid<MyMonoid>::append(m1, m2) << std::endl;
	 *   }
	 * \endcode
	 *
	 * \note Due to technical constraints, `M1` and `M2` must appear as
	 *       different types (to achieve perfect forwarding), but in reality,
	 *       they must be the exact same type, barr constness and reference
	 *       type.
	 *
	 * \see mappend
	 *
	 * \ingroup monoid
	 */
	template<
			typename M1,
			typename M2,
			typename M = plain_type<M1>,
			typename = Requires<
				Monoid<M>()
				&& std::is_same<M,plain_type<M2>>::value
			>
	>
	M operator^ (M1&& m1, M2&& m2) {
		return monoid<M>::append(std::forward<M1>(m1), std::forward<M2>(m2));
	}

	/**
	 * Convenience function object for `monoid::append`.
	 *
	 * Allows easy creation of a function object that can be passed as argument
	 * to higher-order functions as a substitute for `monoid::append`.
	 *
	 * Instances of this function object support curried calling.
	 *
	 * \ingroup monoid
	 */
	struct mAppend
#ifndef DOCUMENTATION_GENERATOR
	: private _dtl::curried_binf<mAppend>
#endif
	{
		template<
				typename M1,
				typename M2,
				typename M = plain_type<M1>,
				typename = Requires<
					Monoid<M>()
					&& std::is_same<M,plain_type<M2>>::value
				>
		>
		M operator() (M1&& m1, M2&& m2) const {
			return
				monoid<M>::append(std::forward<M1>(m1), std::forward<M2>(m2));
		}

		using curried_binf<mAppend>::operator();
	};

	/**
	 * Compile time instance of `mAppend`.
	 *
	 * Example usage:
	 * \code
	 *   int foo() {
	 *       using ftl::operator%;
	 *       using ftl::operator*;
	 *
	 *       auto m1 = ftl::value(ftl::sum(2));
	 *       auto m2 = ftl::value(ftl::sum(2));
	 *
	 *       // Makes use of applicative style function invocation
	 *       auto m3 = ftl::mappend % m1 * m2;
	 *   }
	 * \endcode
	 * Outputs "4".
	 *
	 * \see mAppend
	 *
	 * \ingroup monoid
	 */
	constexpr mAppend mappend{};

	/**
	 * Implementation of monoid for numbers, interpreted as sums.
	 *
	 * The reason for wrapping numbers in this struct when using the monoid
	 * concept is twofold:
	 * - To keep the combining monoid `operator^` from interfering with
	 *   any such operator defined for the plain type
	 * - To allow for secondary or tertiary interpretations, such as products.
	 *
	 * The behaviour of the sum monoid is simple:
	 * \code
	 *   id() => N(0)
	 *   operator^ => N::operator+
	 * \endcode
	 *
	 * \tparam N Any integer or floating point primitive type, _or_ any 
	 *           type that implements `operator+` (in such a way that it does
	 *           not violate the monoid laws) and can be constructed from the
	 *           literal `0`.
	 *
	 * \par Concepts
	 * - \ref fullycons
	 * - \ref eq, if `N` is
	 * - \ref orderablepg, if `N` is
	 *
	 * \par Examples
	 *
	 * Trivial usage:
	 * \code
	 *   ftl::sum_monoid<int> x = 1, y = 2;
	 *   std::cout << ftl::mappend(x, y);
	 * \endcode
	 * Outputs "3"
	 *
	 * Summing the contents of a list:
	 * \code
	 *   std::list<int> l{1,2,3,4};
	 *
	 *   // map sum to each element, creating a new list of sum_monoids
	 *   auto l2 = ftl::sum % l;
	 *
	 *   // fold iterates the collection and accumulates a result using mappend
	 *   std::cout << ftl::fold(l2);
	 * \endcode
	 * Outputs "10"
	 *
	 * \ingroup monoid
	 */
	template<typename N>
	struct sum_monoid {

		/**
		 * Construct from `N`.
		 *
		 * Allows implicit casts from `N` to `sum_monoid`. While this is
		 * sometimes frowned upon, in the expected use cases of `sum_monoid` it
		 * makes sense.
		 */
		constexpr sum_monoid(N num)
		noexcept(std::is_nothrow_copy_constructible<N>::value)
			: n(num) {}

		/**
		 * Implicit cast to `N`.
		 *
		 * This allows more convenient usage, as in the majority of cases,
		 * the `sum_monoid` should be as transparent as possible.
		 */
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
	 * Actual implementation of \ref monoidpg for sums.
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
	 * The reason behind this struct is exactly the same as with
	 * `ftl::sum_monoid`.
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
	 * \par Concepts
	 * - \ref fullycons
	 * - \ref eq, if `N` is
	 * - \ref orderablepg, if `N` is
	 *
	 * \par Examples
	 *
	 * Trivial usage:
	 * \code
	 *   float foo() {
	 *       using ftl::operator^;
	 *       ftl::prod_monoid<float> x = 3.f, y = 2.5f;
	 *       return x ^ y ^ ftl::monoid<ftl::prod_monoid<float>>::id();
	 *   }
	 * \endcode
	 * `foo()` will always return the result of `3.f*2.5f*1.f`.
	 *
	 * Use the fact that "monoidness" is transitive in the case of `ftl::maybe`
	 * and its contained type:
	 * \code
	 *   ftl::maybe<ftl::prod_monoid<int>> m1{2};
	 *   ftl::maybe<ftl::prod_monoid<int>> m2{3};
	 *
	 *   ftl::maybe<ftl::prod_monoid<int>> m3 = ftl::mappend(m1, m2);
	 *   if(m3) std::cout << *m3;
	 * \endcode
	 * Outputs "6"
	 *
	 * \ingroup monoid
	 */
	template<typename N>
	struct prod_monoid {
		/**
		 * Construct from `N`.
		 *
		 * This allows implicit conversion from `N` to `prod_monoid<N>`. The
		 * purpose is of course to make the `prod_monoid` as transparent and
		 * convenient to use as possible.
		 */
		constexpr prod_monoid(N num)
		noexcept(std::is_nothrow_copy_constructible<N>::value)
			: n(num) {}

		constexpr prod_monoid operator* (const prod_monoid& m) const
		noexcept(std::is_nothrow_move_constructible<prod_monoid>::value) {
			return prod_monoid(n*m.n);
		}

		/**
		 * Implicit conversion back to `N`.
		 */
		constexpr operator N () const noexcept {
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
	 * \par Examples
	 *
	 * Trivial usage:
	 * \code
	 *   bool foo() {
	 *       using ftl::operator^;
	 *
	 *       ftl::any a = false, b = true, c = ftl::monoid<ftl::any>::id();
	 *
	 *       return a ^ b ^ c;
	 *   }
	 * \endcode
	 * `foo` will always return `true`.
	 *
	 * Check for `nothing` in a list of maybes.
	 * \code
	 *   bool nothingIsElemOf(std::list<ftl::maybe<SomeType>> l) {
	 *       auto f = [](const ftl::maybe<SomeType>& m) -> ftl::any {
	 *           return m == ftl::nothing;
	 *       };
	 *
	 *       return ftl::foldMap(f, l);
	 *   }
	 * \endcode
	 *
	 * \ingroup monoid
	 */
	struct any {
		/**
		 * Construct from bool.
		 *
		 * Allows implicit conversion from bool to `any`.
		 */
		constexpr any(bool bl) noexcept : b(bl) {}

		/**
		 * Implicit cast back to bool
		 *
		 * This allows a more transparent and convenient way of using the
		 * `any` monoid.
		 */
		constexpr operator bool() const noexcept {
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
	 * \par Examples
	 *
	 * Trivial usage:
	 * \code
	 *   bool foo() {
	 *       using ftl::operator^;
	 *
	 *       ftl::all a = false, b = true, c = ftl::monoid<ftl::any>::id();
	 *
	 *       return a ^ b ^ c;
	 *   }
	 * \endcode
	 * `foo` will always return `false`.
	 *
	 * Check whether all the `maybe`s in a list are values:
	 * \code
	 *   bool allValues(std::list<ftl::maybe<SomeType>> l) {
	 *       auto f = [](const ftl::maybe<SomeType>& m) -> ftl::all {
	 *           return m != ftl::nothing;
	 *       };
	 *
	 *       return ftl::foldMap(f, l);
	 *   }
	 * \endcode
	 *
	 * \ingroup monoid
	 */
	struct all {
		/// Construct from a bool.
		constexpr all(bool bl) noexcept : b(bl) {}

		/// Implicit cast back to bool.
		constexpr operator bool() const noexcept {
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

