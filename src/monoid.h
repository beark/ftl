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
#ifndef BTL_MONOID_H
#define BTL_MONOID_H

#include <type_traits>
#include <tuple>

namespace btl {
	/**
	 * \interface monoid
	 *
	 * Monoid abstraction.
	 *
	 * In addition to the methods listed here, an instance of Monoid must also
	 * implement \c mappend, and its equivalent \c operator^. These should have
	 * the signatures:
	 * \code
	 *   monoid_instance mappend(const monoid_instance&, const monoid_instance&);
	 *   monoid_instance operator^ (const monoid_instance&, const monoid_instance&);
	 * \endcode
	 *
	 * \laws
	 * \li a ^ id = a
	 * \li id ^ a = a
	 * \li a ^ (b ^ c) = (a ^ b) ^ c
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
		 * and + is the definition of mappend / operator^ for sums.
		 */
		static M id();
	};

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
	 * \tparam N Any integer of floating point primitive type, \em or a type
	 *           that is an instance of Number and can be implicitly casted to
	 *           from the literal \c 0.
	 */
	template<typename N>
	struct sum_monoid {

		sum_monoid() = default;

		constexpr sum_monoid(N num)
		noexcept(std::is_nothrow_copy_constructible<N>::value)
			: n(num) {}

		constexpr sum_monoid(const sum_monoid& m)
		noexcept(std::is_nothrow_copy_constructible<N>::value)
			: n(m.n) {}

		constexpr sum_monoid(sum_monoid&& m)
		noexcept(std::is_nothrow_move_constructible<N>::value)
		   	: n(std::move(m.n)) {}

		~sum_monoid() = default;

		constexpr operator N () const noexcept {
			return n;
		}

		const sum_monoid& operator=(const sum_monoid& m)
		noexcept(std::is_nothrow_assignable<N, N>::value) {
			n = m.n;
			return *this;
		}

		const sum_monoid& operator=(sum_monoid&& m)
		noexcept(std::is_nothrow_move_assignable<N>::value) {
			n = std::move(m.n);
			return *this;
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
	 * Example usage:
	 * \code
	 *   template<typename M>
	 *   M foo(const M& m1, const M& m2); // defined elsewhere, uses monoid operations
	 *
	 *   void bar() {
	 *       foo(sum(1), sum(2));
	 *   }
	 * \endcode
	 */
	template<typename N>
	constexpr sum_monoid<N> sum(N num)
	noexcept(std::is_nothrow_constructible<sum_monoid<N>,N>::value) {
		return sum_monoid<N>(num);
	}

	/**
	 * Implementation of monoid identity for sums.
	 */
	template<typename N>
	struct monoid<sum_monoid<N>> {
		static constexpr sum_monoid<N> id()
		noexcept(std::is_nothrow_constructible<sum_monoid<N>,N>::value) {
			return sum(N(0));
		}
	};

	/**
	 * monoid::mappend implementation for sums of numbers.
	 */
	template<typename N>
	constexpr sum_monoid<N> mappend(
			const sum_monoid<N>& n1,
			const sum_monoid<N>& n2) {

		return n1 + n2;
	}

	/**
	 * monoid::operator^ implementation for sums of numbers.
	 */
	template<typename N>
	constexpr sum_monoid<N> operator^ (
			const sum_monoid<N>& n1,
			const sum_monoid<N>& n2) {

		return mappend(n1, n2);
	}

	/**
	 * Implementatino of monoid for numbers, when interpreted as products.
	 *
	 * The reason behind this struct is exactly the same as with sum_monoid.
	 *
	 * The behaviour of the product monoid is simple:
	 * \code
	 *   id() => N(1)
	 *   operator^ => N::operator*
	 * \endcode
	 *
	 * \tparam N Any integer of floating point primitive type, \em or a type
	 *           that is an instance of Number and can be implicitly casted to
	 *           from the literal \c 1.
	 */
	template<typename N>
	struct prod_monoid {
		prod_monoid() = default;

		constexpr prod_monoid(N num)
		noexcept(std::is_nothrow_copy_constructible<N>::value)
			: n(num) {}

		constexpr prod_monoid(const prod_monoid& m)
		noexcept(std::is_nothrow_copy_constructible<N>::value)
			: n(m.n) {}

		constexpr prod_monoid(prod_monoid&& m)
		noexcept(std::is_nothrow_move_constructible<N>::value)
			: n(std::move(m.n)) {}

		~prod_monoid() = default;

		const prod_monoid& operator= (const prod_monoid& m)
		noexcept(std::is_nothrow_assignable<N,N>::value) {
			n = m.n;
			return *this;
		}

		const prod_monoid& operator= (prod_monoid&& m)
		noexcept(std::is_nothrow_move_assignable<N>::value) {
			n = std::move(m.n);
			return *this;
		}

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
	 * Example usage:
	 * \code
	 *   template<typename M>
	 *   M foo(const M& m1, const M& m2); // defined elsewhere, uses monoid operations
	 *
	 *   void bar() {
	 *       foo(prod(1), prod(2));
	 *   }
	 * \endcode
	 */
	template<typename N>
	constexpr prod_monoid<N> prod(N n)
	noexcept(std::is_nothrow_constructible<prod_monoid<N>,N>::value) {
		return prod_monoid<N>(n);
	}

	/**
	 * Implementation of monoid identity for products.
	 */
	template<typename N>
	struct monoid<prod_monoid<N>> {
		static constexpr prod_monoid<N> id()
		noexcept(std::is_nothrow_constructible<prod_monoid<N>,N>::value) {
			return prod(N(1));
		}
	};

	/**
	 * monoid::mappend implementation for products of numbers.
	 */
	template<typename N>
	constexpr prod_monoid<N> mappend(
			const prod_monoid<N>& n1,
			const prod_monoid<N>& n2) {

		return n1 * n2;
	}

	/**
	 * monoid::operator^ implementation for products of numbers.
	 */
	template<typename N>
	constexpr prod_monoid<N> operator^ (
			const prod_monoid<N>& n1,
			const prod_monoid<N>& n2) {

		return mappend(n1, n2);
	}

	/**
	 * Implementation of monoid::id for tuples.
	 *
	 * Basically, id will simply generate a tuple of id:s. That is, a call
	 * to
	 * \code
	 *   monoid<std::tuple<t1, t2, ..., tN>>::id();
	 * \endcode
	 * is equivalent to
	 * \code
	 *   std::make_tuple(
	 *       monoid<t1>::id(),
	 *       monoid<t2>::id(),
	 *       ...,
	 *       monoid<tN>::id());
	 * \endcode
	 *   
	 */
	template<typename...Ts>
	struct monoid<std::tuple<Ts...>> {
		static std::tuple<Ts...> id() {
			return std::make_tuple(monoid<Ts>::id()...);
		}
	};

	// Unnamed private namespace for tuple mappend implementation
	namespace {
		template<std::size_t N, typename T>
		struct tupMappend {
			static void apply(T& ret, const T& t2) {
				tupMappend<N-1, T>::apply(ret, t2);
				std::get<N>(ret) = std::get<N>(ret) ^ std::get<N>(t2);
			}
		};

		template<typename T>
		struct tupMappend<0, T> {
			static void apply(T& ret, const T& t2) {
				std::get<0>(ret) = std::get<0>(ret) ^ std::get<0>(t2);
			}
		};
	}

	/**
	 * Implementation of monoid::mappend for tuples.
	 *
	 * The semantics are that mappend:ing two tuples is equivalent to
	 * mappend:ing every component of tuple 1 to its corresponding component
	 * in tuple 2, and then creating a resulting tuple out of all those values.
	 */
	template<typename...Ts>
	std::tuple<Ts...> mappend(
			const std::tuple<Ts...>& t1,
			const std::tuple<Ts...>& t2) {

		auto ret = t1;
		tupMappend<sizeof...(Ts)-1, std::tuple<Ts...>>::apply(ret, t2);
		return ret;
	}

	template<typename...Ts>
	std::tuple<Ts...> operator^ (const std::tuple<Ts...>& t1, const std::tuple<Ts...>& t2) {
		return mappend(t1, t2);
	}

}

#endif

