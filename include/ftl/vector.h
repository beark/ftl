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
#include "concepts/zippable.h"

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
	 * - \ref zippable
	 *
	 * \par Dependencies
	 * - <vector>
	 * - \ref foldable
	 * - \ref monad
	 */

	template<typename T, typename A>
	struct parametric_type_traits<std::vector<T,A>> {
	private:
		template<typename U>
		using rebind_allocator
			= typename std::allocator_traits<A>::template rebind_alloc<U>;

	public:
		using value_type = T;

		template<typename U>
		using rebind = std::vector<U,rebind_allocator<U>>;
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
			typename Au = Rebind<A,U>
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
			typename Au = Rebind<A,U>
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
	: deriving_monad<back_insertable_container<std::vector<T,A>>> {

#ifdef DOCUMENTATION_GENERATOR
		/// Alias to make type signatures cleaner
		template<typename U>
		using vector = Rebind<std::vector<T,A>,U>;

		/// Creates a one element vector
		static vector<T> pure(const T& t);

		/// Applies `f` to each element
		template<typename F, typename U = result_of<F(T)>>
		static vector<U> map(F&& f, const vector<T>& v);

		/// Rvalue version of map
		template<typename F, typename U = result_of<F(T)>>
		static vector<U> map(F&& f, vector<T>&& v);

		/**
		 * Joins nested vectors by way of concatenation.
		 *
		 * The resulting vector contains every element of every vector contained
		 * in the original vector. Relative order is preserved (from the
		 * perspective of depth first iteration).
		 */
		static vector<T> join(const vector<vector<T>>& v);

		/// \overload
		static vector<T> join(vector<vector<T>>&& v);

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
				typename U = Value_type<Cu>,
				typename = Requires<ForwardIterable<Cu>()>
		>
		static vector<U> bind(const vector<T>& v, F&& f);

		/// \overload
		template<
				typename F,
				typename Cu = result_of<F(T)>,
				typename U = Value_type<Cu>,
				typename = Requires<ForwardIterable<Cu>()>
		>
		static vector<U> bind(vector<T>&& v, F&& f);
#endif
	};

	/**
	 * Foldable instance for std::vector.
	 *
	 * \ingroup vector
	 */
	template<typename T, typename A>
	struct foldable<std::vector<T,A>>
	: deriving_foldable<bidirectional_iterable<std::vector<T,A>>> {};

	/**
	 * Zippable instance for std::vector.
	 *
	 * This particular instance allows a `vector` to be zipped with
	 * any type that satisfies \ref fwditerable. Thus, one can zip a
	 * `vector` with a `forward_list`, `list` or even `ftl::maybe`.
	 *
	 * \ingroup vector
	 */
	template<typename T, typename A>
	struct zippable<std::vector<T,A>>
	: deriving_zippable<back_insertable_container<std::vector<T,A>>> {};

}

#endif

