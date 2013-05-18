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
	 * \brief Concrete definition of the monad concept.
	 *
	 * \note There are really two versions of this interface: this one,
	 * which is for types parameterised on more than the type they're a monad
	 * on, and one that has the appearance
	 * \code
	 *   template<template<typename> class M>
	 *   struct monad<M>;
	 * \endcode
	 * which is of course for types parameterised _only_ on the type they're a
	 * monad on.
	 *
	 * \ingroup monad
	 */
	template<template<typename...> class M>
	struct monad {

		/**
		 * Used for compile time checks.
		 *
		 * Implementors \em must override this default.
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
		template<typename A, typename...Ts>
		static M<A,Ts...> pure(const A&);

		/**
		 * Map a function to a contextualised value.
		 *
		 * \see functor::map
		 */
		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type,
			typename...Ts>
		static M<B,Ts...> map(F&& f, const M<A,Ts...>& m);

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
		 * In the single type parameter case, `bind` is declared as
		 * \code
		 *   static M<B> bind(const M<A>&, F&&);
		 * \endcode
		 *
		 * \tparam F must satisfy \ref fn<M<B>(A)>
		 */
		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type::value_type,
			typename...Ts>
		static M<B,Ts...> bind(const M<A,Ts...>& m, F&& f);
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
		typename F,
		template <typename...> class M,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename...Ts>
	auto operator>>= (const M<A,Ts...>& m, F&& f)
	-> decltype(monad<M>::bind(m, std::forward<F>(f))) {
		return monad<M>::bind(m, std::forward<F>(f));
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
		typename F,
		template <typename> class M,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type>
	auto operator>>= (const M<A>& m, F&& f)
	-> decltype(monad<M>::bind(m, std::forward<F>(f))) {
		return monad<M>::bind(m, std::forward<F>(f));
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
		typename F,
		template <typename...> class M,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename...Ts>
	auto operator>>= (M<A,Ts...>&& m, F&& f)
	-> decltype(monad<M>::bind(std::move(m), std::forward<F>(f))) {
		return monad<M>::bind(std::move(m), std::forward<F>(f));
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
		typename F,
		template <typename> class M,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type>
	auto operator>>= (M<A>&& m, F&& f)
	-> decltype(monad<M>::bind(std::move(m), std::forward<F>(f))) {
		return monad<M>::bind(std::move(m), std::forward<F>(f));
	}

	/**
	 * Convenience operator for monad::bind.
	 *
	 * Mirror of operator >>=
	 *
	 * \ingroup monad
	 */
	template<
		typename F,
		template <typename...> class M,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type::value_type,
		typename...Ts>
	auto operator<<= (F&& f, const M<A,Ts...>& m)
	-> decltype(monad<M>::bind(m, std::forward<F>(f))) {
		return monad<M>::bind(m, std::forward<F>(f));
	}

	/**
	 * \overload
	 * 
	 * \ingroup monad
	 */
	template<
		typename F,
		template <typename> class M,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type::value_type>
	auto operator<<= (F&& f, const M<A>& m)
	-> decltype(monad<M>::bind(m, std::forward<F>(f))) {
		return monad<M>::bind(m, std::forward<F>(f));
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
		template<typename...> class M,
		typename A,
		typename B,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename...Ts>
	M<B,Ts...> operator>> (const M<A,Ts...>& m1, M<B,Ts...> m2) {
		return monad<M>::bind(m1, [m2](A) {
			return m2;
		});
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	//TODO: Add version with lambda capture by move once available (C++14)
	template<
		template<typename...> class M,
		typename A,
		typename B,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename...Ts>
	M<B,Ts...> operator>> (M<A,Ts...>&& m1, M<B,Ts...> m2) {
		return monad<M>::bind(std::move(m1), [m2](A) {
			return m2;
		});
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
		template<typename> class M,
		typename A,
		typename B,
		typename = typename std::enable_if<monad<M>::instance>::type>
	M<B> operator>> (const M<A>& m1, M<B> m2) {
		return monad<M>::bind(m1, [m2](A) {
			return m2;
		});
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	//TODO: Add version with lambda capture by move once available (C++14)
	template<
		template<typename> class M,
		typename A,
		typename B,
		typename = typename std::enable_if<monad<M>::instance>::type>
	M<B> operator>> (M<A>&& m1, M<B> m2) {
		return monad<M>::bind(std::move(m1), [m2](A) {
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
		template<typename...> class M,
		typename A,
		typename B,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename...Ts>
	M<A,Ts...> operator<< (const M<A,Ts...>& m1, M<B,Ts...> m2) {
		return monad<M>::bind(m1, [m2](A a) {
			return monad<M>::bind(m2, [a](B) {
				return monad<M>::template pure<A,Ts...>(a);
			});
		});
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	// TODO: Lambda capture by move (C++14)
	template<
		template<typename...> class M,
		typename A,
		typename B,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename...Ts>
	M<A,Ts...> operator<< (M<A,Ts...>&& m1, M<B,Ts...> m2) {
		return monad<M>::bind(std::move(m1), [m2](A a) {
			return monad<M>::bind(m2, [a](B) {
				return monad<M>::template pure<A,Ts...>(a);
			});
		});
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
		template<typename> class M,
		typename A,
		typename B,
		typename = typename std::enable_if<monad<M>::instance>::type>
	M<A> operator<< (const M<A>& m1, M<B> m2) {
		return monad<M>::bind(m1, [m2](A a) {
			return monad<M>::bind(m2, [a](B) {
				return monad<M>::template pure<A>(a);
			});
		});
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	// TODO: Lambda capture by move (C++14)
	template<
		template<typename> class M,
		typename A,
		typename B,
		typename = typename std::enable_if<monad<M>::instance>::type>
	M<A> operator<< (M<A>&& m1, M<B> m2) {
		return monad<M>::bind(std::move(m1), [m2](A a) {
			return monad<M>::bind(m2, [a](B) {
				return monad<M>::template pure<A>(a);
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
		template<typename...> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename R = typename decayed_result<F(A)>::type,
		typename...Ts>
	M<R,Ts...> liftM(F f, const M<A,Ts...>& m) {
		return m >>= [f] (A a) {
			return monad<M>::template pure<A,Ts...>(f(std::forward<A>(a)));
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
		template<typename...> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename R = typename decayed_result<F(A)>::type,
		typename...Ts>
	M<R,Ts...> liftM(F f, M<A,Ts...>&& m) {
		return std::move(m) >>= [f] (A a) {
			return monad<M>::template pure<A,Ts...>(f(std::forward<A>(a)));
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
		template<typename> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename R = typename decayed_result<F(A)>::type>
	M<R> liftM(F f, const M<A>& m) {
		return m >>= [f] (A a) {
			return monad<M>::pure(f(std::forward<A>(a)));
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
		template<typename> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename R = typename decayed_result<F(A)>::type>
	M<R> liftM(F f, M<A>&& m) {
		return std::move(m) >>= [f] (A a) {
			return monad<M>::pure(f(std::forward<A>(a)));
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
		template<typename...> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type,
		typename...Ts>
	M<B,Ts...> ap(const M<F,Ts...>& f, M<A,Ts...> m) {
		return f >>= [m] (F f) {
			return m >>= [f] (A a) {
				return monad<M>::template pure<A,Ts...>(f(std::forward<A>(a)));
			};
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	// TODO: Catpure by move (C++14)
	template<
		template<typename...> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type,
		typename...Ts>
	M<B,Ts...> ap(M<F,Ts...>&& f, M<A,Ts...> m) {
		return std::move(f) >>= [m] (F f) {
			return m >>= [f] (A a) {
				return monad<M>::template pure<A,Ts...>(f(std::forward<A>(a)));
			};
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	template<
		template<typename> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type>
	M<B> ap(const M<F>& f, M<A> m) {
		return f >>= [m] (function<B,A> f) {
			return m >>= [f] (A a) {
				return monad<M>::pure(f(std::forward<A>(a)));
			};
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup monad
	 */
	// TODO: Catpure by move (C++14)
	template<
		template<typename> class M,
		typename F,
		typename A,
		typename = typename std::enable_if<monad<M>::instance>::type,
		typename B = typename decayed_result<F(A)>::type>
	M<B> ap(M<F>&& f, M<A> m) {
		return std::move(f) >>= [m] (function<B,A> f) {
			return m >>= [f] (A a) {
				return monad<M>::pure(f(std::forward<A>(a)));
			};
		};
	}
}

#endif

