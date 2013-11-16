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
#ifndef FTL_MAP_H
#define FTL_MAP_H

#include <map>
#include "concepts/functor.h"
#include "concepts/foldable.h"

namespace ftl {

	/**
	 * \defgroup map Map
	 *
	 * Concept implementations for std::map.
	 *
	 * Adds the \ref foldablepg, \ref monoidpg, and \ref functorpg concept
	 * instances.
	 *
	 * \code
	 *   #include <ftl/map.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref functor
	 * - \ref foldable
	 */

	template<typename K, typename V, typename C, typename A>
	struct parametric_type_traits<std::map<K,V,C,A>> {
	private:
		template<typename U>
		using rebind_allocator
			= typename std::allocator_traits<A>::template rebind_alloc<U>;

	public:
		using value_type = V;

		template<typename W>
		using rebind = std::map<K,W,C,rebind_allocator<std::pair<const K,W>>>;
	};

	/**
	 * Functor instance for std::map.
	 *
	 * \ingroup map
	 */
	template<typename K, typename T, typename C, typename A>
	struct functor<std::map<K,T,C,A>> {

		/// Type alias for more easily read type signatures.
		template<typename U>
		using Map = Rebind<std::map<K,T,C,A>,U>;

		/**
		 * Maps the function `f` over all values in `m`.
		 */
		template<typename F, typename U = result_of<F(T)>>
		static Map<U> map(F&& f, const Map<T>& m) {
			Map<U> rm;
			for(const auto& kv : m) {
				rm.insert(std::make_pair(kv.first, f(kv.second)));
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
				typename = Requires<!std::is_same<T,U>::value>
		>
		static Map<U> map(F&& f, Map<T>&& m) {
			Map<U> rm;
			for(auto& kv : m) {
				rm.emplace(std::move(kv.first), f(std::move(kv.second)));
			}

			return rm;
		}

		/**
		 * No-copy overload for endofunctions on temporary maps.
		 *
		 * \note Requires a \ref moveassignable `T`.
		 */
		template<
				typename F,
				typename = Requires<
					std::is_same<T,result_of<F(T)>>::value
				>
		>
		static Map<T> map(F&& f, Map<T>&& m) {
			for(auto& kv : m) {
				m[kv.first] = f(std::move(kv.second));
			}

			return m;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Implementation of Foldable for std::map.
	 *
	 * \ingroup map
	 */
	template<typename K, typename T, typename C, typename A>
	struct foldable<std::map<K,T,C,A>>
	: deriving_fold<std::map<K,T,C,A>>, deriving_foldMap<std::map<K,T,C,A>> {

		template<
				typename F,
				typename U,
				typename = Requires<
					std::is_same<U, result_of<F(U,T)>>::value
				>
		>
		static U foldl(F&& f, U z, const std::map<K,T,C,A>& m) {
			for(auto& kv : m) {
				z = f(z, kv.second);
			}

			return z;
		}

		template<
				typename F,
				typename U,
				typename = Requires<
					std::is_same<U, result_of<F(T,U)>>::value
				>
		>
		static U foldr(F&& f, U z, const std::map<K,T,C,A>& m) {
			for(auto it = m.rbegin(); it != m.rend(); ++it) {
				z = f(it->second, z);
			}

			return z;
		}

		static constexpr bool instance = true;
	};

}

#endif


