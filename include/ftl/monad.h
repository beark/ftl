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
#ifndef FTL_MONAD_H
#define FTL_MONAD_H

#include "applicative.h"

namespace ftl {

	/**
	 * \page monadpg Monad
	 *
	 * Abstraction of sequenceable computations in some context.
	 *
	 * Monads are essentially just a functor with some additional structure.
	 * Specifically, types that are monads have the added functionality (on top
	 * of what applicative adds) of sequencing computations in the context of
	 * the monad.
	 *
	 * While this is technically already possible in C++, the abstraction is
	 * useful none the less, because monads also have the power to implicitly
	 * pass state or other useful context information forward, without the user
	 * having to bother with it. 
	 *
	 * The easiest example of the above is probably the maybe monad, where the
	 * user does not need to manually check for nothingness except at the very
	 * end of a sequence of computations&mdash;because maybe's implementation of
	 * monad<M>::bind does that for them.
	 *
	 * As with many of the other concepts in FTL, monads have a set of
	 * associated laws that instances must follow (though technically, there is
	 * nothing enforcing them):
	 * - **Left identity law**
	 *   \code
	 *     pure(x) >>= f    <=> f(x)
	 *   \endcode
	 * - **Right identity law**
	 *   \code
	 *     m >>= pure<T>    <=> m
	 *   \endcode
	 * - **Law of associativity**
	 *   \code
	 *     (m >>= f) >>= g) <=> m >>= ([f](X x){return f(x);} >>= g)
	 *   \endcode
	 *
	 * \see \ref monad (module)
	 */

	/**
	 * \defgroup monad Monad
	 *
	 * \ref monadpg concept and related functions.
	 *
	 * \code
	 *   #include <ftl/monad.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref applicative
	 */

	/**
	 * \interface monad
	 *
	 * Concrete definition of the monad concept.
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct monad {

		/**
		 * The type `M` is a monad on.
		 *
		 * For example, in the case of `maybe<int>`, `T = int`.
		 */
		using T = concept_parameter<M>;

		/**
		 * Used for compile time checks.
		 *
		 * Implementors _must_ override this default.
		 */
		static constexpr bool instance = false;

// Below ifdef section is to make sure the compiler disregards these
// definitions, while allowing a doc generator to find them and generate the
// proper documentation for the monad concept.
#ifdef SILLY_WORKAROUND

		/**
		 * Encapsulate a "pure" value.
		 *
		 * Given a plain value, encapsulate it in the monad M.
		 *
		 * \see applicative::pure
		 */
		static M pure(const T&);

		/**
		 * Map a function to a contextualised value.
		 *
		 * \see functor::map
		 */
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type,
				typename Mu = typename re_parametrise<M,U>::type
		>
		static Mu map(F&& f, const M& m);

