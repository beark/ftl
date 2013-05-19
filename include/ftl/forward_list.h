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
#include <tuple>
#include "foldable.h"
#include "monad.h"
#include "maybe.h"

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
	 *
	 * \par Dependencies
	 * - <forward_list>
	 * - <tuple>
	 * - \ref foldable
	 * - \ref monad
	 * - \ref maybe
	 */

	/**
	 * Workaround to make forward lists compatible with the functor series of
	 * concepts.
	 *
	 * If you require a different allocator than \c std::allocator, then make
	 * a similar type synonym and give it a monad instance of its own.
	 *
	 * \par Concepts
	 * As a regular std::forward_list, with the additions:
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 *
	 * \ingroup fwdlist
	 */
	template<typename T>
	using forward_list = std::forward_list<T, std::allocator<T>>;

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy \ref fn`<`\ref container`<B>(A)>`
	 *
	 * \ingroup fwdlist
	 */
	template<
		typename F,
		typename A,
		typename B = typename decayed_result<F(A)>::type::value_type>
	forward_list<B> concatMap(F&& f, const forward_list<A>& l) {

		forward_list<B> result;
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

			for(auto it = rl.begin(); it != rl.end(); ++it)
				;

			rl.splice_after(it, std::move(l2));
			return rl;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monoid implementation for ftl::forward_list
	 *
	 * Exactly equivalent of the monoid instance for std::forward_list.
	 * 
	 * \ingroup fwdlist
	 */
	template<typename T>
	struct monoid<forward_list<T>> {
		static forward_list<T> id() {
			return forward_list<T>();
		}

		static forward_list<T> append(
				const forward_list<T>& l1,
				const forward_list<T>& l2) {

			forward_list<T> rl(l2);

			rl.insert_after(rl.before_begin(), l2.begin(), l2.end());
			return rl;
		}

		// Optimised cases for when one list can be spliced into the other
		static forward_list<T> append(
				forward_list<T>&& l1,
				const forward_list<T>& l2) {

			forward_list<T> rl(l2);
			rl.splice_after(rl.before_begin(), std::move(l1));
			return rl;
		}

		static forward_list<T> append(
				const forward_list<T>& l1,
				forward_list<T>&& l2) {

			forward_list<T> rl(l1);

			for(auto it = rl.begin(); it != rl.end(); ++it)
				;

			rl.splice_after(it, std::move(l2));
			return rl;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad instance of FTL's alias of forward_list
	 *
	 * This instance is equivalent to the other container monads, e.g.
	 * monad<ftl::list>, and monad<ftl::vector>.
	 *
	 * \ingroup fwdlist
	 */
	template<>
	struct monad<forward_list> {
		template<typename T>
		static forward_list<T> pure(T&& t) {
			forward_list<T> l;
			l.emplace_front(std::forward<T>(t));
			return l;
		}

		template<
				typename F,
				typename A,
				typename B = typename decayed_result<F(A)>::type>
		static forward_list<B> map(F&& f, const forward_list<A>& l) {
			forward_list<B> rl;
			auto it = rl.before_begin();
			for(const auto& e : l) {
				it = rl.insert_after(it, f(e));
			}

			return rl;
		}

		// No copy version available when F is an endofuntion
		template<
				typename F,
				typename A,
				typename = typename std::enable_if<
					std::is_same<
						A,
						typename decayed_result<F(A)>::type
					>::value
				>::type>
		static forward_list<A> map(F&& f, forward_list<A>&& l) {
			for(auto& e : l) {
				e = f(e);
			}

			return std::move(l);
		}

		template<
				typename F,
				typename A,
				typename B = typename decayed_result<F(A)>::type::value_type>
		static forward_list<B> bind(const forward_list<B> l, F&& f) {
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
	template<>
	struct foldable<std::forward_list>
	: fold_default<std::forward_list>, foldMap_default<std::forward_list> {
		template<
				typename F,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						A,
						typename decayed_result<F(B,A)>::type
					>::value
				>::type,
				typename...Ts>
		static A foldl(F&& f, A z, const std::forward_list<B,Ts...>& l) {
			for(auto& e : l) {
				z = f(z, e);
			}

			return z;
		}

		template<
				typename F,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						B,
						typename decayed_result<F(A,B)>::type
					>::value
				>::type,
				typename...Ts>
		static B foldr(F&& f, B&& z, const std::forward_list<A,Ts...>& l) {
			return _dtl::fwdfoldr(
					std::forward<F>(f),
					std::forward<B>(z),
					l.begin(), l.end());
		}
	};

	/**
	 * Instance implementation of Foldable for ftl::forward_lists.
	 *
	 * \ingroup fwdlist
	 */
	template<>
	struct foldable<forward_list>
	: fold_default<forward_list>, foldMap_default<forward_list> {
		template<
				typename F,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						A,
						typename decayed_result<F(B,A)>::type
					>::value
				>::type>
		static A foldl(F&& f, A z, const forward_list<B>& l) {
			for(auto& e : l) {
				z = f(z, e);
			}

			return z;
		}

		template<
				typename F,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						B,
						typename decayed_result<F(A,B)>::type
					>::value
				>::type>
		static B foldr(F&& f, B&& z, const forward_list<A>& l) {
			return _dtl::fwdfoldr(
					std::forward<F>(f),
					std::forward<B>(z),
					l.begin(), l.end());
		}
	};

}

#endif

