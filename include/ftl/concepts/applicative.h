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
#ifndef FTL_APPLICATIVE_H
#define FTL_APPLICATIVE_H

#include "functor.h"

namespace ftl {

	template<typename M>
	struct monad;

	/**
	 * \page applicativepg Applicative Functor
	 *
	 * One step above a functor and one step below a monad.
	 *
	 * What this means is that it has slightly more structure than a plain
	 * functor, but less than a monad.  Specifically, what applicative adds on
	 * functors is a means of contextualising a "pure" value (go from \c type to
	 * \c F<type>, where \c F is some applicative functor), as well as a means
	 * to apply a function that is already in the context of the applicative
	 * functor.
	 *
	 * All Applicative Functors are also Functors. There is no need to define
	 * an instance for Functor if there is one for Applicative.
	 *
	 * Finally, an applicative instance must satisfy the following laws:
	 * - **Identity law**
	 *   \code
	 *     pure(id<T>) * v   <=> v
	 *   \endcode
	 *
	 * - **Homomorphism law**
	 *   \code
	 *     pure(f) * pure(x) <=> pure(f(x))
	 *   \endcode
	 *
	 * \see \ref applicative (module)
	 */

	/**
	 * \defgroup applicative Applicative Functor
	 *
	 * \brief \ref applicativepg and \ref monoidapg concepts and utilities.
	 *
	 * \code
	 *   #include <ftl/applicative.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref functor
	 */

	/**
	 * \interface applicative
	 *
	 * Struct that must be specialised to implement the applicative concept.
	 *
	 * \ingroup applicative
	 */
	template<typename F>
	struct applicative {

		/**
		 * The type F is an applicative functor on.
		 *
		 * For example, in the case of `maybe<int>`, `T = int`.
		 */
		using T = concept_parameter<F>;

		/**
		 * Encapsulate a pure value in the applicative functor.
		 *
		 * Defaults to monad<F>::pure, because any monad is also an
		 * applicative functor.
		 *
		 * \note Default implementation only works if `F` has a monad instance
		 *       defined.
		 *
		 * \note If implementing applicative directly (instead of implicitly by
		 *       way of monad), then `pure` may be overloaded or replaced with a
		 *       `const` reference version, or even a pass by value version.
		 */
		static F pure(const T& x) {
			return monad<F>::pure(x);
		}

		/// \overload
		static F pure(T&& x) {
			return monad<F>::pure(std::move(x));
		}

		/**
		 * Map a function to inner value of functor.
		 *
		 * \see functor<F>::map
		 */
		template<
				typename Fn,
				typename U = result_of<Fn(T)>,
				typename Fu = typename re_parametrise<F,U>::type>
		static Fu map(Fn&& fn, const F& f) {
			return monad<F>::map(std::forward<Fn>(fn), f);
		}

		template<
				typename Fn,
				typename U = result_of<Fn(T)>,
				typename Fu = typename re_parametrise<F,U>::type>
		static Fu map(Fn&& fn, F&& f) {
			return monad<F>::map(std::forward<Fn>(fn), std::move(f));
		}

		/**
		 * Contextualised function application.
		 *
		 * Applies the wrapped/contextualised function `fn` to the similarly
		 * wrapped value `f`.
		 *
		 * Default implementation is to use monad's \c ap().
		 *
		 * \tparam Ff must satisfy
		 *         `std::is_same<typename re_parametrise<Ff,T>::type, F>::value`
		 *
		 * \note Default implementation only works if F is already a monad.
		 */
		template<
			typename Ff,
			typename Ff_ = plain_type<Ff>,
			typename Fn = concept_parameter<Ff_>,
			typename U = result_of<Fn(T)>,
			typename Fu = typename re_parametrise<F,U>::type,
			typename = typename std::enable_if<std::is_same<
				typename re_parametrise<Ff_,T>::type, F
			>::value>::type
		>
		static Fu apply(Ff&& fn, const F& f) {
			return monad<F>::apply(std::forward<Ff>(fn), f);
		}

		/// \overload
		template<
			typename Ff,
			typename Ff_ = plain_type<Ff>,
			typename Fn = concept_parameter<Ff_>,
			typename U = result_of<Fn(T)>,
			typename Fu = typename re_parametrise<F,U>::type,
			typename = typename std::enable_if<std::is_same<
				typename re_parametrise<Ff_,T>::type, F
			>::value>::type
		>
		static Fu apply(Ff&& fn, F&& f) {
			return monad<F>::apply(std::forward<Ff>(fn), std::move(f));
		}

		/**
		 * Used for compile time checks.
		 *
		 * Implementors that aren't also Monads \em must override this default.
		 */
		static constexpr bool instance = monad<F>::instance;
	};

	/**
	 * Convenience operator to ease applicative style programming.
	 *
	 * \code
	 *   a * b <=> applicative<F>::apply(a, b)
	 * \endcode
	 * Where `a` and `b` are complete types of `F` and `F` is an applicative
	 * functor.
	 *
	 * \ingroup applicative
	 */
	template<
			typename F,
			typename Fn,
			typename F_ = plain_type<F>,
			typename = typename std::enable_if<applicative<F_>::instance>::type
	>
	auto operator* (Fn&& u, F&& v)
	-> decltype(applicative<F_>::apply(
				std::forward<Fn>(u),std::forward<F>(v))) {

		return applicative<F_>::apply(
				std::forward<Fn>(u), std::forward<F>(v));
	}

