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
#ifndef FTL_FORWARDS_LIST_H
#define FTL_FORWARDS_LIST_H

#include <forward_list>
#include "foldable.h"
#include "monad.h"

namespace ftl {

	/**
	 * \defgroup fwdlist Forward List
	 *
	 * Singly linked list, its concept implementations, etc.
	 *
	 * \code
	 *   #include <ftl/forward_list.h>
	 * \endcode
	 *
	 * This module adds the following concept instances to std::forward_list:
	 * - \ref monoid
	 * - \ref foldable
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 *
	 * \par Dependencies
	 * - <forward_list>
	 * - \ref foldable
	 * - \ref monad
	 */

	/**
	 * Specialisation of re_parametrise for forward_lists.
	 *
	 * This makes sure the allocator is also properly parametrised on the
	 * new element type.
	 *
	 * \ingroup list
	 */
	template<typename T, typename U, template<typename> class A>
	struct re_parametrise<std::forward_list<T,A<T>>,U> {
		using type = std::forward_list<U,A<U>>;
	};

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy \ref fn`<`\ref container`<B>(A)>`
	 *
	 * \ingroup fwdlist
	 */
	template<
			typename F,
			typename T,
			template<typename> class A,
			typename U = typename decayed_result<F(T)>::type::value_type>
	std::forward_list<U,A<U>> concatMap(
			F&& f,
			const std::forward_list<T,A<T>>& l) {

		std::forward_list<U,A<U>> result;
		auto nested = f % l;

		auto it = result.before_begin();
		for(auto& el : nested) {
			it = result.insert_after(it, el.begin(), el.end());
		}

		return result;
	}

	/**
	 * Monoid implementation for std::forward_list
	 *
	 * Identity element is the empty list, monoid operation is list
	 * concatenation.
	 * 
	 * \ingroup fwdlist
	 */
	template<typename...Ts>
	struct monoid<std::forward_list<Ts...>> {
		static std::forward_list<Ts...> id() {
			return std::forward_list<Ts...>();
		}

		static std::forward_list<Ts...> append(
				const std::forward_list<Ts...>& l1,
				const std::forward_list<Ts...>& l2) {

			std::forward_list<Ts...> rl(l2);

			rl.insert_after(rl.before_begin(), l2.begin(), l2.end());
			return rl;
		}

		// Optimised cases for when one list can be spliced into the other
		static std::forward_list<Ts...> append(
				std::forward_list<Ts...>&& l1,
				const std::forward_list<Ts...>& l2) {

			std::forward_list<Ts...> rl(l2);
			rl.splice_after(rl.before_begin(), std::move(l1));
			return rl;
		}

		static std::forward_list<Ts...> append(
				const std::forward_list<Ts...>& l1,
				std::forward_list<Ts...>&& l2) {

			std::forward_list<Ts...> rl(l1);

			auto it = rl.begin();
			for(; it != rl.end(); ++it)
				;

			rl.splice_after(it, std::move(l2));
			return rl;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad instance of forward_lists
	 *
	 * This instance is equivalent to the other container monads, e.g.
	 * monad<std::list<T>>, and monad<std::vector<T>>.
	 *
	 * \ingroup fwdlist
	 */
	template<typename T, template<typename> class A>
	struct monad<std::forward_list<T,A<T>>> {

		static std::forward_list<T,A<T>> pure(T&& t) {
			std::forward_list<T> l;
			l.emplace_front(std::forward<T>(t));
			return l;
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static std::forward_list<U,A<U>> map(
				F&& f,
				const std::forward_list<T,A<T>>& l) {

			std::forward_list<U,A<U>> rl;
			auto it = rl.before_begin();
			for(const auto& e : l) {
				it = rl.insert_after(it, f(e));
			}

			return rl;
		}

		// No copy version available when F is an endofuntion
		template<
				typename F,
				typename = typename std::enable_if<
					std::is_same<
						T,
						typename decayed_result<F(T)>::type
					>::value
				>::type>
		static std::forward_list<T,A<T>> map(
				F&& f,
				std::forward_list<T,A<T>>&& l) {

			auto rl = std::move(l);
			for(auto& e : rl) {
				e = f(e);
			}

			return rl;
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type::value_type
		>
		static std::forward_list<U,A<U>> bind(
				const std::forward_list<T,A<T>> l,
				F&& f) {

			return concatMap(std::forward<F>(f), l);
		}

		static constexpr bool instance = true;

	};

	namespace _dtl {
		template<typename F, typename Z, typename It>
		Z fwdfoldr(F&& f, Z&& z, It it, It end) {
			if(it != end) {
				return f(*it, fwdfoldr(f, std::forward<Z>(z), ++it, end));
			}

			return z;
		}
	}

	/**
	 * Instance implementation of Foldable for std::forward_lists.
	 *
	 * \ingroup fwdlist
	 */
	template<typename T, typename A>
	struct foldable<std::forward_list<T,A>> :
			fold_default<std::forward_list<T,A>>,
			foldMap_default<std::forward_list<T,A>> {

		template<
				typename F,
				typename U,
				typename = typename std::enable_if<
					std::is_same<
						U,
						typename decayed_result<F(U,T)>::type
					>::value
				>::type
		>
		static U foldl(F&& f, U z, const std::forward_list<T,A>& l) {
			for(auto& e : l) {
				z = f(z, e);
			}

			return z;
		}

		template<
				typename F,
				typename U,
				typename = typename std::enable_if<
					std::is_same<
						U,
						typename decayed_result<F(T,U)>::type
					>::value
				>::type
		>
		static U foldr(F&& f, U&& z, const std::forward_list<T,A>& l) {
			return _dtl::fwdfoldr(
					std::forward<F>(f),
					std::forward<U>(z),
					l.begin(), l.end());
		}
	};

}

#endif

