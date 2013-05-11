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
#ifndef FTL_APPLICATIVE_H
#define FTL_APPLICATIVE_H

#include "functor.h"
#include "function.h"

namespace ftl {

	template<template<typename...> class M>
	struct monad;

	/**
	 * \defgroup applicative Applicative Functor
	 *
	 * \breif One step above a functor and one step below a monad.
	 *
	 * What this means is that it has slightly more structure than a plain
	 * functor, but less than a monad.  Specifically, what applicative adds on
	 * functors is a means of contextualising a "pure" value (go from \c type to
	 * \c F<type>, where \c F is some applicative functor), as well as a means
	 * to apply a function that is already in the context of the applicative
	 * functor.
	 *
	 * All Applicative Functors are also Functors. There is no need to define
	 * an instance for both concepts.
	 *
	 * Finally, an applicative instance must satisfy the following laws:
	 * - **Identity law**
	 *
	 *   given
	 *   \code
	 *     template<typename T>
	 *     T id(T t) { return t; }
	 *   \endcode
	 *   then,
	 *   \code
	 *     pure(id<T>) * v   <=> v
	 *   \endcode
	 *
	 * - **Homomorphism law**
	 *   \code
	 *     pure(f) * pure(x) <=> pure(f(x))
	 *   \endcode
	 *
	 * \par Creating new instances
	 * \code
	 *   #include <ftl/applicative.h>
	 * \endcode
	 * Specialise the ftl::applicative struct and make sure to implement the
	 * static methods found under its documentation.
	 *
	 * \ingroup concepts
	 */

	/**
	 * \interface applicative
	 *
	 * \brief Struct that must be specialised to implement the applicative
	 *        concept.
	 *
	 * \note There exists a second, essentially duplicate interface, used by
	 *       types parameterised _only_ on the type they're an applicative
	 *       functor on. I.e., there is an interface with the appearance
	 *       \code
	 *         template<template<typename> class F>
	 *         struct applicative;
	 *       \endcode
	 *
	 * \ingroup applicative
	 */
	template<template<typename...> class F>
	struct applicative {

		/**
		 * Encapsulate a pure value in the applicative functor.
		 *
		 * Defaults to monad<A>::pure, because any monad is also an
		 * applicative functor.
		 *
		 * For single parameter types implementing applicative, `pure` has the
		 * signature
		 * \code
		 *   template<typename A>
		 *   static F<A> pure(A&& a);
		 * \endcode
		 * instead.
		 *
		 * \note Default implementation only works if F has a monad instance
		 *       defined.
		 *
		 * \note If implementing applicative directly (instead of implicitly by
		 *       way of monad), then `pure` may be overloaded or replaced with a
		 *       `const` reference version, or even a pass by value version.
		 */
		template<typename A, typename...Ts>
		static F<A,Ts...> pure(A&& a) {
			return monad<F>::pure(std::forward<A>(a));
		}

		/**
		 * Map a function to inner value of functor.
		 *
		 * \see functor<F>::map
		 */
		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type,
			typename...Ts>
		static F<B,Ts...> map(Fn&& fn, const F<A,Ts...>& f) {
			return monad<F>::map(std::forward<Fn>(fn), f);
		}

		/// \overload
		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type,
			typename...Ts>
		static F<B,Ts...> map(Fn&& fn, F<A,Ts...>&& f) {
			return monad<F>::map(std::forward<Fn>(fn), std::move(f));
		}

		/**
		 * Contextualised function application.
		 *
		 * Applies the wrapped/contextualised function fn to the similarly
		 * wrapped value f.
		 *
		 * Default implementation is to use monad's \c ap().
		 *
		 * \tparam Fn must satisfy `Function<B(A)>`, where `B` is an arbitrary
		 *         type that can be wrapped in `F`.
		 *
		 * \note Default implementation only works if F is already a monad.
		 */
		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type,
			typename...Ts>
		static F<B,Ts...> apply(const F<Fn,Ts...>& fn, const F<A,Ts...>& f) {
			return ap(fn, f);
		}

		/// \overload
		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type,
			typename...Ts>
		static F<B,Ts...> apply(F<Fn,Ts...>&& fn, F<A,Ts...>&& f) {
			return ap(std::move(fn), std::move(f));
		}

		/**
		 * Used for compile time checks.
		 *
		 * Implementors that aren't also Monads \em must override this default.
		 */
		static constexpr bool instance = monad<F>::instance;
	};

	template<template<typename> class F>
	struct applicative<F> {
		template<typename A>
		static F<A> pure(A a) {
			return monad<F>::pure(std::forward<A>(a));
		}

		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type>
		static F<B> map(Fn&& fn, const F<A>& f) {
			return monad<F>::map(std::forward<Fn>(fn), f);
		}

		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type>
		static F<B> map(Fn&& fn, F<A>&& f) {
			return monad<F>::map(std::forward<Fn>(fn), std::move(f));
		}

		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type>
		static F<B> apply(const F<Fn>& fn, const F<A>& f) {
			return ap(fn, f);
		}

		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type>
		static F<B> apply(F<Fn>&& fn, F<A>&& f) {
			return ap(std::move(fn), std::move(f));
		}

		static constexpr bool instance = monad<F>::instance;
	};

	/**
	 * Convenience operator to ease applicative style programming.
	 *
	 * \code
	 *   a * b <=> applicative<F>::apply(a, b)
	 * \endcode
	 * Where \c a and \c b are complete types of \c F and \c F is an applicative
	 * functor.
	 *
	 * \ingroup applicative
	 */
	template<
		template<typename...> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<applicative<F>::instance>::type,
		typename B = typename decayed_result<Fn(A)>::type,
		typename...Ts>
	F<B,Ts...> operator* (const F<Fn,Ts...>& u, const F<A,Ts...>& v) {
		return applicative<F>::apply(u, v);
	}

	/// \overload
	template<
		template<typename...> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<applicative<F>::instance>::type,
		typename B = typename decayed_result<Fn(A)>::type,
		typename...Ts>
	F<B,Ts...> operator* (F<Fn,Ts...>&& u, F<A,Ts...>&& v) {
		return applicative<F>::apply(std::move(u), std::move(v));
	}

	/// \overload
	template<
		template<typename> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<applicative<F>::instance>::type,
		typename B = typename decayed_result<Fn(A)>::type>
	F<B> operator* (const F<Fn>& u, const F<A>& v) {
		return applicative<F>::apply(u, v);
	}

	/// \overload
	template<
		template<typename> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<applicative<F>::instance>::type,
		typename B = typename decayed_result<Fn(A)>::type>
	F<B> operator* (F<Fn>&& u, F<A>&& v) {
		return applicative<F>::apply(std::move(u), std::move(v));
	}

}

#endif