	/**
	 * Convenience function object.
	 *
	 * Provided to make it easier to pass applicative::pure as parameter to
	 * higher order functions, as one might otherwise have to wrap such calls
	 * in a lambda to deal with the ambiguity in face of overloads.
	 *
	 * \ingroup applicative
	 */
	template<typename F>
	struct aPure {
		using T = concept_parameter<F>;

		F operator() (const T& t) const {
			return applicative<F>::pure(t);
		}

		F operator() (T&& t) const {
			return applicative<F>::pure(std::move(t));
		}
	};

	// TODO: C++14: template variable instance of aPure.

	/**
	 * Convenience function object.
	 *
	 * Provided as a short-cut for applicative<T>::apply, when operator*
	 * can not be used. Particularly useful when passing `apply` as argument to
	 * higher order functions.
	 *
	 * \ingroup applicative
	 */
	struct aApply {
		template<typename Fn, typename F, typename F_ = plain_type<F>>
		auto operator()(Fn&& u, F&& v)
		-> decltype(applicative<F_>::apply(
				std::forward<Fn>(u), std::forward<F>(v))) {

			return applicative<F_>::apply(
				std::forward<Fn>(u), std::forward<F>(v));
		}
	};

	/**
	 * Compile time instance of aApply.
	 *
	 * For added convenience.
	 *
	 * \ingroup applicative
	 */
	constexpr aApply aapply{};

	/**
	 * \page monoidapg Monoidal Alternatives
	 *
	 * A monoid where the monoidal operation signifies "choice" somehow.
	 *
	 * This concept abstracts applicative functors that in some manner
	 * encompass the notion of "failure" and are also monoids under some
	 * binary operation that can result in such a failure state. In monoid
	 * terms, the failure state is the same as the identity element.
	 *
	 * \see \ref applicative (module)
	 */

	/**
	 * \interface monoidA
	 *
	 * Concrete interface of monoidal alternatives concept.
	 *
	 * \tparam F must be an \ref applicative.
	 *
	 * \note Due to the constraint on `F` that it is already an applicative
	 *       functor, the monoid operation may be _effectful_, unlike in the
	 *       \ref monoid concept.
	 *
	 * \ingroup applicative
	 */
	template<typename F>
	struct monoidA {
#ifdef DOCUMENTATION_GENERATOR
		/**
		 * Get an instance of the failure state.
		 */
		static F fail();

		/**
		 * Sequence two applicative computations that can fail.
		 *
		 * The implementation of `orDo` should short-circuit if possible. I.e.,
		 * if it makes sense in the context of `F`, then if computing `f1`
		 * results in "success", f2 should not also be computed.
		 */
		static F orDo(const F& f1, const F& f2);

#endif

		static constexpr bool instance = false;
	};

	/**
	 * Convenience operator for monoidA::orDo
	 *
	 * \ingroup applicative
	 */
	template<
			typename F,
			typename = typename std::enable_if<monoidA<F>::instance>::type
	>
	F operator| (const F& f1, const F& f2) {
		return monoidA<F>::orDo(f1, f2);
	}

	/**
	 * \overload
	 *
	 * \ingroup applicative
	 */
	template<
			typename F,
			typename = typename std::enable_if<monoidA<F>::instance>::type
	>
	F operator| (F&& f1, const F& f2) {
		return monoidA<F>::orDo(std::move(f1), f2);
	}

	/**
	 * \overload
	 *
	 * \ingroup applicative
	 */
	template<
			typename F,
			typename = typename std::enable_if<monoidA<F>::instance>::type
	>
	F operator| (const F& f1, F&& f2) {
		return monoidA<F>::orDo(f1, std::move(f2));
	}

	/**
	 * \overload
	 *
	 * \ingroup applicative
	 */
	template<
			typename F,
			typename = typename std::enable_if<monoidA<F>::instance>::type
	>
	F operator| (F&& f1, F&& f2) {
		return monoidA<F>::orDo(std::move(f1), std::move(f2));
	}

	// Forward declarations
	template<typename T>
	class maybe;

	template<typename A>
	constexpr maybe<plain_type<A>> value(A&& a)
	noexcept(std::is_nothrow_constructible<plain_type<A>,A>::value);

	/**
	 * An optional computation.
	 *
	 * If `f` fails, the `optional` computation as a whole "succeeds" but yields
	 * `nothing`, whereas it otherwise yields `value(x)` where `x` is the
	 * computed result of `f`.
	 *
	 * \tparam F must be an instance of ftl::monoidA
	 */
	template<
			typename F,
			typename A = concept_parameter<F>,
			typename = typename std::enable_if<monoidA<F>::instance>::type
	>
	typename re_parametrise<F,maybe<A>>::type optional(const F& f) {
		using Fm = typename re_parametrise<F,maybe<A>>::type;
		return value<A> % f | applicative<Fm>::pure(maybe<A>{});
	}
}

#endif

