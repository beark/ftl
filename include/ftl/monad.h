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
#ifndef FTL_MONAD_H
#define FTL_MONAD_H

#include "functional.h"
#include "applicative.h"

namespace ftl {


	/**
	 * \interface monad
	 *
	 * Definition of the Monad concept.
	 */
	template<template<typename...> class M>
	struct monad {

		/**
		 * Encapsulate a "pure" value.
		 *
		 * Given a plain value, encapsulate it in the monad M.
		 */
		//template<typename A, typename...Ts>
		//static M<A,Ts...> pure(const A&);

		/// \overload
		//template<typename A, typename...Ts>
		//static M<A,Ts...> pure(A&&);

		// template<typename F, typename A, typename B = typename decayed_result<F(A)>::type, typename...Ts>
		//static M<B,Ts...> map(Fn f, M<A,Ts...> m);

		/**
		 * Bind a value and execute a computation in M on it.
		 */
		//template<
			//typename F,
			//typename A,
			//typename B = typename decayed_result<F(A)>::type::value_type,
			//typename...Ts>
		//static M<B,Ts...> bind(const M<A,Ts...>&, F);

		/**
		 * Used for compile time checks.
		 *
		 * Implementors that aren't also Monads \em must override this default.
		 */
		static constexpr bool instance = false;
	};

	/**
	 * Convenience operator for monad::bind.
	 */
	template<
		typename F,
		template <typename...> class M,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type::value_type,
		typename...Ts>
	M<B,Ts...> operator>>= (const M<A,Ts...>& m, F f) {
		return monad<M>::bind(m, f);
	}

	template<
		typename F,
		template <typename> class M,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type::value_type>
	M<B> operator>>= (const M<A>& m, F f) {
		return monad<M>::bind(m, f);
	}

	/**
	 * Lifts a function into M.
	 */
	template<
		template<typename...> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename R = typename decayed_result<F(A)>::type,
		typename...Ts>
	M<R,Ts...> liftM(F f, const M<A,Ts...>& m) {
		return m >>= [f] (A a) {
			return monad<M>::pure(f(std::forward<A>(a)));
		};
	}

	/// \overload
	template<
		template<typename> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename R = typename decayed_result<F(A)>::type>
	M<R> liftM(F f, const M<A>& m) {
		return m >>= [f] (A a) {
			return monad<M>::pure(f(std::forward(a)));
		};
	}

	/**
	 * Apply a function in M to a value in M.
	 *
	 * This is actually exactly equivalent to applicative<M>::apply.
	 */
	template<
		template<typename...> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type,
		typename...Ts>
	M<B,Ts...> ap(M<F,Ts...> f, const M<A,Ts...>& m) {
		return f >>= [&m] (F f) {
			return m >>= [f] (A a) {
				return monad<M>::pure(f(std::forward<A>(a)));
			};
		};
	}

	/// \overload
	template<
		template<typename> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type>
	M<B> ap(M<F> f, const M<A>& m) {
		return f >>= [&m] (function<B,A> f) {
			return m >>= [f] (A a) {
				return monad<M>::pure(f(std::forward<A>(a)));
			};
		};
	}
}

#endif

