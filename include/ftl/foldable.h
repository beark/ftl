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
	 * \defgroup foldable Foldable
	 *
	 * Abstraction of data structures that can be folded to some summary value.
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
	 * \ingroup foldable
	 */
	template<template<typename...> class F>
	struct foldable {
// Just as in monad, we don't want the compiler to find these, but the API
// reference generator should.
#ifdef SILLY_WORKAROUND
		/**
		 * Fold a structure containing a monoidal type.
		 *
		 * \tparam M must be a \ref monoid
		 */
		template<
				typename M,
				typename = typename std::enable_if<monoid<M>::instance>::type,
				typename...Ts>
		static M fold(const F<M,Ts...>& f);

		/**
		 * Map each element to a monoid and fold the result.
		 *
		 * \tparam M must be a \ref monoid.
		 * \tparam Fn must satisfy \ref fn`<M(A)>`
		 */
		template<
				typename A,
				typename Fn,
				typename M = typename std::result_of<Fn(A)>::type,
				typename = typename std::enable_if<monoid<M>::instance>::type,
				typename...Ts>
		static M foldMap(Fn&& fn, const F<A,Ts...>& f);

		/**
		 * Right associative fold.
		 *
		 * \param fn Binary folding function
		 * \param z Zero value to combine the first element with
		 * \param f Structure to fold
		 *
		 * \tparam Fn must satisfy \ref fn`<B(A,B)>`
		 */
		template<
				typename Fn,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						B,
						typename decayed_result<Fn(A,B)>::type
						>::value
					>::type,
				typename...Ts>
		static B foldr(Fn&& fn, B&& z, const F<A,Ts...>& f);

		/**
		 * Left associative fold.
		 *
		 * \param fn Binary folding function
		 * \param z Zero value to combine first element with
		 * \param f Structure to fold
		 *
		 * \tparam Fn must satisfy \ref fn`<A(B,A)>`
		 */
		template<
				typename Fn,
				typename A,
				typename B,
				typename = typename std::enable_if<
					std::is_same<
						A,
						typename decayed_result<Fn(B,A)>::type
						>::value
					>::type,
				typename...Ts>
		static A foldl(Fn&& fn, A&& z, const F<B,Ts...>& f);

#endif

		/// Compile time constant to check if a type is an instance.
		static constexpr bool instance = false;
	};

	template<template<typename> class F>
	struct foldable<F> {
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
	template<template<typename...> class F>
	struct foldMap_default {
		template<
				typename A,
				typename Fn,
				typename M = typename decayed_result<Fn(A)>::type,
				typename = typename std::enable_if<monoid<M>::instance>::type,
				typename...Ts>
		static M foldMap(Fn fn, const F<A,Ts...>& f) {
			return foldable<F>::foldl(
					[fn](const A& a, const M& b) {
						return monoid<M>::append(
							fn(a),
							b);
					},
					monoid<M>::id(),
					f);
		}
	};

	template<template<typename> class F>
	struct foldMap_default<F> {
		template<
				typename A,
				typename Fn,
				typename M = typename decayed_result<Fn(A)>::type,
				typename = typename std::enable_if<monoid<M>::instance>::type>
		static M foldMap(Fn fn, const F<A>& f) {
			return foldable<F>::foldl(
					[fn](const A& a, const M& b) {
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
	template<template<typename...> class F>
	struct fold_default {
		template<
				typename M,
				typename = typename std::enable_if<monoid<M>::instance>::type,
				typename...Ts>
		static M fold(const F<M,Ts...>& f) {
			return foldable<F>::foldMap(id<M>, f);
		}
	};

	template<template<typename> class F>
	struct fold_default<F> {
		template<
				typename M,
				typename = typename std::enable_if<monoid<M>::instance>::type>
		static M fold(const F<M>& f) {
			return foldable<F>::foldMap(id<M>, f);
		}
	};

	/**
	 * Convenience function alias of foldable<T>::fold.
	 *
	 * \ingroup foldable
	 */
	template<
			template<typename...> class F,
			typename M,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<monoid<M>::instance>::type,
			typename...Ts>
	M fold(const F<M,Ts...>& f) {
		return foldable<F>::fold(f);
	}

	/**
	 * Overloaded for singly type parametrised types.
	 *
	 * \ingroup foldable
	 */
	template<
			template<typename> class F,
			typename M,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<monoid<M>::instance>::type>
	M fold(const F<M>& f) {
		return foldable<F>::fold(f);
	}

	/**
	 * Convenience function alias of foldable<T>::foldMap.
	 *
	 * \ingroup foldable
	 */
	template<
			template<typename...> class F,
			typename A,
			typename Fn,
			typename M = typename std::result_of<Fn(A)>::type,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<monoid<M>::instance>::type,
			typename...Ts>
	static M foldMap(Fn&& fn, const F<A,Ts...>& f) {
		return foldable<F>::foldMap(std::forward<Fn>(fn), f);
	}

	/**
	 * Overloaded for singly type parametrised types.
	 *
	 * \ingroup foldable
	 */
	template<
			template<typename> class F,
			typename A,
			typename Fn,
			typename M = typename std::result_of<Fn(A)>::type,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<monoid<M>::instance>::type>
	static M foldMap(Fn&& fn, const F<A>& f) {
		return foldable<F>::foldMap(std::forward<Fn>(fn), f);
	}

	/**
	 * Convenience function alias of foldable<T>::foldr.
	 *
	 * \ingroup foldable
	 */
	template<
			template<typename...> class F,
			typename Fn,
			typename A,
			typename B,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<
				std::is_same<
					B,
					typename decayed_result<Fn(A,B)>::type
					>::value
				>::type,
			typename...Ts>
	B foldr(Fn&& fn, B&& z, const F<A,Ts...>& f) {
		return foldable<F>::foldr(std::forward<Fn>(fn), std::forward<B>(z), f);
	}

	/**
	 * Overloaded for singly type parametrised types.
	 *
	 * \ingroup foldable
	 */
	template<
			template<typename> class F,
			typename Fn,
			typename A,
			typename B,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<
				std::is_same<
					B,
					typename decayed_result<Fn(A,B)>::type
					>::value
				>::type>
	B foldr(Fn&& fn, B&& z, const F<A>& f) {
		return foldable<F>::foldr(std::forward<Fn>(fn), std::forward<B>(z), f);
	}

	/**
	 * Convenience function alias of foldable<T>::foldl.
	 *
	 * \ingroup foldable
	 */
	template<
			template<typename...> class F,
			typename Fn,
			typename A,
			typename B,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<
				std::is_same<
					A,
					typename decayed_result<Fn(B,A)>::type
					>::value
				>::type,
			typename...Ts>
	A foldl(Fn&& fn, A&& z, const F<B,Ts...>& f) {
		return foldable<F>::foldl(std::forward<Fn>(fn), std::forward<A>(z), f);
	}

	/**
	 * Overloaded for singly type parametrised types.
	 *
	 * \ingroup foldable
	 */
	template<
			template<typename> class F,
			typename Fn,
			typename A,
			typename B,
			typename = typename std::enable_if<foldable<F>::instance>::type,
			typename = typename std::enable_if<
				std::is_same<
					A,
					typename decayed_result<Fn(B,A)>::type
					>::value
				>::type>
	A foldl(Fn&& fn, A&& z, const F<B>& f) {
		return foldable<F>::foldl(std::forward<Fn>(fn), std::forward<A>(z), f);
	}

}

#endif

