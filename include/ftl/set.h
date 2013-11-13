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
#ifndef FTL_SET_H
#define FTL_SET_H

#include <set>
#include "concepts/monad.h"
#include "concepts/foldable.h"

namespace ftl {

	/**
	 * \defgroup set Set
	 *
	 * Concept implementations for std::set.
	 *
	 * \code
	 *   #include <ftl/set.h>
	 * \endcode
	 *
	 * This module adds the following concept instances to `std::set`:
	 * - \ref monoidpg
	 * - \ref foldablepg
	 * - \ref functorpg
	 * - \ref applicativepg
	 * - \ref monadpg
	 *
	 * \par Dependencies
	 * - <set>
	 * - \ref monad
	 * - \ref foldable
	 */

	template<
			typename T,
			typename Cmp,
			typename A
	>
	struct parametric_type_traits<std::set<T,Cmp,A>> {
	private:
		template<typename U>
		using rebind_allocator
			= typename std::allocator_traits<A>::template rebind_alloc<U>;

	public:
		using value_type = T;

		template<typename U>
		using rebind = std::set<U,Rebind<Cmp,U>,rebind_allocator<U>>;
	};

	/**
	 * Implementation of the \ref monoidpg concept.
	 *
	 * Behvariour of the monoid operations are equivalent to:
	 * \code
	 *   id() == set{}
	 *   append(a, b) == set{a}.insert(b.begin(), b.end())
	 * \endcode
	 *
	 * In cases where e.g. one of the sets is a temporary, `append`
	 * might actually mutate one of the sets instead of making a copy.
	 *
	 * \ingroup set
	 */
	template<typename T, typename Cmp, typename A>
	struct monoid<std::set<T,Cmp,A>> {

		static std::set<T,Cmp,A> id()
		noexcept(std::is_nothrow_default_constructible<std::set<T,Cmp,A>>::value) {
			return std::set<T,Cmp,A>{};
		}

		static std::set<T,Cmp,A> append(
				const std::set<T,Cmp,A>& s1,
				const std::set<T,Cmp,A>& s2) {

			std::set<T,Cmp,A> rs{s1};
			rs.insert(s2.begin(), s2.end());

			return rs;
		}

		static std::set<T,Cmp,A> append(
				std::set<T,Cmp,A>&& s1,
				const std::set<T,Cmp,A>& s2) {

			s1.insert(s2.begin(), s2.end());
			return s1;
		}

		static std::set<T,Cmp,A> append(
				const std::set<T,Cmp,A>& s1,
				std::set<T,Cmp,A>&& s2) {

			s2.insert(s1.begin(), s1.end());
			return s2;
		}

		static std::set<T,Cmp,A> append(
				std::set<T,Cmp,A>&& s1,
				std::set<T,Cmp,A>&& s2) {

			if(s1.size() > s2.size()) {
				s1.insert(s2.begin(), s2.end());

				return s1;
			}

			else {
				s2.insert(s1.begin(), s1.end());

				return s2;
			}
		}

		static constexpr bool instance = true;
	};

	/**
	 * \ref monadpg implementation for std::set with parametrised comparator.
	 *
	 * Generally behaves as the other collections. The main difference being,
	 * of course, the data structure used.
	 *
	 * \ingroup set
	 */
	template<typename T, typename Cmp, typename A>
	struct monad<std::set<T,Cmp,A>>
	: deriving_bind<std::set<T,Cmp,A>>
	, deriving_apply<std::set<T,Cmp,A>> {

		/// Alias for cleaner type signatures
		template<typename U>
		using set = Rebind<std::set<T,Cmp,A>,U>;

		/**
		 * Embeds a single value in a set.
		 *
		 * In other words, produces a set with a single element of the value
		 * `t`.
		 */
		static set<T> pure(const T& t) {
			return set<T>{t};
		}

		/// \overload
		static set<T> pure(T&& t) {
			return set<T>{std::move(t)};
		}

		// What to do if/when f returns same U for different T?
		/**
		 * Maps a function to every element of the set. Note that the resulting
		 * set may contain fewer elements than the original, if several results
		 * of applying `f` happen to coincide.
		 *
		 * \tparam F must satisfy \ref fn`<U(T)>`, for some type `U` that is
		 *           strictly orderable by `Cmp<U>`.
		 *
		 * Example:
		 * \code
		 *   auto s = [](int x){ return x/2; } % std::set<int>{1,2,3,4,5,6};
		 *
		 *   // s == std::set<int>{0,1,2,3}
		 * \endcode
		 */
		template<typename F, typename U = result_of<F(T)>>
		static set<U> map(F&& f, const set<T>& s) {
			set<U> rs;
			for(const auto& e : s) {
				rs.insert(f(e));
			}

			return rs;
		}

		/// \overload
		template<typename F, typename U = result_of<F(T)>>
		static set<U> map(F&& f, set<T>&& s) {
			set<U> rs;
			for(auto& e : s) {
				rs.insert(f(std::move(e)));
			}

			return rs;
		}

		/**
		 * Flattens a set of sets.
		 *
		 * Preservation of relative order (from the perspective of depth-first
		 * iteration of the original set of sets) is not guaranteed, simply
		 * because the resulting set is (naturally) sorted.
		 *
		 * Example:
		 * \code
		 *   auto s = ftl::mjoin(std::set<std::set<int>>{{1,3},{2,3,4},{0,5}});
		 *
		 *   // s == std::set<int>{0,1,2,3,4}
		 * \endcode
		 */
		static set<T> join(const set<set<T>>& s) {
			 set<T> rs;
			 for(const auto& ss : s) {
				 for(const auto& e : ss) {
					 rs.insert(e);
				 }
			 }

			return rs;
		}

#ifdef DOCUMENTATION_GENERATOR
		/**
		 * \tparam F must satisfy \ref fn`<set<U>(T)>`, for some type `U` that
		 *           is comparable using `Cmp<U>`.
		 */
		template<
				typename F,
				typename U = typename result_of<F(T)>::value_type
		>
		static set<U> bind(const set<T>& s, F&& f);
#endif

		static constexpr bool instance = true;
	};

	/**
	 * Foldable instance for `std::set`.
	 *
	 * \ingroup set
	 */
	template<typename T, typename C, typename A>
	struct foldable<std::set<T,C,A>>
	: deriving_foldable<bidirectional_iterable<std::set<T,C,A>>> {};

}

#endif

