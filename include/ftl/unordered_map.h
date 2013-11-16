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
#ifndef FTL_UNORDERED_MAP_H
#define FTL_UNORDERED_MAP_H

#include <unordered_map>
#include "concepts/functor.h"

namespace ftl {

	/**
	 * \defgroup unord_map Unordered Map
	 *
	 * Concept implementations for std::unordered_map.
	 *
	 * Adds the \ref functorpg concept instance.
	 *
	 * \code
	 *   #include <ftl/unordered_map.h>
	 * \endcode
	 *
	 * While it's more or less possible to give `unordered_map` a
	 * \ref foldablepg instance or a \ref monoidpg instance, none are provided.
	 * This is because both would lead to unspecified results in certain cases.
	 *
	 * For instance, for the Foldable instance, `ftl::foldl` and `ftl::foldr`
	 * would have unspecified results for non-associative folding functions.
	 * I.e., given two maps containing the exact same key/value-pairs, a fold
	 * might give two different results because their bucket count might not be
	 * the same, or they use different hash functions.
	 *
	 * \par Dependencies
	 * - \ref functor
	 */

	template<typename K, typename V, typename H, typename C, typename A>
	struct parametric_type_traits<std::unordered_map<K,V,H,C,A>> {
	private:
		template<typename U>
		using rebind_allocator
			= typename std::allocator_traits<A>::template rebind_alloc<U>;

	public:
		using value_type = V;

		template<typename W>
		using rebind =
			std::unordered_map<K,W,H,C,rebind_allocator<std::pair<const K,W>>>;
	};

	/**
	 * Functor instance for std::unordered_map.
	 *
	 * \ingroup unord_map
	 */
	template<typename K, typename T, typename H, typename C, typename A>
	struct functor<std::unordered_map<K,T,H,C,A>> {

		/// Type alias for more easily read type signatures.
		template<typename U>
		using unordered_map = Rebind<std::unordered_map<K,T,H,C,A>,U>;

		/**
		 * Maps the function `f` over all values in `m`.
		 */
		template<typename F, typename U = result_of<F(T)>>
		static unordered_map<U> map(F&& f, const unordered_map<T>& m) {
			unordered_map<U> rm;
			for(const auto& kv : m) {
				rm.emplace(kv.first, f(kv.second));
			}

			return rm;
		}

		/**
		 * R-value overload.
		 *
		 * Moves keys and values from `m`.
		 */
		template<
				typename F,
				typename U = result_of<F(T)>,
				typename = Requires<
					!std::is_same<T,U>::value
				>
		>
		static unordered_map<U> map(F&& f, unordered_map<T>&& m) {
			unordered_map<U> rm;
			for(auto& kv : m) {
				rm.emplace(std::move(kv.first), f(std::move(kv.second)));
			}

			return rm;
		}

		/**
		 * No-copy overload for endofunctions on temporary maps.
		 *
		 * \note Requires a `T` that satisfies \ref moveassignable.
		 */
		template<
				typename F,
				typename = Requires<
					std::is_same<T,result_of<F(T)>>::value
				>
		>
		static unordered_map<T> map(F&& f, unordered_map<T>&& m) {
			for(auto& kv : m) {
				m[kv.first] = f(std::move(kv.second));
			}

			return m;
		}

		static constexpr bool instance = true;
	};

}

#endif



