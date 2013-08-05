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

#include "../prelude.h"
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
	 * To be considered an instance of Monad, the interface `ftl::monad` must
	 * be specialised for the type in question.
	 *
	 * \see \ref monad (module)
	 */

	/**
	 * \defgroup monad Monad
	 *
	 * \ref monadpg concept and related functions.
	 *
	 * \code
	 *   #include <ftl/concepts/monad.h>
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
		 * Used for compile time checks.
		 *
		 * Implementors _must_ override this default.
		 */
		static constexpr bool instance = false;

// Below ifdef section is to make sure the compiler disregards these
// definitions, while allowing a doc generator to find them and generate the
// proper documentation for the monad concept.
#ifdef DOCUMENTATION_GENERATOR
		/**
		 * The type `M` is a monad on.
		 *
		 * For example, in the case of `maybe<int>`, `T = int`.
		 */
		using T = concept_parameter<M>;

		/**
		 * Convenient means of re parametrising M
		 */
		template<typename U>
		using M_ = typename re_parametrise<M,U>::type;

		/**
		 * Encapsulate a "pure" value.
		 *
		 * Given a plain value, encapsulate it in the monad M.
		 *
		 * \note All monad instances must define `pure`, it can not be derived.
		 *
		 * \see applicative::pure
		 */
		static M pure(const T&);

		/**
		 * Map a function to a contextualised value.
		 *
		 * \see functor::map, deriving_map
		 */
		template<
				typename F,
				typename U = result_of<F(T)>
		>
		static M_<U> map(F&& f, const M& m);

		/**
		 * Applies an encapsulated function to an encapsulated value.
		 *
		 * \see applicative::apply
		 */
		template<
				typename Mf,
				typename F = concept_parameter<Mf>,
				typename U = result_of<F(T)>,
				typename = typename std::enable_if<
					std::is_same<M_<F>,Mf>::value
				>::type
		>
		static M_<U> apply(const Mf& fn, const M& m);

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
		 * \tparam F must satisfy \ref fn`<M_<U>(T)>`
		 */
		template<
				typename F,
				typename U = concept_parameter<result_of<F(T)>>
		>
		static M_<U> bind(const M& m, F&& f);


		/**
		 * Joins (or "flattens") a nested instance of `M`.
		 *
		 * This function is easy to gain an intuition for; it corresponds to
		 * e.g. making a list of lists into a plain old list by simply
		 * concatenating all the inner lists into a single long one.
		 */
		static M join(const M_<M>& m);
