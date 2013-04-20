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

#include "function.h"

namespace ftl {

	template<template<typename...> class M>
	struct monad;

	/**
	 * \interface Applicative
	 *
	 * The Applicative Functor concept.
	 *
	 * An applicative functor is kind of one step above a functor and one step
	 * below a monad.
	 *
	 * All Applicative Functors are also Functors (so if you find yourself
	 * making an Applicative instance, you should also implement fmap).
	 */
	template<template<typename...> class F>
	struct applicative {

		/**
		 * Encapsulate a pure value in the applicative functor.
		 *
		 * Defaults to monad<A>::pure, because any monad is also an
		 * applicative functor.
		 *
		 * \note Default implementation only works if F has a monad instance
		 *       defined.
		 */
		template<typename A, typename...Ts>
		static F<A,Ts...> pure(A a) {
			return monad<F>::pure(std::forward<A>(a));
		}

		/**
		 * Sequential application.
		 *
		 * Default implementation is to use monad's ap.
		 *
		 * \note Default implementation only works if F is already a monad.
		 */
		template<typename A, typename B, typename...Ts>
		static F<B,Ts...> apply(
				const F<function<B,A>,Ts...>& fn,
				const F<A,Ts...>& f) {
			return ap(fn, f);
		}
	};

	/**
	 * Convenience operator to ease applicative style programming.
	 */
	template<
		template<typename...> class F,
		typename A,
		typename B,
		typename...Ts>
	F<B,Ts...> operator* (
			const F<function<B,A>,Ts...>& u,
			const F<A,Ts...>& v) {
		return applicative<F>::apply(u, v);
	}

}

#endif

