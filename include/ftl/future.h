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
#include "concepts/monad.h"
#include "concepts/monoid.h"

namespace ftl {

	/**
	 * \defgroup future Future
	 *
	 * Concept instances for `std::future`.
	 *
	 * \code
	 *   #include <ftl/future.h>
	 * \endcode
	 *
	 * This module adds instances of the following concepts to std::future:
	 * - \ref functorpg
	 * - \ref applicativepg
	 * - \ref monadpg
	 * - \ref monoidpg
	 *
	 * \par Dependencies
	 * - <future>
	 * - \ref monad
	 * - \ref monoid
	 */

	// Because futures cannot be copied, only moved, we need to specialise
	// apply to make applicative's apply/operator* work.

	// TODO: When lambdas can capture by move, this won't be necessary
	namespace _dtl {
		template<typename A, typename B>
		struct inner_ap {
			using result_type = B;

			explicit inner_ap(std::future<A>&& f) noexcept
			: _f(std::move(f)) {}

			std::future<B> operator() (function<B(A)> fn) {
				return std::move(_f) >>= [fn](A&& a) {
					return monad<std::future<B>>::pure(fn(std::forward<A>(a)));
				};
			}

			std::future<A> _f;
		};
	}

	/**
	 * Monad instance for `std::future`.
	 *
	 * What this essentially does is allow one to add additional computations
	 * "on top of" the deferred/asynchronous one promised by the future. These
	 * additional computations will only be run once `get()` is called on the
	 * resulting `future`.
	 *
	 * Note that futures are not copyable, so when sequencing them together
	 * with monadic/applicative/functorial combinators, they must either be
	 * ecplicitly moved, or be temporaries already. Put differently, _all_
	 * of the operations defined in this Monad instance work on r-value
	 * references only.
	 *
	 * The launch policy in all cases is `std::launch::deferred`.
	 *
	 * \ingroup future
	 */
	template<typename T>
	struct monad<std::future<T>>
#ifndef DOCUMENTATION_GENERATOR
	: deriving_join<in_terms_of_bind<std::future<T>>>
#endif
	{

		/**
		 * Creates a future that returns `t`.
		 *
		 * Note that `t` is copied in all cases. This is because it's generally
		 * a bad idea to pass references to an async call.
		 */
		static std::future<T> pure(T t) {
			return std::async(
					std::launch::deferred,
					[](T t){ return t; },
					t);
		}

		/**
		 * Apply a function sometime in the future.
		 *
		 * Basically, `f` will not be applied until _both_ of the following
		 * has happened:
		 * - we call `get()` on the result of `map`
		 * - `fa` finishes waiting from the resulting call to its `get()`
		 *
		 * Example:
		 * \code
		 *   double asyncOperation();
		 *   double complexComputation(double);
		 *
		 *   auto computation =
		 *       ftl::fmap(
		 *           complexComputation,
		 *           std::async(std::launch::async, asyncOperation)
		 *       );
		 *
		 *   // May have to wait for asyncOperation to finish
		 *   use(computation.get());
		 * \endcode
		 */
		template<typename F, typename U = result_of<F(T)>>
		static std::future<U> map(F f, std::future<T>&& fa) {
			return std::async(
				std::launch::deferred,
				[](F f, std::future<T>&& fa) {
					return f(fa.get());
				},
				f,
				std::move(fa)
			);
		}

		/**
		 * In the future, give `f` one of its arguments.
		 *
		 * Neither `f` nor `m` is waited on until the _resulting future_
		 * is waited on.
		 *
		 * As always with `apply`, if `f` supports curried calling, several
		 * applies can be chained together, resulting in arbitrary arity
		 * function applications.
		 */
		template<typename F, typename U = result_of<F(T)>>
		static std::future<U> apply(std::future<F>&& f, std::future<T>&& m) {
			return std::move(f) >>= _dtl::inner_ap<T,U>(std::move(m));
		}


		/**
		 * Binds a future value to another future computation.
		 *
		 * Getting the value of the resulting future means waiting for `fa`.
		 * Once that is ready, its value is applied to `f`, resulting in a new
		 * future, which we also wait for and finally return the value of.
		 */
		template<
				typename F,
				typename U = Value_type<result_of<F(T)>>
		>
		static std::future<U> bind(std::future<T>&& fa, F&& f) {
			return std::async(
				std::launch::deferred,
				[](std::future<T>&& fa, plain_type<F> f) {
					return f(fa.get()).get();
				},
				std::move(fa),
				std::forward<F>(f)
			);
		}

#ifdef DOCUMENTATION_GENERATOR
		/**
		 * Flattens two nested futures into one.
		 *
		 * As is to be expected, the only way of accomplishing this feat is
		 * to wait for the outer future.
		 */
		static std::future<T> join(std::future<std::future<T>>&& f);
#endif

		static constexpr bool instance = true;

	};

	/**
	 * Monoid instance of `std::future`.
	 *
	 * \tparam T must satisfy \ref monoidpg
	 *
	 * \ingroup future
	 */
	template<typename T>
	struct monoid<std::future<T>> {

		/// Future representing `monoid<T>::id()`
		static auto id()
		-> typename std::enable_if<monoid<T>::instance,std::future<T>>::type {
			return std::async(
				std::launch::deferred,
				[](){ return monoid<T>::id(); });
		}

		/// Future representing `f1.get() ^ f2.get()`
		static auto append(std::future<T>&& f1, std::future<T>&& f2)
		-> typename std::enable_if<monoid<T>::instance,std::future<T>>::type {
			return std::async(
				std::launch::deferred,
				[](std::future<T>&& f1, std::future<T>&& f2) {
					return monoid<T>::append(f1.get(), f2.get());
				},
				std::move(f1),
				std::move(f2));
		}

		static constexpr bool instance = monoid<T>::instance;
	};
}

#endif

