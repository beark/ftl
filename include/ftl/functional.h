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
#ifndef FTL_FUNCTIONAL_H
#define FTL_FUNCTIONAL_H

#include "monad.h"

namespace ftl {
	/**
	 * \page fn Function
	 *
	 * \brief Abstraction of callable objects.
	 *
	 * This differs from the data type ftl::function in that it is not a
	 * concrete type. While ftl::function is certainly an instance of this
	 * concept, they are not the only data type to be so.
	 *
	 * Other instances include (but are not limited to):
	 * \li std::function
	 * \li regular functions
	 * \li lambdas
	 * \li *anything* else with an `operator()` defined
	 *
	 * When possible, instances of Function should also include the type alias
	 * `result_type`, which should be the type returned by calling `operator()`.
	 *
	 * The purpose of the "template parameters" often found when other parts of
	 * the documentation reference this concept is to distinguish specifically
	 * what types the function constraining a parameter by this concept require
	 * the particular Function instance to work on/with. It's quite simple,
	 * really: `Function<B(A)>` simply means that the type constrained by the
	 * aforementioned must take a single parameter of type `A`, and return a
	 * value of type `B`.
	 */

	/**
	 * \defgroup functional Functional
	 *
	 * \brief A collection of higher order utility functions.
	 *
	 * \code
	 *   #include <ftl/functional.h>
	 * \endcode
	 *
	 * The functions herein deal mostly with modifying functions, such as
	 * composing them, flipping the order of their parameters, and so on.
	 *
	 * \par Dependencies
	 * - \ref monad
	 */

	/**
	 * Monad instance for ftl::functions.
	 *
	 * \ingroup functional
	 */
	template<typename R, typename P, typename...Ps>
	struct monad<function<R,P,Ps...>> {

		/// Creates a function that returns `a`, regardless of its parameters.
		static function<R,P,Ps...> pure(R a) {
			return [a](P,Ps...) { return a; };
		}

		/// Equivalent of function composition
		template<
				typename F,
				typename S = typename std::result_of<F(R)>::type
		>
		static function<S,P,Ps...> map(F f, function<R,P,Ps...> fn) {
			return [f,fn] (P p, Ps...ps) {
				return f(fn(std::forward<P>(p), std::forward<Ps>(ps)...));
			};
		}

		/*
		template<
			typename F,
			typename S = typename std::result_of<F(R)>::type
		>
		static function<S,P,Ps...> apply(function<F,P,Ps...> f, function<R,P,Ps...> g) {

			return [=] (P x,Ps...xs) {
				return f(x,xs...)(g(x,xs...));
			};
		}
		*/

		template<
				typename Fn,
				typename Fs = typename std::result_of<Fn(R)>::type,
				typename S = typename std::result_of<Fs(P,Ps...)>::type
		>
		static function<S,P,Ps...> bind(function<R,P,Ps...> f, Fn fn) {
			return [=](P p, Ps...ps) {
				return fn(f(p, ps...))(p, ps...);
			};
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monoid instance for std::functions returning monoids.
	 *
	 * In essence, the same as ftl::function's implementation.
	 *
	 * \ingroup monoid
	 */
	template<typename M, typename...Ps>
	struct monoid<std::function<M(Ps...)>> {
		static auto id()
		-> typename std::enable_if<
				monoid<M>::instance,
				std::function<M(Ps...)>>::type {
			return [](Ps...ps) { return monoid<M>::id(); };
		}

		static auto append(
				const std::function<M(Ps...)>& f1,
				const std::function<M(Ps...)>& f2)
		-> typename std::enable_if<
				monoid<M>::instance,
				std::function<M(Ps...)>>::type {
			return [=] (Ps...ps) {
				return monoid<M>::append(f1(ps...), f2(ps...));
			};
		}

		static constexpr bool instance = monoid<M>::instance;
	};

	template<typename R, typename S, typename...Ps>
	struct re_parametrise<std::function<R(Ps...)>,S> {
		using type = std::function<S(Ps...)>;
	};

	template<typename R, typename...Ps>
	struct parametric_type_traits<std::function<R(Ps...)>> {
		using parameter_type = R;
	};

	/**
	 * Monad instance for std::function.
	 *
	 * Exactly equivalent of `monad<ftl::function>`.
	 */
	template<typename R, typename P, typename...Ps>
	struct monad<std::function<R(P,Ps...)>> {
		static std::function<R(P,Ps...)> pure(R r) {
			return std::function<R(P,Ps...)> {
				[r](P,Ps...) { return r; }
			};
		}

		template<typename F, typename S = typename std::result_of<F(R)>::type>
		static std::function<S(P,Ps...)> map(F fn, std::function<R(P,Ps...)> f) {
			return std::function<S(P,Ps...)>{
				[fn, f](P&& p,Ps&&...ps) {
					return fn(f(std::forward<P>(p), std::forward<Ps>(ps)...));
				}
			};
		}

		template<
				typename Fn,
				typename Fs = typename std::result_of<Fn(R)>::type,
				typename S = typename std::result_of<Fs(P,Ps...)>::type
		>
		static std::function<S(P,Ps...)> bind(std::function<R(P,Ps...)> f, Fn fn) {
			return [=](P p, Ps...ps) {
				return fn(f(p, ps...))(p, ps...);
			};
		}

		static constexpr bool instance = true;
	};

}

#endif

