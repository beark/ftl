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
#ifndef FTL_ORD_H
#define FTL_ORD_H

#include "monoid.h"

namespace ftl {
	/**
	 * Encapsulation of the concept of an ordering.
	 *
	 * In essence, an ordering can be either 'less than' (Lt), 'equal' (Eq), or
	 * 'greater than' (Gt).
	 *
	 * \par Concepts implemented by ord
	 * \li FullyConstructible
	 * \li TriviallyDestructible
	 * \li Assignable
	 * \li Comparable
	 * \li Monoid
	 */
	class ord {
	public:
		/**
		 * Enum of the values an ord can take on.
		 *
		 * Because ord is implicitly constructible from an ordering, whenever you
		 * want to return an ord, you can just be lazy and do something like
		 * \code
		 *   ord example() {
		 *       return ord::Lt;
		 *   }
		 * \endcode
		 * instead.
		 *
		 * Comparison and assignment also works as expected (as long as the lhs
		 * is an ord and the rhs is an ordering).
		 */
		enum ordering {
			Lt = 0, Eq, Gt
		};

		constexpr ord() noexcept {}

		/**
		 * Construct from an int.
		 *
		 * The purpose of this constructor is to allow easy construction from
		 * cstlib's strcmp and similar. Anything less than 0 becomes Lt, 0 is
		 * Eq and >0 is Gt.
		 */
		explicit constexpr ord(int n) noexcept : o(n < 0 ? Lt : (n > 0 ? Gt : Eq)) {}

		constexpr ord(const ord& order) noexcept : o(order.o) {}
		constexpr ord(ord&& order) noexcept : o(std::move(order.o)) {}
		constexpr ord(const ordering& order) noexcept : o(order) {}
		constexpr ord(ordering&& order) noexcept : o(std::move(order)) {}
		~ord() noexcept = default;

		constexpr bool operator== (const ord& order) noexcept {
			return o == order.o;
		}

		constexpr bool operator== (ordering order) noexcept {
			return o == order;
		}

		constexpr bool operator!= (const ord& order) noexcept {
			return o != order.o;
		}

		constexpr bool operator!= (ordering order) noexcept {
			return o != order;
		}

		constexpr bool operator< (const ord& order) noexcept {
			return o < order.o;
		}

		constexpr bool operator<= (const ord& order) noexcept {
			return o <= order.o;
		}

		constexpr bool operator> (const ord& order) noexcept {
			return o > order.o;
		}

		constexpr bool operator>= (const ord& order) noexcept {
			return o >= order.o;
		}

		const ord& operator= (const ord& order) noexcept {
			o = order.o;
			return *this;
		}

		const ord& operator= (ord&& order) noexcept {
			o = std::move(order.o);
			return *this;
		}

	private:
		ordering o = Eq;
	};

	/**
	 * Orderable concept.
	 */
	template<typename T>
	struct orderable {
		/**
		 * Compares two orderables of the same type.
		 *
		 * The default implementation should very rarely need to be overided.
		 *
		 * \return ord::Lt if lhs < rhs, ord::Eq if they're equal, and
		 *         otherwise ord::Gt.
		 */
		static ord compare(const T& lhs, const T& rhs) {
			return lhs < rhs ? ord::Lt : (lhs == rhs ? ord::Eq : ord::Gt);
		}
	};

	/**
	 * Convenience function to more easily compare things.
	 *
	 * Simply invokes \c orderable<T>::compare.
	 */
	template<typename T>
	ord compare(const T& a, const T& b) {
		return orderable<T>::compare(a, b);
	}

	/**
	 * Convenience function to get a comparator for a certain type.
	 */
	template<typename T>
	function<ord,const T&,const T&> getComparator() {
		return [] ( const T& a, const T& b) { return compare(a, b); };
	}

	/**
	 * Monoid instance for ord.
	 *
	 * Quite neat in combination with the monoid instance for function.
	 *
	 * Semantics:
	 * \code
	 *   id() <=> Eq
	 *   append(a,b) <=> a, unless a == Eq, then b
	 * \endcode
	 */
	template<>
	struct monoid<ord> {
		static constexpr ord id() noexcept {
			return ord::Eq;
		}

		static constexpr ord append(ord o1, ord o2) noexcept {
			return (o1 == ord::Lt) ? o1 : (o1 == ord::Eq ? o2 : o1);
		}

		static constexpr bool instance = true;
	};

	/**
	 * Convenience function to compare objects by getter.
	 *
	 * \tparam R Must satisfy Orderable.
	 *
	 * \param method Getter method to do comparison by.
	 *
	 * \return Function that compares two objects by first applying method
	 *         to each object and then compare the two results using their
	 *         Ordering instance.
	 */
	template<typename A, typename R>
	function<ord,const A&,const A&> comparing(R (A::*method)() const) {
		return [=] (const A& a, const A& b) {
			return orderable<R>::compare((a.*method)(), (b.*method)());
		};
	}

	/**
	 * Convenience function to ease integration with stdlib's sort.
	 *
	 * \param cmp Compare function to apply internally.
	 *
	 * \return A function that returns true if the result of performing the
	 *         given comparison is Lt.
	 */
	template<typename A>
	function<bool,A,A> lessThan(const function<ord,A,A>& cmp) {
		return [=] (A a, A b) {
			return cmp(a, b) == ord::Lt;
		};
	}

	/**
	 * Convenience function to ease integration with stdlib's sort.
	 *
	 * \param cmp Compare function to apply internally.
	 *
	 * \return A function that returns true if the result of performing the
	 *         given comparison is Gt.
	 */
	template<typename A>
	function<bool,A,A> greaterThan(const function<ord,A,A>& cmp) {
		return [=] (A a, A b) {
			return cmp(a, b) == ord::Gt;
		};
	}
	/**
	 * Convenience function to ease integration with stdlib's sort.
	 *
	 * \param cmp Compare function to apply internally.
	 *
	 * \return A function that returns true if the result of performing the
	 *         given comparison is Eq.
	 */
	template<typename A>
	function<bool,A,A> equal(const function<ord,A,A>& cmp) {
		return [=] (A a, A b) {
			return cmp(a, b) == ord::Eq;
		};
	}
}

#endif

