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
#ifndef FTL_PRELUDE_H
#define FTL_PRELUDE_H

#include "function.h"

namespace ftl {
	/**
	 * \defgroup prelude Prelude
	 *
	 * A number of utility functions useful with higher order functions.
	 *
	 * \code
	 *   #include <ftl/prelude.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - <ftl/function.h>
	 */

	/**
	 * Identity function.
	 *
	 * Returns whatever is given as parameter. This can be useful in combination
	 * with certain higher order functions.
	 *
	 * \ingroup prelude
	 */
	template<typename T>
	const T& id(const T& t) {
		return t;
	}

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
	 * \ingroup prelude
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
	 * \ingroup prelude
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
	 * \ingroup prelude
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
	 * \ingroup prelude
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
	 * \ingroup prelude
	 */
	template<typename R, typename T1, typename T2>
	function<R,T1,T2> uncurry(function<function<R,T2>,T1> f) {
		return [f] (T1 t1, T2 t2) {
			return f(std::forward<T1>(t1))(std::forward<T2>(t2));
		};
	}

	/**
	 * Function composition first base case.
	 *
	 * Composes an arbitrary function object with a function pointer.
	 *
	 * \ingroup prelude
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
	 * \ingroup prelude
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
	 * \ingroup prelude
	 */
	template<typename F, typename...Fs>
	auto compose(F f, Fs...fs)
	-> decltype(compose(f,compose(std::forward<Fs>(fs)...))) {
		return compose(f, compose(std::forward<Fs>(fs)...));
	}

	/**
	 * Flip the parameter order of a binary function.
	 *
	 * \ingroup prelude
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
	 * \ingroup prelude
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
	 * \ingroup prelude
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

