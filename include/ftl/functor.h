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
	/**
	 * \defgroup concepts Concepts
	 */

	// Forward declaration so we can mention applicatives
	template<template<typename...> class F>
	struct applicative;

	/**
	 * \defgroup functor Functor
	 *
	 * \brief Abstraction of data types that can be "mapped to".
	 *
	 * Mathematically, functors are mappings from one category to another,
	 * following a set of well defined laws. What this means in FTL is that
	 * partial types (ie, vector is partial, vector<int> is complete) can often
	 * be made functors by giving a means of mapping a morphism (function) from
	 * the "pure" universe of plain types, to its own internal type universe.
	 *
	 * Ie, \c int is a plain type, \c vector<int> is an \c int "trapped" in the
	 * context of \c vector, and the mapping should apply the function "inside"
	 * that context. The most obvious way of doing so would be to apply the
	 * mapped function to every element in the vector.
	 *
	 * All instances of Functor should follow the following laws (which are part
	 * of the mathematical definition from which this concept is derived):
	 * - given
	 *   \code
	 *     template<typename T>
	 *     T id(T t) { return t; }
	 *   \endcode
	 *   then,
	 *   \code
	 *     map(id<T>, t)          <=> t
	 *     map(compose(f, g), t)  <=> compose(curry(map)(f), curry(map)(g))(t)
	 *   \endcode
	 *
	 * \par Creating new instances
	 * \code
	 *   #include <ftl/functor.h>
	 * \endcode
	 * Specialise the ftl::functor struct and make sure to implement the static
	 * methods found under its documentation.
	 *
	 * \ingroup concepts
	 */

	/**
	 * \interface functor
	 *
	 * \brief Struct that must be specialised to implement the functor concept.
	 *
	 * This is one of the concrete interface that types wishing to implement the
	 * functor concept must specialise (assuming they do not implement
	 * applicative or monad already).
	 *
	 * In particular, this interface must be specialised by types parameterised
	 * on more types than the one they're a functor on. Such a type must not
	 * change in its other type parameters when being mapped to. For guidance,
	 * look at the type signatures and template parameters of map below.
	 *
	 * \ingroup functor
	 */
	template<template<typename...> class F>
	struct functor {
		/**
		 * Maps a function to the contained value(s).
		 *
		 * \tparam Fn must satisfy Function<B(A)>, where \c B is any arbitrary
		 *         type that can be put into the context of \c F.
		 */
		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type,
			typename...Ts>
		static F<B,Ts...> map(Fn&& fn, const F<A,Ts...>& f) {
			return applicative<F>::map(std::forward<Fn>(fn), f);
		}

		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type,
			typename...Ts>
		static F<B,Ts...> map(Fn&& fn, F<A,Ts...>&& f) {
			return applicative<F>::map(std::forward<Fn>(fn), std::move(f));
		}

		/**
		 * Compile time check whether a type is a functor.
		 *
		 * Because all applicative functors are functors, \c F is an instance
		 * of functor if it is an instance of applicative.
		 */
		static constexpr bool instance = applicative<F>::instance;
	};

	/**
	 * \interface functor
	 *
	 * \brief Struct that must be specialised to implement the functor concept.
	 *
	 * This is one of the concrete interface that types wishing to implement the
	 * functor concept must specialise (assuming they do not implement
	 * applicative or monad already).
	 *
	 * In particular, this interface must be specialised by types parameterised
	 * \em only by the type they're a functor on. E.g., \c maybe specialises
	 * this interface, \c either specialises the other version.
	 *
	 * \ingroup functor
	 */
	template<template<typename> class F>
	struct functor<F> {
		/**
		 * Maps a function to the contained value(s).
		 *
		 * \tparam Fn must satisfy Function<B(A)>, where \c B is any arbitrary
		 *         type that can be put into the context of \c F.
		 */
		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type>
		static F<B> map(Fn&& fn, const F<A>& f) {
			return applicative<F>::map(std::forward<Fn>(fn), f);
		}

		template<
			typename Fn,
			typename A,
			typename B = typename decayed_result<Fn(A)>::type>
		static F<B> map(Fn&& fn, F<A>&& f) {
			return applicative<F>::map(std::forward<Fn>(fn), std::move(f));
		}

		static constexpr bool instance = applicative<F>::instance;
	};

	/**
	 * Convenience operator for functor::map.
	 *
	 * \ingroup functor
	 */
	template<
		template<typename...> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<functor<F>::instance>::type,
		typename...Ts>
	auto operator% (Fn&& fn, const F<A,Ts...>& f)
	-> decltype(functor<F>::map(std::forward<Fn>(fn), f)) {
		return functor<F>::map(std::forward<Fn>(fn), f);
	}

	/// \overload
	template<
		template<typename...> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<functor<F>::instance>::type,
		typename...Ts>
	auto operator% (Fn&& fn, F<A,Ts...>&& f)
	-> decltype(functor<F>::map(std::forward<Fn>(fn), std::move(f))) {
		return functor<F>::map(std::forward<Fn>(fn), std::move(f));
	}

	/// \overload
	template<
		template<typename> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<functor<F>::instance>::type>
	auto operator% (Fn&& fn, const F<A>& f)
	-> decltype(functor<F>::map(std::forward<Fn>(fn), f)) {
		return functor<F>::map(std::forward<Fn>(fn), f);
	}

	/// \overload
	template<
		template<typename> class F,
		typename Fn,
		typename A,
		typename = typename std::enable_if<functor<F>::instance>::type>
	auto operator% (Fn&& fn, F<A>&& f)
	-> decltype(functor<F>::map(std::forward<Fn>(fn), std::move(f))) {
		return functor<F>::map(std::forward<Fn>(fn), std::move(f));
	}

	/**
	 * Distribute function inside a context across entire context.
	 *
	 * A practical example would be if you had a list of functions from
	 * A to R, then \c distributing that list would give you a function from
	 * A to a list of Rs.
	 *
	 * \ingroup functor
	 */
	/*
	 * TODO: Implement the Representable concept and make distribute work
	 * with any Representable.
	 */
	template<
		template<typename...> class F,
		typename A,
		typename R,
		typename = typename std::enable_if<functor<F>::instance>::type,
		typename...Ts>
	function<F<R,Ts...>,A> distribute(F<function<R,A>,Ts...> f) {
		return [f](A a) {
			functor<F>::map(
				[f,a](function<R,A> fn) {
					return f(a);
				},
				f);
		};
	}

	/// \overload
	template<
		template<typename> class F,
		typename A,
		typename R,
		typename = typename std::enable_if<functor<F>::instance>::type>
	function<F<R>,A> distribute(F<function<R,A>> f) {
		return [f](A a) {
			functor<F>::map(
				[f,a](function<R,A> fn) {
					return f(a);
				},
				f);
		};
	}
}

#endif

