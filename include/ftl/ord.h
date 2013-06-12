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

#include "type_traits.h"
#include "monoid.h"

namespace ftl {
	/**
	 * \page orderablepg Orderable
	 *
	 * Anything that can be ordered in some strict sense.
	 *
	 * In essence, any type that defines the operators `<`, `==`, and `>`.
	 * Technically, the Orderable concept also requires \ref eq, but there is
	 * nothing in FTL that enforces this.
	 *
	 * \see \ref ord (module)
	 */

	/**
	 * \defgroup ord Ord
	 *
	 * \ref orderablepg concept, `ord` data type, concept instances, and related.
	 *
	 * \code
	 *   #include <ftl/ord.h>
	 * \endcode
	 *
	 * The `ord` data type represents an ordering, in the form of "less than",
	 * "equal", or "greater than". The Orderable concept relies on this data
	 * type to order values.
	 *
	 * \par Dependencies
	 * - \ref monoid
	 */

	/**
	 * Encapsulation of the concept of an ordering.
	 *
	 * In essence, an ordering can be either 'less than' (Lt), 'equal' (Eq), or
	 * 'greater than' (Gt).
	 *
	 * \par Concepts implemented by ord
	 * \li \ref fullycons
	 * \li \ref assignable
	 * \li \ref orderable
	 * \li \ref monoid
	 *
	 * \ingroup ord
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

		/// Default to Eq.
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
	 * \interface orderable
	 *
	 * Concrete definition of \ref orderablepg concept.
	 *
	 * In essence, instances of Orderable can be ordered in some strict sense.
	 *
	 * For a type to become an instance of Orderable, it must either implement
	 * all of the operators `<`, `==`, and `>`, or it must specialise this
	 * struct. Both of these work equally well.
	 *
	 * A type that is orderable must also implement \ref eq.
	 *
	 * \ingroup ord
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

		static constexpr bool instance =
			has_eq<T>::value
			&& has_neq<T>::value
			&& has_lt<T>::value
			&& has_gt<T>::value;
	};

	/**
	 * Convenience function to more easily compare things.
	 *
	 * Simply invokes \c orderable<T>::compare.
	 *
	 * \ingroup ord
	 */
	template<typename T>
	auto compare(const T& a, const T& b)
	-> decltype(orderable<T>::compare(a, b)) {
		return orderable<T>::compare(a, b);
	}

	/**
	 * Convenience function to get a comparator for a certain type.
	 *
	 * \ingroup ord
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
	 *
	 * \ingroup ord
	 * \ingroup monoid
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
	 * \tparam R Must satisfy orderable.
	 *
	 * \param method Getter method to do comparison by.
	 *
	 * \return Function that compares two objects by first applying method
	 *         to each object and then compare the two results using their
	 *         Orderable instance.
	 *
	 * \ingroup ord
	 */
	template<typename A, typename R>
	function<ord,const A&,const A&> comparing(R (A::*method)() const) {
		return [=] (const A& a, const A& b) {
			return orderable<R>::compare((a.*method)(), (b.*method)());
		};
	}

	/**
	 * Convenience function to compare objects by "converter".
	 *
	 * \tparam F Must satisfy \ref fn<orderable(A)>, or in other words, must
	 *           return something that is orderable.
	 *
	 * The use case for this convenience function is similar to the comparing
	 * function that works with getter methods, but in this case the comparison
	 * is made after a free function has been aplied to the original values.
	 *
	 * Example:
	 * \code
	 *   list<maybe<string>> l{value("abc"), value("de"), value("f")};
	 *   sort(l.begin(), l.end(),
	 *           lessThan(comparing([] (const maybe<string>& m) -> size_t {
	 *       if(m) return m->size();
	 *       return 0;
	 *   })));
	 * \endcode
	 * The above would sort `l` in ascending order of string length. Any element
	 * that is `nothing` would be considered to be of length `0`.
	 *
	 * \ingroup ord
	 */
	template<typename A, typename B>
	function<ord,const A&,const A&> comparing(function<B,A> cmp) {
		return [=] (const A& a, const A& b) {
			return orderable<typename std::decay<B>::type>::compare(
					f(a), f(b));
		};
	}

	/**
	 * Convenience function to ease integration with stdlib's sort.
	 *
	 * \param cmp Compare function to apply internally.
	 *
	 * \return A function that returns true if the result of performing the
	 *         given comparison is Lt.
	 *
	 * \ingroup ord
	 */
	template<typename A>
	function<bool,const A&,const A&> lessThan(
			function<ord,const A&,const A&> cmp) {
		return [=] (const A& a, const A& b) {
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
	 *
	 * \ingroup ord
	 */
	template<typename A>
	function<bool,const A&,const A&> greaterThan(
			function<ord,const A&,const A&> cmp) {
		return [=] (const A& a, const A& b) {
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
	 *
	 * \ingroup ord
	 */
	template<typename A>
	function<bool,const A&,const A&> equal(
			function<ord,const A&,const A&> cmp) {
		return [=] (A a, A b) {
			return cmp(a, b) == ord::Eq;
		};
	}
}

#endif

