/*
 * Copyright (c) 2013, 2016 Björn Aili
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
	 *     pure(id) * v   <=> v
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
	 * \ref applicativepg and \ref monoidapg concepts and utilities.
	 *
	 * \code
	 *   #include <ftl/concepts/applicative.h>
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
	template<typename F_>
	struct applicative {

		/**
		 * Clean way of expressing `F` parametrised over different types.
		 */
		template<typename U>
		using F = Rebind<F_,U>;

		/**
		 * The type this particular instance of `F` is parametrised on.
		 *
		 * For example, in the case of `maybe<int>`, `T = int`.
		 */
		using T = Value_type<F_>;

		/**
		 * Encapsulate a pure value in the applicative functor.
		 *
		 * Defaults to `monad<F>::pure`, because any monad is also an
		 * applicative functor.
		 *
		 * \note Default implementation only works if `F` has a monad instance
		 *       defined.
		 *
		 * \note If implementing applicative directly (instead of implicitly by
		 *       way of monad), then `pure` may be overloaded or replaced with a
		 *       `const` reference version, or even a pass by value version.
		 */
		static F<T> pure(const T& x) {
			return monad<F_>::pure(x);
		}

		/// \overload
		static F<T> pure(T&& x) {
			return monad<F_>::pure(std::move(x));
		}

		/**
		 * Map a function to inner value of functor.
		 *
		 * Much like `pure`, defaults to the monad implementation, in order to
		 * prevent need for duplicate code.
		 *
		 * \see functor<F>::map
		 */
		template<typename Fn, typename U = result_of<Fn(T)>>
		static F<U> map(Fn&& fn, const F<T>& f) {
			return monad<F_>::map(std::forward<Fn>(fn), f);
		}

		/// \overload
		template<typename Fn, typename U = result_of<Fn(T)>>
		static F<U> map(Fn&& fn, F<T>&& f) {
			return monad<F_>::map(std::forward<Fn>(fn), std::move(f));
		}

		/**
		 * Contextualised function application.
		 *
		 * Applies the wrapped/contextualised function `fn` to the similarly
		 * wrapped value `f`.
		 *
		 * Default implementation is to use the monad instance's version.
		 *
		 * \note Default implementation only works if `F` is already a monad.
		 */
		template<
				typename Ff,
				typename Fn = Value_type<::std::decay_t<Ff>>,
				typename U = result_of<Fn(T)>>
		static F<U> apply(Ff&& fn, const F<T>& f) {
			return monad<F_>::apply(std::forward<Ff>(fn), f);
		}

		/// \overload
		template<
				typename Ff,
				typename Fn = Value_type<::std::decay_t<Ff>>,
				typename U = result_of<Fn(T)>>
		static F<U> apply(Ff&& fn, F<T>&& f) {
			return monad<F_>::apply(std::forward<Ff>(fn), std::move(f));
		}

		/**
		 * Used for compile time checks.
		 *
		 * Implementors that aren't also Monads \em must override this default.
		 */
		static constexpr bool instance = monad<F_>::instance;
	};

	/**
	 * Concepts lite-compatible predicate for applicative instances.
	 *
	 * Can of course be used for similar purposes by way of SFINAE already.
	 *
	 * \par Examples
	 *
	 * Using implicit bool conversion:
	 * \code
	 *   template<
	 *       typename F,
	 *       typename = Requires<Applicative<F>{}>
	 *   >
	 *   myFunction(const F& f);
	 * \endcode
	 *
	 * \ingroup applicative
	 */
	template<typename F>
	struct Applicative {
		static constexpr bool value = applicative<F>::instance;

		constexpr operator bool() const noexcept {
			return value;
		}
	};

	/**
	 * Inheritable default implementation of `applicative::pure`.
	 *
	 * This implementation of `pure` is applicable to any type with either a
	 * suitable unary constructor (e.g., one taking a single
	 * `Value_type<F>` as argument), or a constructor taking an
	 * initialiser list of the same kind.
	 *
	 * \tparam F must satisfy `std::is_constructible<F,Value_type<F>>`
	 *
	 * \par Examples
	 *
	 * Derive a `pure` implementation for `UserType`.
	 * \code
	 *   template<typename T>
	 *   struct applicative<UserType<T>> : deriving_pure<UserType<T>> {
	 *       // Implementation of map and apply
	 *   };
	 * \endcode
	 *
	 * \ingroup applicative
	 */
	template<typename F>
	struct deriving_pure {
		using T = Value_type<F>;

		static constexpr F pure(const T& t)
		noexcept(std::is_nothrow_constructible<F,const T&>::value) {
			return F{t};
		}

		static constexpr F pure(T&& t)
		noexcept(std::is_nothrow_constructible<F,T&&>::value) {
			return F{std::move(t)};
		}
	};

	/**
	 * Convenience operator to ease applicative style programming.
	 *
	 * In other words,
	 * \code
	 *   a * b <=> ftl::aapply(a, b)
	 * \endcode
	 * where `a` and `b` are complete types of `F`, and `F` is an applicative
	 * functor.
	 *
	 * \ingroup applicative
	 */
	template<
			typename F,
			typename Fn,
			typename F_ = ::std::decay_t<F>,
			typename = Requires<
				Applicative<F_>::value,
				is_same_template<::std::decay_t<Fn>,F_>::value
			>
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
	 * \par Examples
	 *
	 * Using `aPure` to create an instance of some applicative
	 * \code
	 *   template<typename F>
	 *   void example(const F& f) {
	 *     using Fint = ftl::Rebind<F,int>;
	 *     Fint x = aPure<Fint>()(12);
	 *   }
	 * \endcode
	 *
	 * \ingroup applicative
	 */
	template<typename F>
	struct aPure {
		static_assert(
			Applicative<F>(),
			"F is not an instance of Applicative"
		);

		using T = Value_type<F>;

		F operator() (const T& t) const {
			return applicative<F>::pure(t);
		}

		F operator() (T&& t) const {
			return applicative<F>::pure(std::move(t));
		}
	};

	// TODO: C++14: template variable instance of aPure.

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _aapply : public _dtl::curried_binf<_aapply> {
		template<typename Fn, typename F, typename F_ = ::std::decay_t<F>>
		auto operator() (Fn&& u, F&& v) const
		-> decltype(applicative<F_>::apply(
				std::forward<Fn>(u), std::forward<F>(v))) {

			return applicative<F_>::apply(
				std::forward<Fn>(u), std::forward<F>(v));
		}

		using _dtl::curried_binf<_aapply>::operator();
	} aapply{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Function object representing `applicative::apply`.
	 *
	 * Makes it syntactically cheap to use `apply` both in higher-order
	 * functions and in general use (when there is reason not to use
	 * `ftl::operator*`).
	 *
	 * Acts as if it were a curried function of type
	 * \code
	 *   (F<(A) -> B>, F<A>) -> F<B>
	 * \endcode
	 *
	 * \par Examples
	 *
	 * Calculate every possible way of summing the values of two lists:
	 * \code
	 *   vector<int> v1{1, 3};
	 *   vector<int> v2{10, 20};
	 *
	 *   auto v3 = ftl::aapply(ftl::fmap(plus<int>, v1), v2);
	 *   // v3 == {11, 21, 13, 23}
	 * \endcode
	 *
	 * \ingroup applicative
	 */
	aapply;
#endif

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
	 * The `ftl::monoidA` interface documentation contains a more concrete
	 * and programmatic definition of what an instance must implement.
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
	 * Compile time check for `monoidA` instances.
	 *
	 * \ingroup applicative
	 */
	template<typename Alt>
	struct MonoidAlt {
		static constexpr bool value = monoidA<Alt>::instance;

		constexpr operator bool() const noexcept {
			return value;
		}
	};

	/**
	 * Convenience operator for monoidA::orDo
	 *
	 * \ingroup applicative
	 */
	template<
			typename F1,
			typename F2,
			typename F = ::std::decay_t<F1>,
			typename = Requires<
				MonoidAlt<F>()
				&& std::is_same<F,::std::decay_t<F2>>::value
			>
	>
	F operator| (F1&& f1, F2&& f2) {
		return monoidA<F>::orDo(std::forward<F1>(f1), std::forward<F2>(f2));
	}
}

#endif

