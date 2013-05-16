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
#ifndef FTL_LIST_H
#define FTL_LIST_H

#include <list>
#include "foldable.h"
#include "monad.h"

namespace ftl {

	/**
	 * \defgroup list List
	 *
	 * Doubly linked list, its concept implementations, and so on.
	 *
	 * \code
	 *   #include <ftl/list.h>
	 * \endcode
	 *
	 * The list type is in reality just a simple type alias of std::list. The
	 * main reason for this is because the template parameter signature of
	 * std::list is not compatible with some of the concepts FTL defines.
	 *
	 * \par Dependencies
	 * - <list>
	 * - \ref foldable
	 * - \ref monad
	 */

	/**
	 * Workaround to make lists compatible with the functor series of concepts.
	 *
	 * If you require a different allocator than \c std::allocator, then make
	 * a similar type synonym and give it a monad instance of its own.
	 *
	 * \par Concepts
	 * As an ordinary std::list, with the additions:
	 * - \ref monoid
	 * - \ref monad
	 *
	 * \ingroup list
	 */
	template<typename T>
	using list = std::list<T, std::allocator<T>>;

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy \ref fn`<`\ref container`<B>(A)>`
	 *
	 * \ingroup list
	 */
	template<
		typename F,
		typename A,
		typename B = typename decayed_result<F(A)>::type::value_type>
	list<B> concatMap(F f, const list<A>& l) {

		list<B> result;
		auto nested = f % l;

		for(auto& el : nested) {
			for(auto& e : el) {
				result.push_back(e);
			}
		}

		return result;
	}

	/**
	 * Monoid implementation for list.
	 *
	 * The identity element is (naturally) the empty list, and the append
	 * operation is (again, naturally) to append the second list to the first.
	 *
	 * \ingroup list
	 */
	// TODO: Make monoid instance for ftl::list too, because apparently that's
	// necessary
	template<typename...Ts>
	struct monoid<std::list<Ts...>> {
		static std::list<Ts...> id() {
			return std::list<Ts...>();
		}

		static std::list<Ts...> append(
				const std::list<Ts...>& l1,
				const std::list<Ts...>& l2) {
			auto l3 = l1;
			l3.insert(l3.end(), l2.begin(), l2.end());
			return l3;
		}

		// For performance reasons, we give overloads for cases where we don't
		// have to copy any of the lists
		static std::list<Ts...> append(
				std::list<Ts...>&& l1,
				const std::list<Ts...>& l2) {
			l1.insert(l1.end(), l2.begin(), l2.end());
			return std::move(l1);
		}

		static std::list<Ts...> append(
				const std::list<Ts...>& l1,
				std::list<Ts...>&& l2) {
			l2.insert(l2.end(), l1.begin(), l1.end());
			return std::move(l2);
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad implementation for list.
	 *
	 * \note This concept instance is only valid for the list alias commonly
	 *       used in FTL. It is not valid for regular std::lists. If you
	 *       require the use of an allocator aside from std::allocator, then
	 *       you must create your own alias of std::list and give it a monad
	 *       instance.
	 *
	 * \ingroup list
	 */
	template<>
	struct monad<list> {

		/**
		 * Produces a singleton list.
		 *
		 * That is, pure generates a one element list, where that single element
		 * is a.
		 */
		template<typename A>
		static list<A> pure(const A& a) {
			return list<A>{a};
		}

		template<typename A, typename Alloc>
		static list<A> pure(A&& a) {
			return list<A>{std::move(a)};
		}

		/**
		 * Applies f to each element.
		 *
		 * The result of each call to f is collected and returned as an element
		 * in the list returned by the call to map.
		 */
		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type>
		static list<B> map(F&& f, const list<A>& l) {
			list<B> ret;
			for(const auto& e : l) {
				ret.push_back(f(e));
			}

			return ret;
		}

		/**
		 * Move optimised version enabled when f does not change domain.
		 *
		 * Basically, if the return type of f is the same as its parameter type
		 * and l is a temporary (rvalue reference), then l is re-used. This
		 * means no copies are made.
		 */
		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type,
			typename = typename std::enable_if<std::is_same<A,B>::value>::type>
		static list<A> map(F&& f, list<A>&& l) {
			for(auto& e : l) {
				e = f(e);
			}

			// Make sure compiler remembers l is a temporary.
			// This is safe, because we're returning it to the context from
			// whence it came.
			return std::move(l);
		}

		/// Equivalent of flip(concatMap)
		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type::value_type>
		static list<B> bind(const list<A>& l, F&& f) {
			return concatMap(std::forward<F>(f), l);
		}

		static constexpr bool instance = true;
	};

	/**
	 * Foldable instance for std::lists.
	 *
	 * \ingroup list
	 */
	template<>
	struct foldable<std::list>
	: fold_default<std::list>, foldMap_default<std::list> {
		template<
				typename Fn,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						A,
						typename decayed_result<Fn(B,A)>::type
						>::value
					>::type,
				typename...Ts>
		static A foldl(Fn&& fn, A z, const std::list<B,Ts...>& l) {
			for(auto& e : l) {
				z = fn(z, e);
			}

			return z;
		}

		template<
				typename Fn,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						B,
						typename decayed_result<Fn(A,B)>::type
						>::value
					>::type,
				typename...Ts>
		static B foldr(Fn&& fn, B z, const std::list<A,Ts...>& l) {
			for(auto it = l.rbegin(); it != l.rend(); ++it) {
				z = fn(*it, z);
			}

			return z;
		}

		static constexpr bool instance = true;
	};

}

#endif

