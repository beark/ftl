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
#ifndef FTL_FUNCTOR_H
#define FTL_FUNCTOR_H

#include "type_functions.h"
#include "function.h"

namespace ftl {
	// Forward declaration so we can mention applicatives
	template<template<typename...> class F>
	struct applicative;

	/**
	 * \interface functor
	 *
	 * The Functor concept.
	 */
	template<template<typename...> class F>
	struct functor {
		/**
		 * Maps a function to the contained value(s).
		 *
		 * \tparam Fn must satisfy Function<B(A)>
		 */
		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type,
			typename...Ts>
		static F<B,Ts...> map(Fn fn, F<A,Ts...> f) {
			return applicative<F>::map(fn, f);
		}

		/**
		 * Compile time check whether a type is a functor.
		 *
		 * Because all applicative functors are functors, \c F is an instance
		 * of functor if it is an instance of applicative.
		 */
		static constexpr bool instance = applicative<F>::instance;
	};

	template<template<typename> class F>
	struct functor<F> {
		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type>
		static F<B> map(Fn fn, F<A> f) {
			return applicative<F>::map(fn, f);
		}

		static constexpr bool instance = applicative<F>::instance;
	};

	/**
	 * Convenience operator for functor::map.
	 */
	template<
		template<typename...> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<functor<F>::instance>::type,
		typename B = typename decayed_result<Fn(A)>::type,
		typename...Ts>
	F<B,Ts...> operator% (Fn fn, F<A,Ts...> f) {
		return functor<F>::map(fn, f);
	}

	template<
		template<typename> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<functor<F>::instance>::type,
		typename B = typename decayed_result<Fn(A)>::type>
	F<B> operator% (Fn fn, F<A> f) {
		return functor<F>::map(fn, f);
	}

	/**
	 * Distribute function inside a context across entire context.
	 *
	 * A practical example would be if you had a list of functions from
	 * A to R, then \c distributing that list would give you a function from
	 * A to a list of Rs.
	 *
	 * TODO: Implement the Representable concept and make distribute work
	 * with any Representable.
	 */
	template<
		template<typename...> class F,
		typename A,
		typename R,
		typename...Ts>
	function<F<R,Ts...>,A> distribute(F<function<R,A>,Ts...> f) {
		return [f](A a) {
			functor<F>::map(
				[f,a](function<R,A> fn) {
					return f(a);
				},
				f);
		}
	}

	/// \overload
	template<
		template<typename> class F,
		typename A,
		typename R>
	function<F<R>,A> distribute(F<function<R,A>> f) {
		return [f](A a) {
			functor<F>::map(
				[f,a](function<R,A> fn) {
					return f(a);
				},
				f);
		}
	}
}

#endif

