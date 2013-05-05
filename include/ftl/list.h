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
#include "monoid.h"
#include "monad.h"

/**
 * \file list.h
 *
 * Various concept implementations for std::lists.
 */

namespace ftl {

	/**
	 * Workaround to make lists compatible with the functor series of concepts.
	 *
	 * If you require a different allocator than \c std::allocator, then make
	 * a similar type synonym and give it a monad instance of its own.
	 */
	template<typename T>
	using list = std::list<T, std::allocator<T>>;

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy Function<Container<B>(A)>
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
	 */
	template<typename...Ts>
	struct monoid<list<Ts...>> {
		static list<Ts...> id() {
			return list<Ts...>();
		}

		static list<Ts...> append(
				const list<Ts...>& l1,
				const list<Ts...>& l2) {
			auto l3 = l1;
			l3.insert(l3.end(), l2.begin(), l2.end());
			return l3;
		}

		// For performance reasons, we give overloads for cases where we don't
		// have to copy any of the lists

		static list<Ts...> append(
				list<Ts...>&& l1,
				const list<Ts...>& l2) {
			l1.insert(l1.end(), l2.begin(), l2.end());
			return std::move(l1);
		}

		static list<Ts...> append(
				const list<Ts...>& l1,
				list<Ts...>&& l2) {
			l2.insert(l2.end(), l1.begin(), l1.end());
			return std::move(l2);
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad implementation for list.
	 */
	template<>
	struct monad<list> {

		/// Produces a singleton list
		template<typename A>
		static list<A> pure(const A& a) {
			return list<A>{a};
		}

		template<typename A, typename Alloc>
		static list<A> pure(A&& a) {
			return list<A>{std::move(a)};
		}

		/// Applies f to each element
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

}

#endif

