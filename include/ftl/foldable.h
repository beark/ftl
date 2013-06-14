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
#ifndef FTL_FOLDABLE_H
#define FTL_FOLDABLE_H

#include "monoid.h"
#include "prelude.h"

namespace ftl {
	/**
	 * \page foldablepg Foldable
	 *
	 * Abstraction of data structures that can be folded to some summary value.
	 *
	 * Despite what one may be lead to believe, this concept does not quite
	 * correspond to the mathematical notion of a catamorphism. Unlike a _true_
	 * catamorphism, the `Foldable` concept can only traverse structures
	 * "linearly". I.e., it is impossible to use a fold to build an isomorphic
	 * representation of a tree.
	 *
	 * \see \ref foldable (module)
	 */

	/**
	 * \defgroup foldable Foldable
	 *
	 * Interface and utilities relating to the \ref foldablepg concept.
	 *
	 * \code
	 *   #include <ftl/foldable.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref prelude
	 * - \ref monoid
	 */

	/**
	 * \interface foldable
	 *
	 * Concrete definition of the foldable concept.
	 *
	 * The most typical types that are instances of foldable are containers of
	 * different sorts, in which case the fold operation is usually a
	 * generalisation of a "sum" operation on the elements of the container.
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct foldable {
		/// Typedef for easier reference to the type to fold on
		using T = concept_parameter<F>;

// Just as in monad, we don't want the compiler to find these, but the API
// reference generator should.
#ifdef SILLY_WORKAROUND
		/**
		 * Fold a structure containing a monoidal type.
		 *
		 * \tparam T must be a \ref monoid
		 */
		template<typename = typename std::enable_if<monoid<T>::instance>::type>
		static T fold(const F& f);

		/**
		 * Map each element to a monoid and fold the result.
		 *
		 * \tparam Fn must satisfy \ref fn`<M(A)>`
		 */
		template<
				typename Fn,
				typename M = typename std::result_of<Fn(T)>::type,
				typename = typename std::enable_if<monoid<M>::instance>::type
		>
		static M foldMap(Fn&& fn, const F& f);

		/**
		 * Right associative fold.
		 *
		 * \param fn Binary folding function
		 * \param z Zero value to combine the first element with
		 * \param f Structure to fold
		 *
		 * \tparam Fn must satisfy \ref fn`<U(T,U)>`
		 */
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
		static U foldr(Fn&& fn, U&& z, const F& f);

		/**
		 * Left associative fold.
		 *
		 * \param fn Binary folding function
		 * \param z Zero value to combine first element with
		 * \param f Structure to fold
		 *
		 * \tparam Fn must satisfy \ref fn`<U(U,T)>`
		 */
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
		static U foldl(Fn&& fn, U&& z, const F& f);

#endif

		/// Compile time constant to check if a type is an instance.
		static constexpr bool instance = false;
	};

	/**
	 * Default implementation of foldable::foldMap.
	 *
	 * Foldable instances implementing foldable::foldl can simply inherit from
	 * this struct to get foldable::foldMap for "free".
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct foldMap_default {
		template<
				typename Fn,
				typename T = concept_parameter<F>,
				typename M = typename decayed_result<Fn(T)>::type,
				typename = typename std::enable_if<monoid<M>::instance>::type
		>
		static M foldMap(Fn fn, const F& f) {
			return foldable<F>::foldl(
					[fn](const T& a, const M& b) {
						return monoid<M>::append(
							fn(a),
							b);
					},
					monoid<M>::id(),
					f);
		}
	};

	/**
	 * Default implementation of foldable::fold.
	 *
	 * Foldable instances implementing foldable::foldMap can simply inherit
	 * fromt this struct to get foldable::fold for "free".
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct fold_default {
		template<
				typename M = concept_parameter<F>,
				typename = typename std::enable_if<monoid<M>::instance>::type
		>
		static M fold(const F& f) {
			return foldable<F>::foldMap(identity<M>(), f);
		}
	};

	/**
	 * Convenience function alias of foldable<T>::fold.
	 *
	 * \ingroup foldable
	 */
	template<
			typename F,
			typename M = concept_parameter<F>,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<monoid<M>::instance>::type
	>
	M fold(const F& f) {
		return foldable<F>::fold(f);
	}

	/**
	 * Convenience function alias of foldable<T>::foldMap.
	 *
	 * \ingroup foldable
	 */
	template<
			typename F,
			typename T = concept_parameter<F>,
			typename Fn,
			typename M = typename std::result_of<Fn(T)>::type,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<monoid<M>::instance>::type
	>
	static M foldMap(Fn&& fn, const F& f) {
		return foldable<F>::foldMap(std::forward<Fn>(fn), f);
	}

	/**
	 * Convenience function alias of foldable<T>::foldr.
	 *
	 * \ingroup foldable
	 */
	template<
			typename F,
			typename Fn,
			typename U,
			typename T = concept_parameter<F>,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<
				std::is_same<
					U,
					typename decayed_result<Fn(T,U)>::type
				>::value
			>::type
	>
	U foldr(Fn&& fn, U&& z, const F& f) {
		return foldable<F>::foldr(std::forward<Fn>(fn), std::forward<U>(z), f);
	}

	/**
	 * Convenience function alias of foldable<T>::foldl.
	 *
	 * \ingroup foldable
	 */
	template<
			typename F,
			typename Fn,
			typename U,
			typename T = concept_parameter<F>,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<
				std::is_same<
					U,
					typename decayed_result<Fn(U,T)>::type
				>::value
			>::type
	>
	U foldl(Fn&& fn, U&& z, const F& f) {
		return foldable<F>::foldl(std::forward<Fn>(fn), std::forward<U>(z), f);
	}

}

#endif