		/**
		 * Bind a value and execute a computation in M on it.
		 *
		 * The bind operation is the basic operation used to sequence monadic
		 * computations. In essence, you can say that the left hand side is
		 * computed first (the definition of "computed" in this case depending
		 * on the particular instance of monad used), whereafter its result is
		 * "unwrapped" and fed to `f`, which in turn produces a new monadic
		 * computation. This second computation is returned, but never "run"
		 * (to keep it in the context of `M` and to allow further sequencing).
		 *
		 * Instances are free to provide `bind` using move semantics on `M`,
		 * either in addition to `const` reference version, or instead of.
		 *
		 * \tparam F must satisfy \ref fn`<M<U>(T)>` where `M` refers to the
		 *           templated type parametrised by `T`.
		 */
		template<
				typename F,
				typename Mu = typename decayed_result<F(T)>::type,
				typename = typename std::enable_if<std::is_same<
					typename re_parametrise<Mu,T>::type,
					M
				>::value>::type
		>
		static Mu bind(const M& m, F&& f);
#endif
	};

	/**
	 * Convenience operator for monad::bind.
	 *
	 * Basically makes monadic code a lot cleaner.
	 *
	 * \ingroup monad
	 */
	template<
			typename M,
			typename F,
			typename = typename std::enable_if<monad<M>::instance>::type
	>
	auto operator>>= (const M& m, F&& f)
	-> decltype(monad<M>::bind(m, std::forward<F>(f))) {
		return monad<M>::bind(m, std::forward<F>(f));
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
			typename M,
			typename F,
			typename M_ = plain_type<M>,
			typename = typename std::enable_if<monad<M_>::instance>::type
	>
	auto operator>>= (M&& m, F&& f)
	-> decltype(monad<M_>::bind(std::move(m), std::forward<F>(f))) {
		return monad<M_>::bind(std::move(m), std::forward<F>(f));
	}

	/**
	 * Convenience operator for monad::bind.
	 *
	 * Mirror of operator >>=
	 *
	 * \ingroup monad
	 */
	template<
			typename M,
			typename F,
			typename M_ = plain_type<M>,
			typename = typename std::enable_if<monad<M>::instance>::type
	>
	auto operator<<= (F&& f, M&& m)
	-> decltype(monad<M_>::bind(std::forward<M_>(m), std::forward<F>(f))) {
		return monad<M_>::bind(std::forward<M_>(m), std::forward<F>(f));
	}

	/**
	 * Perform two monadic computations, discard result of first.
	 *
	 * Using this operator to chain monadic computations is often times more
	 * desirable than running them in separate statements, because whatever
	 * operations \c M hides in its bind operation are still performed this way
	 * (in other words, nothing:s propagate down the chain etc).
	 *
	 * \ingroup monad
	 */
	template<
			typename Mt_,
			typename Mu_,
			typename Mt = plain_type<Mt_>,
			typename Mu = plain_type<Mu_>,
			typename T = concept_parameter<Mt>,
			typename = typename std::enable_if<monad<Mt>::instance>::type,
			typename = typename std::enable_if<
				std::is_same<typename re_parametrise<Mu,T>::type, Mt>::valueA
			>::type
	>
	Mu operator>> (Mt_&& m1, Mu_&& m2) {
		return monad<Mt_>::bind(std::forward<Mt>(m1), [m2](T&&) {
			return m2;
		});
	}

	/**
	 * Sequence two monadic computations, return the first.
	 *
	 * This operator is used to perform the computations \c m1 and \c m2
	 * in left-to-right order, and then return the result of \c m1.
	 * 
	 * Use case is when we have two computations that must be done in sequence,
	 * but it's only the first one that yields an interesting result. Most
	 * likely, the second one is only needed for a side effect of some kind.
	 *
	 * \ingroup monad
	 */
	template<
			typename Mt_,
			typename Mu,
			typename Mt = plain_type<Mt_>,
			typename T = concept_parameter<Mt>,
			typename U = concept_parameter<Mu>,
			typename = typename std::enable_if<monad<Mt>::instance>::type,
			typename = typename std::enable_if<
				std::is_same<typename re_parametrise<Mu,T>::type, Mt>::value
			>::type
	>
	Mt operator<< (Mt_&& m1, Mu m2) {
		return monad<Mt>::bind(std::forward<Mt>(m1), [m2](T t) {
			return monad<Mu>::bind(m2, [t](U&&) {
				return monad<Mt>::pure(t);
			});
		});
	}

	/**
	 * Lifts a function into M.
	 *
	 * The function f is lifted into the monadic computation M. Or in other
	 * words, the value M<A> is unwrapped, passed to f, and its result
	 * rewrapped.
	 *
	 * \ingroup monad
	 */
	template<
			typename Mt_,
			typename F,
			typename Mt = plain_type<Mt_>,
			typename T = concept_parameter<Mt>,
			typename = typename std::enable_if<monad<Mt>::instance>::type,
			typename U = typename decayed_result<F(T)>::type,
			typename Mu = typename re_parametrise<Mt,U>::type
	>
	Mu liftM(F f, Mt_&& m) {
		return std::forward<Mt>(m) >>= [f] (T&& t) {
			return monad<Mu>::pure(f(std::forward<T>(t)));
		};
	}

	/**
	 * Apply a function in M to a value in M.
	 *
	 * This is actually exactly equivalent to applicative<M>::apply.
	 *
	 * \ingroup monad
	 */
	template<
			typename Mt,
			typename Mf,
			typename T = concept_parameter<Mt>,
			typename F = concept_parameter<Mf>,
			typename U = typename decayed_result<F(T)>::type,
			typename Mu = typename re_parametrise<Mt,U>::type
	>
	Mu ap(const Mf& f, Mt m) {
		return f >>= [m] (F fn) {
			return m >>= [fn] (const T& t) {
				return monad<Mu>::pure(fn(t));
			};
		};
	}

	template<
			typename Mt,
			typename Mf,
			typename T = concept_parameter<Mt>,
			typename F = concept_parameter<Mf>,
			typename U = typename decayed_result<F(T)>::type,
			typename Mu = typename re_parametrise<Mt,U>::type
	>
	Mu ap(Mf&& f, Mt m) {
		return std::move(f) >>= [m] (F fn) {
			return m >>= [fn] (const T& t) {
				return monad<Mu>::pure(fn(t));
			};
		};
	}

}

#endif

