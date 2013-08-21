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
#include "concepts/foldable.h"
#include "concepts/monad.h"

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
	template<typename T, typename U, typename A>
	struct re_parametrise<std::vector<T,A>,U> {
	private:
		using Au = typename re_parametrise<A,U>::type;

	public:
		using type = std::vector<U,Au>;
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
			typename A,
			typename U = typename result_of<F(T)>::value_type,
			typename Au = typename re_parametrise<A,U>::type
	>
	std::vector<U,Au> concatMap(F f, const std::vector<T,A>& v) {

		std::vector<U,Au> result;
		result.reserve(v.size() * 2);
		auto nested = f % v;

		for(auto& el : nested) {
			result.insert(
					result.end(),
					std::make_move_iterator(el.begin()),
					std::make_move_iterator(el.end())
			);
		}

		return result;
	}

	/**
	 * \overload
	 *
	 * \ingroup vector
	 */
	template<
			typename F,
			typename T,
			typename A,
			typename U = typename result_of<F(T)>::value_type,
			typename Au = typename re_parametrise<A,U>::type
	>
	std::vector<U,Au> concatMap(F f, std::vector<T,A>&& v) {

		auto nested = f % std::move(v);

		std::vector<U,Au> result;
		result.reserve(nested.size() * 2);

		for(auto& el : nested) {
			result.insert(
					result.end(),
					std::make_move_iterator(el.begin()),
					std::make_move_iterator(el.end())
			);
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
			rv.reserve(v1.size() + v2.size());
			rv.insert(rv.end(), v2.begin(), v2.end());
			return rv;
		}

		static std::vector<Ts...> append(
				std::vector<Ts...>&& v1,
				const std::vector<Ts...>& v2) {
			v1.reserve(v1.size() + v2.size());
			v1.insert(v1.end(), v2.begin(), v2.end());
			return v1;
		}

		static std::vector<Ts...> append(
				const std::vector<Ts...>& v1,
				std::vector<Ts...>&& v2) {
			v2.reserve(v2.size() + v1.size());
			v2.insert(v2.begin(), v1.begin(), v1.end());
			return v2;
		}

		static std::vector<Ts...> append(
				std::vector<Ts...>&& v1,
				std::vector<Ts...>&& v2) {
			v1.reserve(v1.size() + v2.size());
			std::move(v2.begin(), v2.end(), std::back_inserter(v1));
			return v1;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad implementation of vectors
	 *
	 * \ingroup vector
	 */
	template<typename T, typename A>
	struct monad<std::vector<T,A>>
	: deriving_bind<back_insertable_container<std::vector<T,A>>>
	, deriving_apply<std::vector<T,A>> {

		/// Alias to make type signatures cleaner
		template<typename U>
		using vector = typename re_parametrise<std::vector<T,A>,U>::type;

		/// Creates a one element vector
		static vector<T> pure(const T& t) {
			vector<T> v{};
			v.push_back(t);
			return v;
		}

		static vector<T> pure(T&& t) {
			vector<T> v{};
			v.emplace_back(std::move(t));
			return v;
		}

		/// Applies `f` to each element
		template<typename F, typename U = result_of<F(T)>>
		static vector<U> map(F&& f, const vector<T>& v) {
			vector<U> ret;
			ret.reserve(v.size());
			for(const auto& e : v) {
				ret.push_back(f(e));
			}

			return ret;
		}

		/// Rvalue version of map
		template<
				typename F,
				typename U = result_of<F(T)>,
				typename =
					typename std::enable_if<!std::is_same<T,U>::value>::type
		>
		static vector<U> map(F&& f, vector<T>&& v) {
			vector<U> ret;
			ret.reserve(v.size());
			for(auto& e : v) {
				ret.push_back(f(std::move(e)));
			}

			return ret;
		}

		/**
		 * Move optimised version enabled when `f` does not change domain.
		 *
		 * Basically, if the return type of `f` is the same as its parameter
		 * type and `v` is a temporary (rvalue reference), then `v` is re-used.
		 * This means no copies are made.
		 */
		template<
				typename F,
				typename U = result_of<F(T)>,
				typename =
					typename std::enable_if<std::is_same<T,U>::value>::type
		>
		static vector<T> map(F&& f, vector<T>&& v) {
			for(auto& e : v) {
				e = f(e);
			}

			return v;
		}

		/**
		 * Joins nested vectors by way of concatenation.
		 *
		 * The resulting vector contains every element of every vector contained
		 * in the original vector. Relative order is preserved (from the
		 * perspective of depth first iteration).
		 */
		static vector<T> join(const vector<vector<T>>& v) {
			vector<T> rv;
			for(const auto& vv : v) {
				for(const auto& e : vv) {
					rv.push_back(e);
				}
			}

			return rv;
		}

		/// \overload
		static vector<T> join(vector<vector<T>>&& v) {
			vector<T> rv(2*v.size());
			for(auto& vv : v) {
				for(auto& e : vv) {
					rv.emplace_back(std::move(e));
				}
			}

			return rv;
		}

#ifdef DOCUMENTATION_GENERATOR
		/**
		 * Can be viewed as a non-deterministic computation: `v` is a vector of
		 * possible values, each of which we apply `f` to. As `f` itself is also
		 * non-deterministic, it may return several possible answers for each
		 * element in `v`. Finally, all of the results are collected in a flat
		 * vector.
		 *
		 * \note `f` is allowed to return _any_ \ref fwditerable, not only
		 *       vectors. The final result, however, is always a vector.
		 *
		 * Example:
		 * \code
		 *   auto v =
		 *       vector<int>{1, 2, 3}
		 *       >>= [](int x){ return vector<int>{x-1, x, x+1}; };
		 *
		 *   // Note how each group of three corresponds to one input element
		 *   // v == vector<int>{0,1,2, 1,2,3, 2,3,4}
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
		static vector<U> bind(const vector<T>& v, F&& f);

		/// \overload
		template<
				typename F,
				typename Cu = result_of<F(T)>,
				typename U = concept_parameter<Cu>,
				typename = typename std::enable_if<
					ForwardIterable<Cu>()
				>::type
		>
		static vector<U> bind(vector<T>&& v, F&& f);
#endif

		static constexpr bool instance = true;
	};

	/**
	 * Foldable instance for std::vector.
	 *
	 * \ingroup vector
	 */
	template<typename T, typename A>
	struct foldable<std::vector<T,A>>
	: deriving_foldl<std::vector<T,A>>
	, deriving_fold<std::vector<T,A>>, deriving_foldMap<std::vector<T,A>> {

		template<
				typename Fn,
				typename U,
				typename = typename std::enable_if<
					std::is_same<U, result_of<Fn(T,U)>>::value
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

