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
	template<typename F>
	struct applicative;

	/**
	 * \page functorpg Functor
	 *
	 * Abstraction of contexts that can be mapped to.
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
	 * All instances of Functor should follow the laws below (which are part
	 * of the mathematical definition from which this concept is derived):
	 * - **Preservation of identity**
	 *   \code
	 *     map(id<T>, t)          <=> t
	 *   \endcode
	 * - **Preservatin of composition**
	 *   \code
	 *     map(compose(f, g), t)  <=> compose(curry(map)(f), curry(map)(g))(t)
	 *   \endcode
	 *
	 * \see \ref functor (module)
	 */

	/**
	 * \defgroup functor Functor
	 *
	 * Module containg the \ref functorpg concept and related functions.
	 *
	 * \code
	 *   #include <ftl/functor.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref typelevel
	 * - <ftl/function.h>
	 */

	/**
	 * \interface functor
	 *
	 * \brief Struct that must be specialised to implement the functor concept.
	 *
	 * When specialising, it is important to remember that the described
	 * interface here is _minimal_, and an instance is encouraged to include
	 * additional overloads of the functions for optimization reasons and
	 * similar. For types that cannot be copied, for instance, it makes sense
	 * that all of the functor interface functions accept an rvalue reference.
	 *
	 * \ingroup functor
	 */
	template<typename F>
	struct functor {
		/**
		 * Maps a function to the contained value(s).
		 *
		 * \tparam Fn must satisfy \ref fn<U(T)>, where `T` is the type `F` is
		 *            parametrised on, and `U` is a type the unparametrised
		 *            `F` _can_ encapsulate.
		 */
		template<
				typename Fn,
				typename T = concept_parameter<F>,
				typename U = typename decayed_result<Fn(T)>::type,
				typename Fu = typename re_parametrise<F,U>::type>
		static Fu map(Fn&& fn, const F& f) {
			return applicative<F>::map(
					std::forward<Fn>(fn), f);
		}

		template<
				typename Fn,
				typename T = concept_parameter<F>,
				typename U = typename decayed_result<Fn(T)>::type,
				typename Fu = typename re_parametrise<F,U>::type>
		static Fu map(Fn&& fn, F&& f) {
			return applicative<F>::map(
					std::forward<Fn>(fn),
					std::move(f));
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
	 * Convenience operator for functor::map.
	 *
	 * \ingroup functor
	 */
	template<
		typename F,
		typename Fn,
		typename F_ = plain_type<F>,
		typename = typename std::enable_if<functor<F_>::instance>::type>
	auto operator% (Fn&& fn, F&& f)
	-> decltype(functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f))) {
		return functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f));
	}

	/**
	 * Convenience function object.
	 *
	 * Provided to make it easier to pass functor::map as parameter to
	 * higher order functions, as one might otherwise have to wrap such calls
	 * in a lambda to deal with the ambiguity in face of overloads.
	 *
	 * \ingroup functor
	 */
	template<typename F>
	struct fMap {
		template<typename Fn>
		auto operator() (Fn&& fn, const F& f) const
		-> decltype(functor<F>::map(std::forward<Fn>(fn), f)) {
			return functor<F>::map(std::forward<Fn>(fn), f);
		}

		template<typename Fn>
		auto operator() (Fn&& fn, F&& f) const
		-> decltype(functor<F>::map(std::forward<Fn>(fn), std::move(f))) {
			return functor<F>::map(std::forward<Fn>(fn), std::move(f));
		}
	};

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
			typename...Ts,
			typename = typename std::enable_if<
				functor<F<A,Ts...>>::instance>::type>
	function<F<R,Ts...>,A> distribute(F<function<R,A>,Ts...> f) {
		return [f](A a) {
			functor<F<A,Ts...>>::map(
				[f,a](function<R,A> fn) {
					return f(a);
				},
				f);
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup functor
	 */
	template<
			template<typename> class F,
			typename A,
			typename R,
			typename = typename std::enable_if<
				functor<F<A>>::instance>::type>
	function<F<R>,A> distribute(F<function<R,A>> f) {
		return [f](A a) {
			functor<F<A>>::map(
				[f,a](function<R,A> fn) {
					return f(a);
				},
				f);
		};
	}
}

#endif

