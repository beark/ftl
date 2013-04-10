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
#include "type_functions.h"
#include "monoid.h"

/**
 * \file list.h
 *
 * Various concept implementations for std::lists.
 */

namespace ftl {

	/**
	 * Implementation of mappable::map for lists.
	 *
	 * Applies to cases where the allocator type is dependant on the element
	 * type.
	 */
	template<
		typename F,
		typename A,
		template<typename,typename...> class Alloc,
		typename B = typename decayed_result<F(A)>::type,
		typename...AllocArgs>
	auto map(const F& f, const std::list<A, Alloc<A, AllocArgs...>>& l)
	-> std::list<B, Alloc<B, AllocArgs...>> {
		std::list<B, Alloc<B>> result;
		for(const auto& e : l)
			result.push_back(f(e));

		return result;
	}

	/**
	 * Implementation of mappable::map for lists.
	 *
	 * Applies to cases where the allocator type is independant of the element
	 * type.
	 */
	template<
		typename F,
		typename Alloc,
		typename A,
		typename B = typename decayed_result<F(A)>::type>
	std::list<B, Alloc> map(const F& f, const std::list<A, Alloc>& l) {
		std::list<B, Alloc> result;
		for(const auto& e : l)
			result.push_back(f(e));

		return result;
	}

	/**
	 * Implementation of mappable::map (method version) for lists.
	 */
	template<
		typename A,
		typename B,
		typename Alloc,
		typename...Ps>
	std::list<A, Alloc>& map(
			B (A::*method)(Ps...),
			std::list<A, Alloc>& l,
			const Ps&...ps) {
		for(auto& e : l)
			(e.*method)(ps...);

		return l;
	}

	/**
	 * Implementation of mappable::concatMap for lists.
	 *
	 * Applies to lists with allocators whose type depends on the \c value_type
	 * of the list.
	 */
	template<
		typename F,
		template <typename, typename...> class Alloc,
		typename A,
		typename B = typename decayed_result<F(A)>::type::value_type,
		typename...AllocArgs>
	auto concatMap(const F& f, const std::list<A, Alloc<A, AllocArgs...>>& l)
	-> std::list<B, Alloc<B, AllocArgs...>> {

		std::list<B, Alloc<B, AllocArgs...>> result;
		auto nested = map(f, l);

		for(auto& el : nested) {
			for(auto& e : el) {
				result.push_back(e);
			}
		}

		return result;
	}

	/**
	 * Implementation of mappable::concatMap for lists.
	 *
	 * Applies to lists where the allocator is not dependant on the
	 * \c value_type of the list.
	 */
	template<
		typename F,
		typename Alloc,
		typename A,
		typename B = typename decayed_result<F(A)>::type::value_type>
	auto concatMap(const F& f, const std::list<A, Alloc>& l)
	-> std::list<B, Alloc> {

		std::list<B, Alloc> result;
		auto nested = map(f, l);

		for(auto& el : nested) {
			for(auto& e : el) {
				result.push_back(e);
			}
		}

		return result;
	}

	/**
	 * Implementation of functor for lists
	 */
	template<
		typename F,
		typename A,
		typename Alloc>
	auto fmap(const F& f, const std::list<A, Alloc>& l)
	-> decltype(map(f, l)) {
		return map(f, l);
	}

	/**
	 * Implementation of monad::bind for lists.
	 *
	 * Defined essentially just as \c flip(concatMap).
	 */
	template<
		typename F,
		template <typename, typename...> class Alloc,
		typename A,
		typename B = typename decayed_result<F(A)>::type,
		typename...AllocArgs>
	auto bind(const std::list<A, Alloc<A, AllocArgs...>>& l, const F& f)
	-> std::list<B, Alloc<B, AllocArgs...>> {
		return concatMap(f, l);
	}

	/**
	 * Implementation of monoid for lists.
	 *
	 * The identity element is (naturally) the empty list, and the append
	 * operation is (again, naturally) to append the second list to the first..
	 */
	template<typename...Ps>
	struct monoid<std::list<Ps...>> {
		static std::list<Ps...> id() {
			return std::list<Ps...>();
		}

		static std::list<Ps...> append(
				const std::list<Ps...>& l1,
				const std::list<Ps...>& l2) {
			auto l3 = l1;
			l3.insert(l3.end(), l2.begin(), l2.end());
			return l3;
		}
	};

	template<typename...Ps>
	std::list<Ps...> operator^(const std::list<Ps...>& l1, const std::list<Ps...>& l2) {
		return monoid<std::list<Ps...>>::append(l1, l2);
	}


}

#endif

