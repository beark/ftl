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
#include "concepts/foldable.h"
#include "concepts/monad.h"
#include "concepts/zippable.h"

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
	 * - \ref monoidpg
	 * - \ref foldablepg
	 * - \ref functorpg
	 * - \ref applicativepg
	 * - \ref monadpg
	 * - \ref zippablepg
	 *
	 * \par Dependencies
	 * - <forward_list>
	 * - \ref foldable
	 * - \ref monad
	 * - \ref zippable
	 */

	template<typename T, typename A>
	struct parametric_type_traits<std::forward_list<T,A>> {
	private:
		template<typename U>
		using rebind_allocator
			= typename std::allocator_traits<A>::template rebind_alloc<U>;

	public:
		using value_type = T;

		template<typename U>
		using rebind = std::forward_list<U,rebind_allocator<U>>;
	};

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy \ref fn`<`\ref fwditerable`<U>(T)>`
	 *
	 * \ingroup fwdlist
	 */
	template<
			typename F,
			typename T,
			typename A,
			typename U = Value_type<result_of<F(T)>>,
			typename Au = Rebind<A,U>
	>
	std::forward_list<U,Au> concatMap(
			F&& f,
			const std::forward_list<T,A>& l) {

		std::forward_list<U,Au> result;
		auto nested = std::forward<F>(f) % l;

		auto it = result.before_begin();
		for(auto& el : nested) {
			it = result.insert_after(
					it,
					std::make_move_iterator(el.begin()),
					std::make_move_iterator(el.end())
			);
		}

		return result;
	}

	/**
	 * \overload
	 *
	 * \ingroup fwdlist
	 */
	template<
			typename F,
			typename T,
			typename A,
			typename U = Value_type<result_of<F(T)>>,
			typename Au = Rebind<A,U>
	>
	std::forward_list<U,Au> concatMap(
			F&& f,
			std::forward_list<T,A>&& l) {

		auto nested = std::forward<F>(f) % std::move(l);

		std::forward_list<U,Au> result;

		auto it = result.before_begin();
		for(auto& el : nested) {
			it = result.insert_after(
					it,
					std::make_move_iterator(el.begin()),
					std::make_move_iterator(el.end())
			);
		}

		return result;
	}

	/**
	 * Monoid implementation for `std::forward_list`
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

			rl.insert_after(rl.before_begin(), l1.begin(), l1.end());
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

			std::forward_list<Ts...> l(l1);

			l2.splice_after(l2.before_begin(), std::move(l));

			return l2;
		}

		static std::forward_list<Ts...> append(
				std::forward_list<Ts...>&& l1,
				std::forward_list<Ts...>&& l2) {

			l2.splice_after(l2.before_begin(), std::move(l1));

			return l2;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad instance of forward_lists
	 *
	 * This instance is equivalent to the other container monads, e.g.
	 * `monad<std::list<T>>`, and `monad<std::vector<T>>`.
	 *
	 * \ingroup fwdlist
	 */
	template<typename T, typename A>
	struct monad<std::forward_list<T,A>>
	: deriving_pure<std::forward_list<T,A>>
	, deriving_join<in_terms_of_bind<std::forward_list<T,A>>>
	, deriving_apply<in_terms_of_bind<std::forward_list<T,A>>> {

		/// Alias to make type signatures more easily read.
		template<typename U>
		using forward_list = Rebind<std::forward_list<T,A>,U>;

#ifdef DOCUMENTATION_GENERATOR
		/**
		 * Embed a `T` in a forward list.
		 *
		 * Simply creates a singleton list, containing `t`.
		 */
		static forward_list<T> pure(const T& t);


		/// \overload
		static forward_list<T> pure(T&& t);
#endif

		/**
		 * Maps the given function over all elements in the list.
		 *
		 * Similar to `std::transform`, except `l` is not mutated and `f` is
		 * allowed to change domain.
		 */
		template<typename F, typename U = result_of<F(T)>>
		static forward_list<U> map(F&& f, const forward_list<T>& l) {

			forward_list<U> rl;
			auto it = rl.before_begin();
			for(const auto& e : l) {
				it = rl.insert_after(it, f(e));
			}

			return rl;
		}

		/// \overload
		template<
				typename F,
				typename U = result_of<F(T)>,
				typename = Requires<!std::is_same<T,U>::value>

		>
		static forward_list<U> map(F&& f, forward_list<T>&& l) {

			forward_list<U> rl;
			auto it = rl.before_begin();
			for(auto& e : l) {
				it = rl.insert_after(it, f(std::move(e)));
			}

			return rl;
		}

		/**
		 * Mutating, no-copy optimised version.
		 *
		 * Kicks in if `f` does not change domain and `l` is a temporary.
		 */
		template<
				typename F,
				typename = Requires<std::is_same<T, result_of<F(T)>>::value>
		>
		static forward_list<T> map(F&& f, forward_list<T>&& l) {

			auto rl = std::move(l);
			for(auto& e : rl) {
				e = f(e);
			}

			return rl;
		}

		/**
		 * Monadic bind operation.
		 *
		 * Can be viewed as a non-deterministic computation: `l` is a list of
		 * possible values, each of which we apply `f` to. As `f` itself is also
		 * non-deterministic, it may return several possible answers for each
		 * element in `l`. Finally, all of the results are collected in a flat
		 * list.
		 *
		 * \note `f` is allowed to return _any_ \ref fwditerable, not only
		 *       `forward_list`. The final result, however, is always a
		 *       `forward_list`.
		 *
		 * Example:
		 * \code
		 *   auto l =
		 *       forward_list<int>{1, 2, 3}
		 *       >>= [](int x){ return list<int>{x-1, x, x+1}; };
		 *
		 *   // Note how each group of three corresponds to one input element
		 *   // l == forward_list<int>{0,1,2, 1,2,3, 2,3,4}
		 * \endcode
		 */
		template<typename F, typename U = typename result_of<F(T)>::value_type>
		static forward_list<U> bind(const forward_list<T>& l, F&& f) {

			return concatMap(std::forward<F>(f), l);
		}

		/// \overload
		template<typename F, typename U = typename result_of<F(T)>::value_type>
		static forward_list<U> bind(forward_list<T>&& l, F&& f) {

			return concatMap(std::forward<F>(f), std::move(l));
		}

		static constexpr bool instance = true;

	};

	namespace _dtl {
		template<typename F, typename Z, typename It>
		Z fwdfoldr(F&& f, Z&& z, It it, It end) {
			if(it != end) {
				auto& x = *it;
				return f(
					x,
					fwdfoldr(
						std::forward<F>(f),
						std::forward<Z>(z),
						++it,
						end
					)
				);
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
			deriving_foldl<std::forward_list<T,A>>,
			deriving_fold<std::forward_list<T,A>>,
			deriving_foldMap<std::forward_list<T,A>> {

		template<
				typename F,
				typename U,
				typename = Requires<std::is_same<U, result_of<F(T,U)>>::value>
		>
		static U foldr(F&& f, U&& z, const std::forward_list<T,A>& l) {
			return _dtl::fwdfoldr(
					std::forward<F>(f),
					std::forward<U>(z),
					l.cbegin(), l.cend());
		}

		static constexpr bool instance = true;
	};

	/**
	 * \ref zippablepg implementation for `std::forward_list`.
	 *
	 * This particular instance allows a `forward_list` to be zipped with
	 * any type that satisfies \ref fwditerable. Thus, one can zip a
	 * `forward_list` with a `vector`, `list` or even `maybe`.
	 *
	 * \ingroup fwdlist
	 */
	template<typename T, typename A>
	struct zippable<std::forward_list<T,A>> {

		template<typename U>
		using A_ = Rebind<A,U>;

		template<
				typename F,
				typename FwdIt,
				typename U = Value_type<FwdIt>,
				typename V = result_of<F(T,U)>,
				typename = Requires<ForwardIterable<FwdIt>()>
		>
		static std::forward_list<V,A_<V>> zipWith(
				F f, const std::forward_list<T,A>& l, const FwdIt& it
		) {
			std::forward_list<V,A_<V>> result;

			auto insert_it = result.before_begin();
			auto it1 = l.begin();
			auto it2 = it.begin();
			while(it1 != l.end() && it2 != std::end(it)) {
				insert_it = result.insert_after(insert_it, f(*it1, *it2));

				++it1;
				++it2;
			}

			return result;
		}

		static constexpr bool instance = true;
	};

}

#endif

