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
#ifndef FTL_VECTOR_H
#define FTL_VECTOR_H

#include <vector>
#include "foldable.h"
#include "monad.h"

namespace ftl {

	/**
	 * \defgroup vector Vector
	 *
	 * Implementations of concepts, utility functions, and more for std::vector.
	 *
	 * \code
	 *   #include <ftl/vector.h>
	 * \endcode
	 *
	 * This module adds the following concept instances to std::vector:
	 * - \ref monoid
	 * - \ref foldable
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 *
	 * \par Dependencies
	 * - <vector>
	 * - \ref foldable
	 * - \ref monad
	 */

	/**
	 * Specialisation of re_parametrise for vectors.
	 *
	 * This makes sure the allocator is also properly parametrised on the
	 * new element type.
	 *
	 * \ingroup list
	 */
	template<typename T, typename U, template<typename> class A>
	struct re_parametrise<std::vector<T,A<T>>,U> {
		using type = std::vector<U,A<U>>;
	};

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy \ref fn`<`\ref container`<B>(A)>`
	 *
	 * \ingroup vector
	 */
	template<
		typename F,
		typename T,
		template<typename> class A,
		typename U = typename decayed_result<F(T)>::type::value_type>
	std::vector<U,A<U>> concatMap(F f, const std::vector<T,A<T>>& v) {

		std::vector<U,A<U>> result;
		result.reserve(v.size() * 2);	// Reasonable assumption? TODO: test!
		auto nested = f % v;

		for(auto& el : nested) {
			for(auto& e : el) {
				result.push_back(e);
			}
		}

		return result;
	}

	/**
	 * Monoid implementation for std::vectors.
	 *
	 * Essentially equivalent of monoid<std::list<Ts...>>.
	 *
	 * \ingroup vector
	 */
	template<typename...Ts>
	struct monoid<std::vector<Ts...>> {
		static std::vector<Ts...> id() {
			return std::vector<Ts...>();
		}

		static std::vector<Ts...> append(
				const std::vector<Ts...>& v1,
				const std::vector<Ts...>& v2) {
			auto rv(v1);
			rv.reserve(v2.size());
			rv.insert(rv.end(), v2.begin(), v2.end());
			return rv;
		}

		static std::vector<Ts...> append(
				std::vector<Ts...>&& v1,
				const std::vector<Ts...>& v2) {
			v1.reserve(v2.size());
			v1.insert(v1.end(), v2.begin(), v2.end());
			return v1;
		}

		static std::vector<Ts...> append(
				const std::vector<Ts...>& v1,
				std::vector<Ts...>&& v2) {
			v2.reserve(v1.size());
			v2.insert(v2.begin(), v1.begin(), v1.end());
			return v1;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad implementation of vectors
	 *
	 * \ingroup vector
	 */
	template<typename T, template<typename> class A>
	struct monad<std::vector<T,A<T>>> {

		/// Creates a one element vector
		static std::vector<T,A<T>> pure(T&& t) {
			std::vector<T,A<T>> v{};
			v.emplace_back(std::forward<T>(t));
			return v;
		}

		/// Applies f to each element
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static std::vector<U,A<U>> map(F&& f, const std::vector<T,A<T>>& v) {
			std::vector<U,A<U>> ret;
			ret.reserve(v.size());
			for(const auto& e : v) {
				ret.push_back(f(e));
			}

			return ret;
		}

		/**
		 * Move optimised version enabled when `f` does not change domain.
		 *
		 * Basically, if the return type of `f` is the same as its parameter type
		 * and `v` is a temporary (rvalue reference), then `v` is re-used. This
		 * means no copies are made.
		 */
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type,
				typename =
					typename std::enable_if<std::is_same<T,U>::value>::type
		>
		static std::vector<T,A<T>> map(F&& f, std::vector<T,A<T>>&& v) {
			for(auto& e : v) {
				e = f(e);
			}

			// Move into new (temporary) container, so reference does not go
			// and die
			return std::vector<T,A<T>>(std::move(v));
		}

		/// Equivalent of flip(concatMap)
		template<
			typename F,
			typename U = typename decayed_result<F(T)>::type::value_type
		>
		static std::vector<U,A<U>> bind(const std::vector<T,A<T>>& v, F&& f) {
			return concatMap(std::forward<F>(f), v);
		}

		static constexpr bool instance = true;
	};

	/**
	 * Foldable instance for std::vector.
	 *
	 * \ingroup vector
	 */
	template<typename T, typename A>
	struct foldable<std::vector<T,A>>
	: fold_default<std::vector<T,A>>, foldMap_default<std::vector<T,A>> {

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
		static U foldl(Fn&& fn, U z, const std::vector<T,A>& v) {
			for(auto& e : v) {
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
		static U foldr(Fn&& fn, U z, const std::vector<T,A>& v) {
			for(auto it = v.rbegin(); it != v.rend(); ++it) {
				z = fn(*it, z);
			}

			return z;
		}

		static constexpr bool instance = true;
	};

}

#endif

