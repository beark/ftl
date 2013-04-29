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
#ifndef FTL_FUTURE_H
#define FTL_FUTURE_H

#include <future>
#include "monad.h"

namespace ftl {

	// Some silly implementation details
	// TODO: maybe generalise this and move into type_functions?
	namespace _dtl {
		template<template<typename> class O>
		struct inner_type;

		template<typename T>
		struct inner_type<std::future<T>> {
			using type = T;
		};
	}

	/**
	 * Monad instance for futures.
	 *
	 * This is a pretty powerful instance. It lets us perform all sorts of
	 * computations on future values, without actually waiting for them. Only
	 * when we explicitly \em need the value will it be computed (when we call
	 * \c get on the final future).
	 */
	struct monad<std::future> {

		/**
		 * Creates a future that returns t.
		 */
		template<typename T>
		static std::future<T> pure(T t) {
			return std::async(std::launch:deferred, [](T t){ return t; }, t);
		}

		/**
		 * Apply a function sometime in the future.
		 *
		 * Basically, f will not be applied until \em both of the following
		 * has happened:
		 * \li we call \c get on the result of \c map
		 * \li \c fa finishes waiting from the resulting call to its \c get
		 */
		template<
			typename F,
			typename A,
			typename B = typename std::result_of<F(A)>::type>
		static std::future<B> map(F f, const std::future<A>& fa) {
			return std::async(
				std::launch::deferred,
				[](F f, const std::future<A>& fa) {
					return f(fa.get());
				},
				f,
				fa
			);
		}

		/**
		 * Binds a future value to another future computation.
		 *
		 * Getting the value of the resulting future means waiting for \c fa.
		 * Once that is ready, its value is applied to \c f, resulting in a new
		 * future, which we also wait for and finally return the value of.
		 */
		template<
			typename F,
			typename A,
			typename B = typename _dtl::inner_type<
				typename std::result_of<F(A)>::type>::type>>
		static std::future<B> bind(const std::future<A>& fa, F f) {
			return std::async(
				std::launch::deferred,
				[](const std::future<A>& fa, F f) {
					return f(fa.get()).get();
				},
				fa,
				f
			);
		}

		// Poof! futures are functors, applicatives, and monads!
		static constexpr bool instance = true;

	};
}

#endif

