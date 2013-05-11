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
	 * - ftl/applicative.h
	 * - ftl/function.h
	 * - ftl/monoid.h
	 * - ftl/type_functions.h
	 * - memory
	 * - functional
	 * - type_traits
	 *
	 * \ingroup modules
	 */

	/**
	 * \defgroup fn Function<R(Ps...)>
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
	 * The purpose of the "template parameters" of this concept is to
	 * distinguish specifically what types the function constraining a parameter
	 * by this concept require the function to work on/with. It's quite simple,
	 * really: `Function<B(A)>` simply means that the type constrained by the
	 * aforementioned must take a single parameter of type `A`, and return a
	 * value of type `B`.
	 *
	 * \ingroup concepts
	 */

	/**
	 * Applicative Functor instance for ftl::functions.
	 *
	 * \ingroup functional
	 * \ingroup function
	 * \ingroup applicative
	 * \ingroup functor
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
	 * Curries a binary function.
	 *
	 * Currying is the process of turning a function of (a,b) -> c into
	 * a -> b -> c. In other words, instead of taking two arguments and
	 * returning the answer, the curried function takes one argument and
	 * returns a function that takes another one and \em then returns the
	 * answer.
	 *
	 * \note This requires the "outer" function (the one who takes a parameter
	 *       and returns an inner function) to capture its parameter by value.
	 *       In other words, T1 must be copy or move constructible and must not
	 *       refer to something that will be destroyed or otherwise made
	 *       inaccessible.
	 *
	 * \ingroup functional
	 */
	template<typename R, typename T1, typename T2>
	function<function<R,T2>,T1> curry(function<R,T1,T2> f) {
		// TODO: Change with move capturing lambdas (C++14)
		return [f] (T1 t1) {
			return [f,t1] (T2&& t2) {
				return f(t1, std::forward<T2>(t2));
			};
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup functional
	 */
	template<typename R, typename T1, typename T2>
	function<function<R,T2>,T1> curry(R (*f) (T1, T2)) {
		return [f](T1 t1) {
			return [f, t1](T2&& t2) {
				return f(t1, std::forward<T2>(t2));
			};
		};
	}

	/**
	 * Curry a ternary function
	 *
	 * Similar to the binary curry, except this works with functions taking
	 * three parameters.
	 *
	 * \ingroup functional
	 */
	template<typename R, typename T1, typename T2, typename T3>
	function<function<function<R,T3>,T2>,T1> curry(function<R,T1,T2,T3> f) {
		return [f](T1 t1) {
			return [f,t1](T2 t2) {
				return [f,t1,t2](T3&& t3) {
					return f(t1, t2, std::forward<T2>(t2));
				};
			};
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup functional
	 */
	template<typename R, typename T1, typename T2, typename T3>
	function<function<function<R,T3>,T2>,T1> curry(R (*f) (T1, T2, T3)) {
		return [f](T1 t1) {
			return [f, t1](T2 t2) {
				return [f,t1,t2](T3&& t3) {
					return f(t1, t2, std::forward<T3>(t3));
				};
			};
		};
	}

	/**
	 * Uncurries a binary function.
	 *
	 * \ingroup functional
	 */
	template<typename R, typename T1, typename T2>
	function<R,T1,T2> uncurry(function<function<R,T2>,T1> f) {
		return [f] (T1 t1, T2 t2) {
			return f(std::forward<T1>(t1))(std::forward<T2>(t2));
		};
	}

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

	/**
	 * Function composition first base case.
	 *
	 * Composes an arbitrary function object with a function pointer.
	 *
	 * \ingroup functional
	 */
	template<
		typename F,
		typename A,
		typename B = typename std::result_of<F(A)>::type,
		typename...Ps>
	function<B,Ps...> compose(F f, A (*fn)(Ps...)) {
		return [f,fn](Ps...ps) {
			return f(fn(std::forward<Ps>(ps)...));
		};
	}

	/**
	 * Function composition second base case.
	 *
	 * Composes an arbitrary function object with an ftl::function.
	 *
	 * \ingroup functional
	 */
	template<
		typename F,
		typename A,
		typename B = typename std::result_of<F(A)>::type,
		typename...Ps>
	function<B,Ps...> compose(F f, function<A,Ps...> fn) {
		return [f,fn](Ps...ps) {
			return f(fn(std::forward<Ps>(ps)...));
		};
	}

	/**
	 * Generalised, n-ary function composition.
	 *
	 * Composes an arbitrary number of functions, where each function's return
	 * value is piped to the next. The right-most function in the sequence is
	 * the first to be evaluated and its result is passed to the one step to the
	 * left. Return values must match parameter type of the next one in the
	 * chain.
	 *
	 * \ingroup functional
	 */
	template<typename F, typename...Fs>
	auto compose(F f, Fs...fs)
	-> decltype(compose(f,compose(std::forward<Fs>(fs)...))) {
		return compose(f, compose(std::forward<Fs>(fs)...));
	}

	/**
	 * Flip the parameter order of a binary function.
	 *
	 * \ingroup functional
	 */
	template<typename A, typename B, typename R>
	function<R,B,A> flip(function<R,A,B> f) {
		return [f](B b, A a) {
			return f(std::forward<A>(a), std::forward<B>(b));
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup functional
	 */
	template<typename A, typename B, typename R>
	function<R,B,A> flip(R (&f) (A,B)) {
		return [&f](B b, A a) {
			return f(std::forward<A>(a), std::forward<B>(b));
		};
	}

	/**
	 * Flip parameter order of a curried binary function.
	 *
	 * \ingroup functional
	 */
	template<typename R, typename A, typename B>
	function<function<R,A>,B> flip(function<function<R,B>,A> f) {
		return [f](B b) {
			return [f,b](A a) {
				return f(std::forward<A>(a))(b);
			};
		};
	}

}

#endif

