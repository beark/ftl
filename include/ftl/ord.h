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

#include "concepts/orderable.h"
#include "concepts/monoid.h"

namespace ftl {
	/**
	 * \defgroup ord Ord
	 *
	 * The `ord` data type, concept instances, and related utilities.
	 *
	 * \code
	 *   #include <ftl/ord.h>
	 * \endcode
	 *
	 * The `ord` data type represents an ordering, in the form of "less than",
	 * "equal", or "greater than".
	 *
	 * \par Dependencies
	 * - \ref orderable
	 * - \ref monoid
	 */

	/**
	 * Data type representing an ordering relationship.
	 *
	 * In essence, an ordering can be either 'less than' (Lt), 'equal' (Eq), or
	 * 'greater than' (Gt).
	 *
	 * \par Concepts implemented by ord
	 * - \ref fullycons
	 * - \ref assignable
	 * - \ref orderable
	 * - \ref monoid
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

		/// Defaults to Eq.
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

		// TODO: constexpr in c++14
		ord(ord&& order) noexcept : o(std::move(order.o)) {}

		constexpr ord(const ordering& order) noexcept : o(order) {}

		// TODO: constexpr in c++14
		ord(ordering&& order) noexcept : o(std::move(order)) {}
		
		~ord() noexcept = default;

		constexpr bool operator== (const ord& order) const noexcept {
			return o == order.o;
		}

		constexpr bool operator== (ordering order) const noexcept {
			return o == order;
		}

		constexpr bool operator!= (const ord& order) const noexcept {
			return o != order.o;
		}

		constexpr bool operator!= (ordering order) const noexcept {
			return o != order;
		}

		constexpr bool operator< (const ord& order) const noexcept {
			return o < order.o;
		}

		constexpr bool operator<= (const ord& order) const noexcept {
			return o <= order.o;
		}

		constexpr bool operator> (const ord& order) const noexcept {
			return o > order.o;
		}

		constexpr bool operator>= (const ord& order) const noexcept {
			return o >= order.o;
		}

		ord& operator= (const ord& order) noexcept {
			o = order.o;
			return *this;
		}

		ord& operator= (ord&& order) noexcept {
			o = std::move(order.o);
			return *this;
		}

	private:
		ordering o = Eq;
	};

	/**
	 * Comparison function for \ref orderablepg objects.
	 *
	 * \ingroup ord
	 */
	template<
			typename Ord,
			typename = Requires<Orderable<Ord>()>
	>
	ord compare(const Ord& lhs, const Ord& rhs) {
		return lhs < rhs ? ord::Lt : (lhs == rhs ? ord::Eq : ord::Gt);
	}

	/**
	 * Convenience function to get a comparator for a certain type.
	 *
	 * This can be a very useful function for compositional purposes. I.e.,
	 * it is possible to compose a comparator using the composite \ref monoidpg
	 * instance of `ftl::function` and `ftl::ord`.
	 *
	 * Example:
	 * \code
	 *   using ftl::asc;
	 *   using ftl::getComparator;
	 *   using ftl::operator^;
	 *
	 *   std::sort(collection.begin(), collection.end(),
	 *   	asc(comparing(&SomeType::Property) ^ getComparator<SomeType>()));
	 * \endcode
	 *
	 * \ingroup ord
	 */
	template<typename T>
	function<ord(const T&,const T&)> getComparator() {
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
	 */
	template<>
	struct monoid<ord> {
		// TODO: constexpr c++14
		static ord id() noexcept {
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
	 * \tparam R Must satisfy \ref orderablepg.
	 *
	 * \param method Getter method to do comparison by.
	 *
	 * \return Function that compares two objects by first applying method
	 *         to each object and then compare the two results using their
	 *         Orderable instance.
	 *
	 * Example:
	 * \code
	 *     list<string> l{"aaaa", "a", "aaa", "aa"};
	 *
	 *     sort(l.begin(), l.end(), asc(comparing(&string::size)));
	 * \endcode
	 * Resulting list: `{"a", "aa", "aaa", "aaaa"}`
	 *
	 * \ingroup ord
	 */
	template<
			typename A,
			typename R,
			typename = Requires<Orderable<R>()>
	>
	function<ord(const A&,const A&)> comparing(R (A::*method)() const) {
		return [=] (const A& a, const A& b) {
			return compare((a.*method)(), (b.*method)());
		};
	}

	/**
	 * Convenience function to compare objects by "converter".
	 *
	 * \tparam F Must satisfy \ref fn`<`\ref orderablepg`(A)>`, or in other
	 *           words, must return something that is orderable.
	 *
	 * The use case for this convenience function is similar to the comparing
	 * function that works with getter methods, but in this case the comparison
	 * is made after a free function has been aplied to the original values.
	 *
	 * Example:
	 * \code
	 *   list<maybe<string>> l{value("abc"), value("de"), value("f")};
	 *
	 *   sort(l.begin(), l.end(), asc(comparing(
	 *       [] (const maybe<string>& m) -> size_t { return m ? m->size() : 0; }
	 *   )));
	 * \endcode
	 * The above would sort `l` in ascending order of string length. Any element
	 * that is `nothing` would be considered to be of length `0`.
	 *
	 * \ingroup ord
	 */
	template<
			typename A,
			typename B,
			typename = Requires<Orderable<B>()>
	>
	function<ord(A,A)> comparing(function<B(A)> f) {
		return [=] (A a, A b) {
			return compare(f(a), f(b));
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
	function<bool(const A&,const A&)> asc(
			function<ord(const A&,const A&)> cmp) {
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
	function<bool(const A&,const A&)> desc(
			function<ord(const A&,const A&)> cmp) {
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
	function<bool(const A&,const A&)> equal(
			function<ord(const A&,const A&)> cmp) {
		return [=] (A a, A b) {
			return cmp(a, b) == ord::Eq;
		};
	}
}

#endif

