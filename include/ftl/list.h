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
	template<typename T, typename U, template<typename> class A>
	struct re_parametrise<std::list<T,A<T>>,U> {
		using type = std::list<U,A<U>>;
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
			template<typename> class A,
			typename U = typename decayed_result<F(T)>::type::value_type
	>
	std::list<U,A<U>> concatMap(F&& f, const std::list<T,A<T>>& l) {

		std::list<U,A<U>> result;
		auto nested = std::forward<F>(f) % l;

		for(auto& el : nested) {
			result.splice(result.end(), el);
		}

		return result;
	}

	template<
			typename F,
			typename T,
			template<typename> class A,
			typename U = typename decayed_result<F(T)>::type::value_type
	>
	std::list<U,A<U>> concatMap(F&& f, std::list<T,A<T>>&& l) {

		std::list<U,A<U>> result;
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
			return std::move(l1);
		}

		static std::list<Ts...> append(
				const std::list<Ts...>& l1,
				std::list<Ts...>&& l2) {
			l2.insert(l2.begin(), l1.begin(), l1.end());
			return std::move(l2);
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
	 * Monad implementation for std::list.
	 *
	 * Note that this instance is only valid for std::lists using an
	 * allocator of the form `template<typename> class Allocator`, where the
	 * template parameter to the allocator is the elment type of the list.
	 *
	 * \ingroup list
	 */
	template<typename T, template<typename> class A>
	struct monad<std::list<T,A<T>>> {

		/**
		 * Produces a singleton list.
		 *
		 * That is, pure generates a one element list, where that single element
		 * is `a`.
		 */
		static std::list<T,A<T>> pure(const T& a) {
			std::list<T,A<T>> l{};
			l.emplace_back(a);
			return l;
		}

		/// \overload
		static std::list<T,A<T>> pure(T&& a) {
			std::list<T,A<T>> l{};
			l.emplace_back(std::move(a));
			return l;
		}

		/**
		 * Applies f to each element.
		 *
		 * The result of each call to f is collected and returned as an element
		 * in the list returned by the call to map.
		 */
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static std::list<U,A<U>> map(F&& f, const std::list<T,A<T>>& l) {
			std::list<U,A<U>> ret;
			for(const auto& e : l) {
				ret.push_back(f(e));
			}

			return ret;
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type,
				typename = typename std::enable_if<
					!std::is_same<T,U>::value
				>::type
		>
		static std::list<U,A<U>> map(F&& f, std::list<T,A<T>>&& l) {
			std::list<U,A<U>> ret;
			for(auto& e : l) {
				ret.push_back(f(std::move(e)));
			}

			return ret;
		}

		/**
		 * Move optimised version enabled when `f` does not change domain.
		 *
		 * Basically, if the return type of `f` is the same as its parameter type
		 * and `l` is a temporary (rvalue reference), then l is re-used. This
		 * means no copies are made.
		 */
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type,
				typename = typename std::enable_if<
					std::is_same<T,U>::value
				>::type
		>
		static std::list<T,A<T>> map(F&& f, std::list<T,A<T>>&& l) {
			for(auto& e : l) {
				e = f(e);
			}

			return std::list<T,A<T>>{std::move(l)};
		}

		/// Equivalent of flip(concatMap)
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type::value_type
		>
		static std::list<U,A<U>> bind(const std::list<T,A<T>>& l, F&& f) {
			return concatMap(std::forward<F>(f), l);
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type::value_type
		>
		static std::list<U,A<U>> bind(std::list<T,A<T>>&& l, F&& f) {
			return concatMap(std::forward<F>(f), std::move(l));
		}

		static constexpr bool instance = true;
	};

	/**
	 * Foldable instance for std::lists.
	 *
	 * \ingroup list
	 */
	template<
			typename T,
			typename A
	>
	struct foldable<std::list<T,A>>
	: fold_default<std::list<T,A>>, foldMap_default<std::list<T,A>> {

		template<
				typename Fn,
				typename U,
				typename = typename std::enable_if<
					std::is_same<
						U,
						typename decayed_result<Fn(U,T)>::type
					>::value
				>::type
		>
		static U foldl(Fn&& fn, U z, const std::list<T,A>& l) {
			for(auto& e : l) {
				z = fn(z, e);
			}

			return z;
		}

		template<
				typename Fn,
				typename U,
				typename = typename std::enable_if<
					std::is_same<
						U,
						typename decayed_result<Fn(T,U)>::type
					>::value
				>::type
		>
		static U foldr(Fn&& fn, U z, const std::list<T,A>& l) {
			for(auto it = l.rbegin(); it != l.rend(); ++it) {
				z = fn(*it, z);
			}

			return z;
		}

		static constexpr bool instance = true;
	};

}

#endif

