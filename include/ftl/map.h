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

	/**
	 * Re parameterisation specialisation of std::map.
	 *
	 * Applies the parametrisation recursively to the compare function and
	 * allocator types, meaning that if either of the two requires special
	 * consideration, a user can simply specialise `re_parametrise` in an
	 * appropriate manner.
	 *
	 * \note When parameterising the allocator `A`, it's given
	 *       `std::pair<const K,U>` as type parameter. If this is not the
	 *       desired behaviour, it is possible to write a template
	 *       specialisation that catches this and extracts `U`.
	 *
	 * \see ftl::re_parametrise.
	 *
	 * \ingroup map
	 */
	template<typename K, typename T, typename C, typename A, typename U>
	struct re_parametrise<std::map<K,T,C,A>,U> {
	private:
		using Cu = typename re_parametrise<C,U>::type;
		using Au = typename re_parametrise<A,std::pair<const K,U>>::type;

	public:
		using type = std::map<K,U,Cu,Au>;
	};

	template<typename K, typename V, typename...Ts>
	struct parametric_type_traits<std::map<K,V,Ts...>> {
		using parameter_type = V;
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
		using Map = typename re_parametrise<std::map<K,T,C,A>,U>::type;

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

		/// No-copy overload for endofunctions on temporary maps.
		template<
				typename F,
				typename = typename std::enable_if<
					std::is_same<T,result_of<F(T)>>::value
				>::type
		>
		static Map<T> map(F&& f, Map<T>&& m) {
			for(auto& kv : m) {
				m[kv.first] = f(kv.second);
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
				typename = typename std::enable_if<
					std::is_same<U, result_of<F(U,T)>>::value
				>::type
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
				typename = typename std::enable_if<
					std::is_same<U, result_of<F(T,U)>>::value
				>::type
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


