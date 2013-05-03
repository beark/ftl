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
		template<typename>
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
	template<>
	struct monad<std::future> {

		/**
		 * Creates a future that returns t.
		 */
		template<typename T>
		static std::future<T> pure(T&& t) {
			return std::async(
					std::launch::deferred,
					[](T t){ return t; },
					std::forward<T>(t));
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
		static std::future<B> map(F&& f, std::future<A>&& fa) {
			return std::async(
				std::launch::deferred,
				[](F&& f, std::future<A>&& fa) {
					return f(fa.get());
				},
				std::forward<F>(f),
				std::move(fa)
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
				typename std::result_of<F(A)>::type>::type>
		static std::future<B> bind(std::future<A>&& fa, F&& f) {
			return std::async(
				std::launch::deferred,
				[](std::future<A>&& fa, F&& f) {
					return f(fa.get()).get();
				},
				std::move(fa),
				std::forward<F>(f)
			);
		}

		// Poof! futures are functors, applicatives, and monads!
		static constexpr bool instance = true;

	};

	// Because futures cannot be copied, only moved, we need to specialise
	// ap to make applicative's apply/operator* work.

	// TODO: When lambdas can capture by move, this won't be necessary
	namespace _dtl {
		template<typename A, typename B>
		struct inner_ap {
			using result_type = B;

			explicit inner_ap(std::future<A>&& f) noexcept
			: _f(std::move(f)) {}

			std::future<B> operator() (function<B,A> fn) {
				return std::move(_f) >>= [fn](A&& a) {
					return monad<std::future>::pure(fn(std::forward<A>(a)));
				};
			}

			std::future<A> _f;
		};
	}

	/// Specialised for futures, because they cannot be copied.
	template<
		typename F,
		typename A,
		typename B = typename std::result_of<F(A)>::type>
	std::future<B> ap(std::future<F>&& f, std::future<A>&& m) {
		return std::move(f) >>= _dtl::inner_ap<A,B>(std::move(m));
	}
}

#endif