#endif
	};

	/**
	 * Concepts lite-compatible check for monad instances.
	 *
	 * \ingroup monad
	 */
	template<typename M>
	constexpr bool Monad() noexcept {
		return monad<M>::instance;
	}

	/**
	 * Inheritable implementation of `monad<M>::join`.
	 *
	 * Monad implementations (as in, the template specialisations of monad<M>)
	 * may inherit this struct to get a default implementation of `join`, in
	 * terms of `bind`.
	 *
	 * \tparam M the template specialisation that is to derive `join`.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   struct monad<Identity<T>> : deriving_join<Identity<T>> {
	 *       // Implementations of bind, map, and pure
	 *   };
	 * \endcode
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct deriving_join {
		using T = concept_parameter<M>;
		
		template<typename U>
		using M_ = typename re_parametrise<M,U>::type;

		static M_<T> join(const M_<M_<T>>& m) {
			return monad<M>::bind(m, id);
		}

		static M_<T> join(M_<M_<T>>&& m) {
			return monad<M>::bind(std::move(m), id);
		}
	};

	/**
	 * Inheritable implementation of `monad<M>::map`.
	 *
	 * Implementations of `monad<M>` may inherit this struct to have a default
	 * implementation of `map` included. The default might not be the most
	 * performant version possible of `map`.
	 *
	 * \tparam M the monad specialisation that is to derive `map`
	 *
	 * \note Requires that the inheriting monad specialisation implements `bind`
	 *       and `pure`.
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct deriving_map {
		using T = concept_parameter<M>;
		
		template<typename U>
		using M_ = typename re_parametrise<M,U>::type;

		template<typename F, typename U = result_of<F(T)>>
		static M_<U> map(F f, const M_<T>& m) {
			return monad<M>::bind(
				m,
				[f](const T& t){ return monad<M_<U>>::pure(f(t)); }
			);
		}

		template<typename F, typename U = result_of<F(T)>>
		static M_<U> map(F f, M_<T>&& m) {
			return monad<M>::bind(
				std::move(m),
				[f](T&& t){ return monad<M_<U>>::pure(f(std::move(t))); }
			);
		}
	};

	/**
	 * Inheritable implementation of `monad::bind`.
	 *
	 * Monad specialisations (not the types that are monads themselves,
	 * their specialisations of the monad<M> struct) may inherit this struct
	 * to get a default implementatin of `bind`, in terms of `join` and `map`.
	 *
	 * \tparam M The same type monad<M> is being specialised for.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   struct monad<my_type<T>> : deriving_bind<my_type<T>> {
	 *       // Implementation of join and map
	 *   };
	 * \endcode
	 *
	 * \note To use this `deriving` implementation, there _must_ be an
	 *       actual implementation of `monad::join` and `monad::map`, you
	 *       cannot rely on `deriving` for either.
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct deriving_bind {
		using T = concept_parameter<M>;

		template<typename U>
		using M_ = typename re_parametrise<M,U>::type;

		template<
				typename F,
				typename U = concept_parameter<result_of<F(T)>>
		>
		static M_<U> bind(const M_<T>& m, F&& f) {
			return monad<M_<U>>::join(
				monad<M_<T>>::map(std::forward<F>(f), m)
			);
		}

		template<
				typename F,
				typename U = concept_parameter<result_of<F(T)>>
		>
		static M_<U> bind(M_<T>&& m, F&& f) {
			return monad<M_<U>>::join(
				monad<M_<T>>::map(std::forward<F>(f), std::move(m))
			);
		}
	};

	/**
	 * Inheritable implementation of `monad::apply`.
	 *
	 * This inheritable `apply` implementation is given in terms of `bind` and
	 * `pure`. Thus, `bind` cannot itself be derived when deriving from this
	 * struct.
	 *
	 * \tparam M The same type monad<M> is being specialised for.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   struct monad<my_type<T>> : deriving_apply<my_type<T>> {
	 *       // Implementation of pure and bind
	 *   };
	 * \endcode
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct deriving_apply {
		using T = concept_parameter<M>;

		template<typename U>
		using M_ = typename re_parametrise<M,U>::type;

		template<
				typename Mf,
				typename Mf_ = plain_type<Mf>,
				typename F = concept_parameter<Mf_>,
				typename U = result_of<F(T)>
		>
		static M_<U> apply(Mf&& f, M m) {
			return monad<Mf_>::bind(
				std::forward<Mf>(f),
				[m] (F fn) {
					return monad<M>::bind(
						m,
						[fn] (const T& t) { return monad<M_<U>>::pure(fn(t)); }
					);
				}
			);
		}

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
			typename M_ = plain_type<M>,
			typename = typename std::enable_if<Monad<M_>()>::type
	>
	auto operator>>= (M&& m, F&& f)
	-> decltype(monad<M_>::bind(std::forward<M>(m), std::forward<F>(f))) {
		return monad<M_>::bind(std::forward<M>(m), std::forward<F>(f));
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
			typename = typename std::enable_if<Monad<M_>()>::type
	>
	auto operator<<= (F&& f, M&& m)
	-> decltype(monad<M_>::bind(std::forward<M>(m), std::forward<F>(f))) {
		return monad<M_>::bind(std::forward<M>(m), std::forward<F>(f));
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
			typename Mt,
			typename Mu,
			typename Mt_ = plain_type<Mt>,
			typename T = concept_parameter<Mt_>,
			typename = typename std::enable_if<Monad<Mt_>()>::type,
			typename = typename std::enable_if<
				std::is_same<typename re_parametrise<Mu,T>::type, Mt_>::value
			>::type
	>
	Mu operator>> (Mt&& m1, const Mu& m2) {
		return monad<Mt_>::bind(std::forward<Mt>(m1), [m2](const T&) {
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
			typename Mt,
			typename Mu,
			typename = typename std::enable_if<Monad<Mt>()>::type,
			typename T = concept_parameter<Mt>,
			typename U = concept_parameter<Mu>,
			typename = typename std::enable_if<
				std::is_same<typename re_parametrise<Mu,T>::type, Mt>::value
			>::type
	>
	Mt operator<< (const Mt& m1, Mu m2) {
		return monad<Mt>::bind(m1, [m2](T t) {
			return monad<Mu>::bind(m2, [t](const U&) {
				return monad<Mt>::pure(t);
			});
		});
	}

	template<
			typename Mt,
			typename Mu,
			typename = typename std::enable_if<Monad<Mt>()>::type,
			typename T = concept_parameter<Mt>,
			typename U = concept_parameter<Mu>,
			typename = typename std::enable_if<
				std::is_same<typename re_parametrise<Mu,T>::type, Mt>::value
			>::type
	>
	Mt operator<< (Mt&& m1, Mu m2) {
		return monad<Mt>::bind(m1, [m2](T t) {
			return monad<Mu>::bind(m2, [t](const U&) {
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
			typename Mt,
			typename F,
			typename T = concept_parameter<Mt>,
			typename = typename std::enable_if<Monad<Mt>()>::type,
			typename U = result_of<F(T)>,
			typename Mu = typename re_parametrise<Mt,U>::type
	>
	Mu liftM(F f, const Mt& m) {
		return m >>= [f] (const T& t) {
			return monad<Mu>::pure(f(t));
		};
	}

	/**
	 * Convenience function object.
	 *
	 * Provided to make it easier to treat monadic bind as a first class
	 * function, even though many/all monad instances have overloaded
	 * versions. I.e., in many cases where one might want to pass monad::bind
	 * as a parameter to a function, it is not possible due to ambiguous
	 * overloads. In these cases, one could either use a lambda, or&mdash;more
	 * concisely&mdash;this function object.
	 *
	 * \ingroup monad
	 */
	struct mBind {
		template<typename M, typename F, typename M_ = plain_type<M>>
		auto operator() (M&& m, F&& f) const
		-> decltype(monad<M_>::bind(std::forward<M>(m), std::forward<F>(f))) {
			return monad<M_>::bind(std::forward<M>(m), std::forward<F>(f));
		}
	};

	/**
	 * Compile time instance of mBind.
	 *
	 * Makes higher order passing of monad<T>::bind even more convenient.
	 *
	 * \ingroup monad
	 */
	constexpr mBind mbind{};

}

#endif

