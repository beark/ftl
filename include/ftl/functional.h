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

#include "applicative.h"

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
	 * - \ref applicative
	 */

	/**
	 * Applicative Functor instance for ftl::functions.
	 *
	 * \ingroup functional
	 */
	template<>
	struct applicative<function> {

		/// Creates a function that returns `a`, regardless of its parameters.
		template<typename A, typename...Ts>
		static function<A,Ts...> pure(A a) {
			return [a] (Ts...) { return a; };
		}

		/// Equivalent of function composition
		template<
			typename F,
			typename A,
			typename B = typename std::result_of<F(A)>::type,
			typename...Ps>
		static function<B,Ps...> map(F f, function<A,Ps...> fn) {
			return [f,fn] (Ps...ps) {
				return f(fn(std::forward<Ps>(ps)...));
			};
		}

		template<
			typename F,
			typename A,
			typename B = typename std::result_of<F(A)>::type,
			typename...Ts>
		static function<B,Ts...> apply(
				function<function<B,A>, Ts...> fn,
				const function<A,Ts...>& f) {
			return [=] (Ts...ts) {
				auto fab = fn(ts...);
				return fab(f(ts...));
			};
		}

		static constexpr bool instance = true;
	};

	/*
	 * N-ary curry, commented out until GCC fixes the bug where template
	 * parameter packs cannot be captured in lambds...
	 * TODO: Enable once gcc has fixed this
	namespace {

		template<
			typename R,
			typename T,
			typename...Ts,
			typename...Ps>
		auto curry_rec(function<R,Ts...> f, type_vec<T> dummy, Ps...ps)
		-> function<R,T> {
			return [f,ps...] (T t) {
				return f(std::forward<T>(t), std::forward<Ps>(ps)...);
			};
		}

		template<
			typename R,
			typename T,
			typename...OTs,
			typename...Ts,
			typename...Ps>
		auto curry_rec(function<R,OTs..> f, type_vec<T,Ts...> dummy, Ps...ps)
		-> function<decltype(curry_rec(f,type_vec<Ts...>(),std::forward<T>(T()),std::forward<Ps>(ps)...)),T> {
			return  [f,ps...] (T t) {
				return curry_rec(f, type_vec<Ts...>(), std::forward<T>(t), std::forward<Ps>(ps)...);
			};
		}
	}

	template<
		typename R,
		typename T,
		typename...Ts>
	auto curry(function<R,T,Ts...> f)
	-> function<decltype(curry_rec(f,type_vec<Ts...>(), std::forward<T>(T()))), T> {
		return [f] (T t) {
			return curry_rec(f, type_vec<Ts...>(), std::forward<T>(t));
		};
	}
	*/

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

}

#endif

