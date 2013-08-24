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
#include "concepts/foldable.h"
#include "concepts/monad.h"

namespace ftl {

	/**
	 * \defgroup list List
	 *
	 * Concept implementations and utility functions for std::list.
	 *
	 * \code
	 *   #include <ftl/list.h>
	 * \endcode
	 *
	 * This module adds the following concept instances to std::list:
	 * - \ref monoid
	 * - \ref foldable
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 *
	 * \par Dependencies
	 * - <list>
	 * - \ref foldable
	 * - \ref monad
	 */

	/**
	 * Specialisation of re_parametrise for lists.
	 *
	 * This makes sure the allocator is also properly parametrised on the
	 * new element type.
	 *
	 * \ingroup list
	 */
	template<typename T, typename U, typename A>
	struct re_parametrise<std::list<T,A>,U> {
	private:
		using Au = typename re_parametrise<A,U>::type;

	public:
		using type = std::list<U,Au>;
	};

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy \ref fn`<`\ref container`<U>(T)>`
	 *
	 * \ingroup list
	 */
	template<
			typename F,
			typename T,
			typename A,
			typename U = typename result_of<F(T)>::value_type,
			typename Au = typename re_parametrise<A,U>::type
	>
	std::list<U,Au> concatMap(F&& f, const std::list<T,A>& l) {

		std::list<U,Au> result;
		auto nested = std::forward<F>(f) % l;

		for(auto& el : nested) {
			result.splice(result.end(), el);
		}

		return result;
	}

	template<
			typename F,
			typename T,
			typename A,
			typename U = typename result_of<F(T)>::value_type,
			typename Au = typename re_parametrise<A,U>::type
	>
	std::list<U,Au> concatMap(F&& f, std::list<T,A>&& l) {

		std::list<U,Au> result;
		auto nested = std::forward<F>(f) % std::move(l);

		for(auto& el : nested)
			result.splice(result.end(), el);

		return result;
	}

	/**
	 * Monoid implementation for std::list.
	 *
	 * The identity element is (naturally) the empty list, and the append
	 * operation is (again, naturally) to append the second list to the first.
	 *
	 * \ingroup list
	 */
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
			return l1;
		}

		static std::list<Ts...> append(
				const std::list<Ts...>& l1,
				std::list<Ts...>&& l2) {
			l2.insert(l2.begin(), l1.begin(), l1.end());
			return l2;
		}

		static std::list<Ts...> append(
				std::list<Ts...>&& l1,
				std::list<Ts...>&& l2) {
			if(l1.get_allocator() == l2.get_allocator()) {
				l1.splice(l1.end(), std::move(l2));
			}

			else {
				std::move(l2.begin(), l2.end(), std::back_inserter(l1));
			}

			return l1;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad implementation for `std::list`.
	 *
	 * Behaves like most collections; which is to say, the monad can be said
	 * to model nondeterministic computations.
	 *
	 * \ingroup list
	 */
	template<typename T, typename A>
	struct monad<std::list<T,A>>
	: deriving_bind<insertable_container<std::list<T,A>>>
	, deriving_join<std::list<T,A>>
	, deriving_apply<std::list<T,A>> {

		/// Alias to make type signatures cleaner
		template<typename U>
		using list = typename re_parametrise<std::list<T,A>,U>::type;

		/**
		 * Produces a singleton list.
		 *
		 * That is, pure generates a one element list, where that single element
		 * is `a`.
		 */
		static list<T> pure(const T& a) {
			list<T> l{};
			l.emplace_back(a);
			return l;
		}

		/// \overload
		static list<T> pure(T&& a) {
			list<T> l{};
			l.emplace_back(std::move(a));
			return l;
		}

		/**
		 * Applies `f` to each element.
		 *
		 * The result of each call to `f` is collected and returned as an
		 * element in the list returned by the call to map.
		 *
		 * Example:
		 * \code
		 *   auto l = ftl::fmap([](int x){ return x+1; }, list<int>{1,2,3});
		 *   // l == list<int>{2,3,4}
		 * \endcode
		 */
		template<typename F, typename U = result_of<F(T)>>
		static list<U> map(F&& f, const list<T>& l) {
			list<U> ret;
			for(const auto& e : l) {
				ret.push_back(f(e));
			}

			return ret;
		}

		/**
		 * R-value overload.
		 *
		 * Moves elements out of `l` when applying `f`.
		 */
		template<
				typename F,
				typename U = result_of<F(T)>,
				typename = typename std::enable_if<
					!std::is_same<T,U>::value
				>::type
		>
		static list<U> map(F&& f, list<T>&& l) {
			list<U> ret;
			for(auto& e : l) {
				ret.push_back(f(std::move(e)));
			}

			return ret;
		}

		/**
		 * Move optimised version enabled when `f` does not change domain.
		 *
		 * Basically, if the return type of `f` is the same as its parameter
		 * type and `l` is a temporary (rvalue reference), then `l` is mutated.
		 * instead of copied. This means no copies are made.
		 */
		template<
				typename F,
				typename U = result_of<F(T)>,
				typename = typename std::enable_if<
					std::is_same<T,U>::value
				>::type
		>
		static list<T> map(F&& f, list<T>&& l) {
			for(auto& e : l) {
				e = f(e);
			}

			return l;
		}

#ifdef DOCUMENTATION_GENERATOR
		/**
		 * Joins nested lists by way of concatenation.
		 *
		 * The resulting list contains every element of every list contained
		 * in the original list. Relative order is preserved (from the
		 * perspective of depth first iteration).
		 */
		static list<T> join(const list<list<T>>& l);

		/// \overload
		static list<T> join(list<list<T>>&& l);

		/**
		 * Monad bind operation.
		 *
		 * Can be viewed as a non-deterministic computation: `l` is a list of
		 * possible values, each of which we apply `f` to. As `f` itself is also
		 * non-deterministic, it may return several possible answers for each
		 * element in `l`. Finally, all of the results are collected in a flat
		 * list.
		 *
		 * \note `f` is allowed to return _any_ \ref fwditerable, not only
		 *       lists. The final result, however, is always a list.
		 *
		 * Example:
		 * \code
		 *   auto l =
		 *       list<int>{1, 2, 3}
		 *       >>= [](int x){ return list<int>{x-1, x, x+1}; };
		 *
		 *   // Note how each group of three corresponds to one input element
		 *   // l == list<int>{0,1,2, 1,2,3, 2,3,4}
		 * \endcode
		 */
		template<
				typename F,
				typename Cu = result_of<F(T)>,
				typename U = concept_parameter<Cu>,
				typename = typename std::enable_if<
					ForwardIterable<Cu>()
				>::type
		>
		static list<U> bind(const list<T>& l, F&& f);

		/// \overload
		template<
				typename F,
				typename Cu = result_of<F(T)>,
				typename U = concept_parameter<Cu>,
				typename = typename std::enable_if<
					ForwardIterable<Cu>()
				>::type
		>
		static list<U> bind(list<T>&& l, F&& f);
#endif

		static constexpr bool instance = true;
	};

	/**
	 * Foldable instance for std::lists.
	 *
	 * \ingroup list
	 */
	template<typename T, typename A>
	struct foldable<std::list<T,A>>
	: deriving_foldable<bidirectional_iterable<std::list<T,A>>> {};

}

#endif

