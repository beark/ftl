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
	 * - Left identity law:
	 *   \code
	 *     pure(x) >>= f    <=> f(x)
	 *   \endcode
	 * - Right identity law:
	 *   \code
	 *     m >>= pure<T>    <=> m
	 *   \endcode
	 * - Law of associativity:
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
	 * - \ref prelude
	 * - \ref applicative
	 */

	/**
	 * \interface monad
	 *
	 * Concrete definition of the monad concept.
	 *
	 * \par Writing A New Instance
	 * To create a monad implementation for a user defines type, you need to
	 * specialise the `struct` that defines this interface. In general, this
	 * will look something akin to:
	 *
	 * \code
	 *   template<typename T>
	 *   struct monad<MyType<T>> {
	 *       // Implementation goes here
	 *   };
	 * \endcode
	 *
	 * For details about the various monadic operations that should be part of
	 * the implementation, simply refer to the rest of the documentation of this
	 * interface.
	 *
	 * \ingroup monad
	 */
	template<typename M_>
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
		 * Provides some sugar for referring to differently parametrised `M_`s.
		 *
		 * This results in much cleaner type signatures.
		 */
		template<typename U>
		using M = Rebind<M_,U>;

		/**
		 * The type `M` is a monad on.
		 *
		 * For example, in the case of `maybe<int>`, `T = int`. In the general
		 * case, `T` depends on how (if at all) `M` specialises the
		 * `parametric_traits` struct.
		 */
		using T = Value_type<M_>;

		/**
		 * Encapsulate a "pure" value.
		 *
		 * Given a plain value, encapsulate it in the monad `M`.
		 *
		 * \note All monad instances must define `pure`, it can not be derived.
		 *
		 * \see applicative::pure
		 */
		static M<T> pure(const T&);

		/**
		 * Map a function to a contextualised value.
		 *
		 * Depending on how you decide to view a monad, `map` could be
		 * interpreted to mean e.g. either of:
		 * - Apply `f` to every element contained in `m` and return a new
		 *   container of all the results.
		 * - Return a computation that is the result of running the computation
		 *   `m` and then applying `f` to that result.
		 *
		 * \see functor::map, deriving_map
		 */
		template<typename F, typename U = result_of<F(T)>>
		static M<U> map(F&& f, const M<T>& m);

		/**
		 * Applies an encapsulated function to an encapsulated value.
		 *
		 * Somewhat similar to `map`, except in this case the function `f` is
		 * also wrapped in a monadic context. I.e., in the case of the list
		 * monad, `fn` would be a list of functions; in the case of
		 * `ftl::maybe`, it would _maybe_ be a function; and so on.
		 *
		 * \see applicative::apply
		 */
		template<typename F, typename U = result_of<F(T)>>
		static M<U> apply(const M<F>& fn, const M<T>& m);

		/**
		 * Bind a value and execute a computation in `M` on it.
		 *
		 * The bind operation is the basic operation used to sequence monadic
		 * computations. In essence, you can say that the left hand side is
		 * computed first (the definition of "computed" in this case depending
		 * on the particular instance of monad used), whereafter its result is
		 * "unwrapped" and fed to `f`, which in turn produces a new monadic
		 * computation. This second computation is returned, but not "run"
		 * (to keep it in the context of `M` and to allow further sequencing).
		 *
		 * Instances are free to provide `bind` using move semantics on `M`,
		 * either in addition to `const` reference version, or instead of.
		 *
		 * \tparam F must satisfy \ref fn`<M<U>(T)>`
		 */
		template<typename F, typename U = Value_type<result_of<F(T)>>>
		static M<U> bind(const M<T>& m, F&& f);


		/**
		 * Joins (or "flattens") a nested instance of `M`.
		 *
		 * This function is easy to gain an intuition for; it corresponds to
		 * e.g. making a list of lists into a plain old list by simply
		 * concatenating all the inner lists into a single long one.
		 */
		static M<T> join(const M<M<T>>& m);
#endif
	};

	/**
	 * Predicate to check whether a type is an instance of Monad.
	 *
	 * Naturally, this predicate can already be used in conjunction with SFINAE
	 * to hide particular functions, methods, and classes/structs if a template
	 * parameter is not a Monad.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<
	 *       typename M,
	 *       typename = Requires<Monad<M>()>
	 *   >
	 *   void myFunc(const M& m) {
	 *       // Use bind, join, apply, etc on m
	 *   }
	 * \endcode 
	 *
	 * \ingroup monad
	 */
	template<typename M>
	constexpr bool Monad() noexcept {
		return monad<M>::instance;
	}

	/**
	 * Tag that can be used to specify which concept implementation to derive.
	 *
	 * This particular tag is used to specify that some monad method should be
	 * derived in terms of `monad::bind`. Thus, there must be an existing
	 * implementation of `bind` that does not rely on whatever method is being
	 * derived in terms of it.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct monad<AType<T>>
	 *   : deriving_map<in_terms_of_bind<AType<T>> {
	 *       // Bind must be here, or else independently derived
	 *
	 *       // Probably other method implementations
	 *   };
	 * \endcode
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct in_terms_of_bind {};

	template<typename>
	struct deriving_join;

	/**
	 * Inheritable implementation of `monad::join`.
	 *
	 * Monad implementations (as in, the template specialisations of `monad<>`)
	 * may inherit this struct to get a default implementation of `join`, in
	 * terms of `bind`.
	 *
	 * Both const reference and r-value reference versions are generated.
	 *
	 * \tparam M the template specialisation that is to derive `join`.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct monad<MyMonad<T>>
	 *   : deriving_join<in_terms_of_bind<MyMonad<T>>> {
	 *       // Implementations of bind, map, apply, and pure
	 *   };
	 * \endcode
	 *
	 * \note Requires that the inheriting monad specialisation implements
	 *       `bind`. All of the other monadic operations may be derived.
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct deriving_join<in_terms_of_bind<M>> {
		using T = Value_type<M>;
		
		template<typename U>
		using M_ = Rebind<M,U>;

		static M_<T> join(const M_<M_<T>>& m) {
			return monad<M_<M_<T>>>::bind(m, id);
		}

		static M_<T> join(M_<M_<T>>&& m) {
			return monad<M_<M_<T>>>::bind(std::move(m), id);
		}
	};

	/**
	 * Inheritable implementation of `monad::map`.
	 *
	 * Implementations of `monad<>` may inherit this struct to have a default
	 * implementation of `map` included. The default might not be the most
	 * performant version possible of `map`.
	 *
	 * Both const reference and r-value reference versions are generated.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct monad<MyMonad<T>>
	 *   : deriving_map<in_terms_of_bind<MyMonad<T>>> {
	 *       // Implementations of bind, pure, apply, and join
	 *   };
	 * \endcode
	 *
	 * \note `M` must not have its `bind` operation implemented in terms of
	 *       `map` and `join`.
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct deriving_map<in_terms_of_bind<M>> {
		using T = Value_type<M>;
		
		template<typename U>
		using M_ = Rebind<M,U>;

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
	 * Both const reference and r-value reference versions are generated.
	 *
	 * This _deriving_ construct is specialised for
	 * `ftl::back_insertable_container`.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct monad<MyMonad<T>> : deriving_bind<MyMonad<T>> {
	 *       // Implementation of join and map
	 *   };
	 * \endcode
	 *
	 * \note To use this `deriving` implementation, there must be an
	 *       actual implementation of `monad::join` and `monad::map`, you
	 *       cannot rely on `deriving` for either. The other monadic operations
	 *       may be derived if desired, however.
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct deriving_bind {
		using T = Value_type<M>;

		template<typename U>
		using M_ = Rebind<M,U>;

		template<
				typename F,
				typename U = Value_type<result_of<F(T)>>
		>
		static M_<U> bind(const M_<T>& m, F&& f) {
			return monad<M_<U>>::join(
				monad<M_<T>>::map(std::forward<F>(f), m)
			);
		}

		template<
				typename F,
				typename U = Value_type<result_of<F(T)>>
		>
		static M_<U> bind(M_<T>&& m, F&& f) {
			return monad<M_<U>>::join(
				monad<M_<T>>::map(std::forward<F>(f), std::move(m))
			);
		}
	};

	/**
	 * Inheritable `bind` implementation for containers supporting `insert`.
	 *
	 * The following requirements must be met by `M_`:
	 * - There must exist an `M::insert(iterator pos, const T& value)`,
	 *   returning an iterator to the inserted element. An R-value version is
	 *   also beneficial in some cases.
	 * - There must be an implementation of `monad::map` available for `M_`
	 *   that is _not_ derived in terms of `bind`.
	 *
	 * Also note that types using this construct to generate a `bind`
	 * implementation will be capable of binding with any function returning
	 * a \ref fwditerable, regardless of _which_ ForwardIterable that is.
	 * I.e., in the example below, you could bind `MyContainer` with a function
	 * returning e.g. `maybe<T>`, `std::vector<T>`, etc.
	 *
	 * Example:
	 * \code
	 *   namespace ftl {
	 *       template<typename T>
	 *       struct monad<MyContainer<T>>
	 *       : deriving_bind<back_insertable_container<MyContainer<T>> {
	 *           // Implementations of pure, map, join, apply
	 *       };
	 *   }
	 * \endcode
	 *
	 * \see ftl::deriving_bind
	 *
	 * \ingroup monad
	 */
	template<typename M_>
	struct deriving_bind<back_insertable_container<M_>> {

		/// Type alias for cleaner type signatures.
		using T = Value_type<M_>;

		/// Another type alias to get more easily read type signatures.
		template<typename U>
		using M = Rebind<M_,U>;

		/**
		 * This `bind` version follows the standard container theme of modelling
		 * non-deterministic computations. In other words, `m` is viewed as a
		 * collection of _possible_ values, each of which `f' will generate a
		 * new set of possible answers for.
		 *
		 * \par Examples
		 *
		 * \code
		 *   SomeCollection<int> c{2,4,5};
		 *   auto c2 = c >>= [](int x){ return SomeCollection<int>{x/2, 2*x}; };
		 *
		 *   // Note how each group of two results correspond to one input value
		 *   // c2 == SomeCollection<int>{1,4, 2,8, 2,10}
		 * \endcode
		 */
		template<
				typename F,
				typename Cu = result_of<F(T)>,
				typename U = Value_type<Cu>
		>
		static M<U> bind(const M<T>& m, F&& f) {
			static_assert(
				DefaultConstructible<M<U>>(),
				"Rebind<M_,U> must result in a DefaultConstructible type"
			);

			static_assert(
				ForwardIterable<Cu>(),
				"F(T) does not return an instance of ForwardIterable"
			);

			auto m2 = std::forward<F>(f) % m;
			M<U> result;
			auto it = std::inserter(result, result.begin());
			for(auto& c : m2) {
				it = std::move(c.begin(), c.end(), it);
			}

			return result;
		}

		/// \overload
		template<
				typename F,
				typename Cu = result_of<F(T)>,
				typename U = Value_type<Cu>
		>
		static M<U> bind(M<T>&& m, F&& f) {
			static_assert(
				DefaultConstructible<M<U>>(),
				"Rebind<M_,U> must result in a DefaultConstructible type"
			);

			static_assert(
				ForwardIterable<Cu>(),
				"F(T) does not return an instance of ForwardIterable"
			);

			auto m2 = std::forward<F>(f) % std::move(m);
			M<U> result;
			auto it = std::inserter(result, result.begin());
			for(auto& c : m2) {
				it = std::move(c.begin(), c.end(), it);
			}

			return result;
		}

	};

	template<typename M>
	struct deriving_apply {};

	/**
	 * Inheritable implementation of `monad::apply`.
	 *
	 * The derived implementation will be in terms of `bind` and `pure`. Hence,
	 * these must be implemented by some means.
	 *
	 * Both const reference and r-value reference versions are generated.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct monad<my_type<T>>
	 *   : deriving_apply<in_terms_of_bind<my_type<T>>> {
	 *       // Implementation of pure and bind
	 *   };
	 * \endcode
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct deriving_apply<in_terms_of_bind<M>> {
		using T = Value_type<M>;

		template<typename U>
		using M_ = Rebind<M,U>;

		template<
				typename Mf,
				typename Mf_ = plain_type<Mf>,
				typename F = Value_type<Mf_>,
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

	template<typename M>
	struct deriving_monad;

	/**
	 * Inheritable monad implementation for many containers.
	 *
	 * Requires the following out of `M` to be valid:
	 * - Must obviously be \ref fwditerable
	 * - There must exist an `M::insert(iterator pos, const T& value)`,
	 *   returning an iterator to the inserted element. An R-value version is
	 *   also beneficial in some cases.
	 * - `ftl::deriving_pure` must be applicable to `M`.
	 *
	 * Any container fulfilling these requirements can have its monad
	 * implementation almost automatically, by letting it derive from this
	 * implementation.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct monad<Container<T>>
	 *   : deriving_monad<back_insertable_container<Container<T>>> {};
	 * \endcode
	 *
	 * \ingroup monad
	 */
	template<typename M>
	struct deriving_monad<back_insertable_container<M>>
	: deriving_pure<M>, deriving_map<back_insertable_container<M>>
	, deriving_bind<back_insertable_container<M>>
	, deriving_join<in_terms_of_bind<M>>
	, deriving_apply<in_terms_of_bind<M>> {

		static constexpr bool instance = true;
	};

	/**
	 * Convenience operator for `ftl::mbind`.
	 *
	 * The following substitution is always true:
	 * \code
	 *   a >>= b <=> ftl::mbind(a, b)
	 * \endcode
	 *
	 * The purpose is to make it more easy to read and write monadic code. All
	 * arguments are perfectly forwarded, for cases where it matters if the
	 * const reference or r-value reference version of `bind` is triggered.
	 *
	 * \par Examples
	 *
	 * \code
	 *   ftl::maybe<int> maybeGetInt();
	 *   ftl::maybe<float> mightFail(int);
	 *
	 *   ftl::maybe<float> example() {
	 *       return maybeGetInt() >>= mightFail;
	 *   }
	 * \endcode
	 *
	 * \note Unlike Haskell, `operator>>=` is _right_ associative in C++,
	 *       resulting in some convolution when chaining several together.
	 *
	 * \ingroup monad
	 */
	template<
			typename M,
			typename F,
			typename M_ = plain_type<M>,
			typename = Requires<Monad<M_>()>
	>
	auto operator>>= (M&& m, F&& f)
	-> decltype(monad<M_>::bind(std::forward<M>(m), std::forward<F>(f))) {
		return monad<M_>::bind(std::forward<M>(m), std::forward<F>(f));
	}

	/**
	 * Convenience operator for monad::bind.
	 *
	 * Mirror of `ftl::operator>>=`. The only difference is the order of the
	 * arguments.
	 *
	 * Unlike `operator>>=`, the fixity of this operator is in fact just as
	 * expected, causing it to sometimes be significantly more convenient.
	 *
	 * \par Examples
	 *
	 * Clean chaining:
	 * \code
	 *   auto m = ftl::value(2.f);
	 *   auto f = [](float x){ return x == 0 ? nothing : value(8.f/x); };
	 *
	 *   auto r = f <<= f <<= m;
	 *   // r == value(2.f), or 8 / (8 / 2)
	 * \endcode
	 *
	 * \ingroup monad
	 */
	template<
			typename M,
			typename F,
			typename M_ = plain_type<M>,
			typename = Requires<Monad<M_>()>
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
	 * operations `M` hides in its bind operation are still performed this way.
	 * For example, `ftl::nothing` propagates down the sequence in the case of
	 * `ftl::maybe` and so on.
	 *
	 * \par Examples
	 *
	 * Basic usage:
	 * \code
	 *   MyMonad<SomeType> foo();
	 *   MyMonad<OtherType> bar();
	 *
	 *   MyMonad<OtherType> example() {
	 *       using ftl::operator>>;
	 *       return foo() >> bar();
	 *
	 *       // Equivalent code using plain bind:
	 *       // return foo() >>= [](SomeType){ return bar(); };
	 *   }
	 * \endcode
	 *
	 * Side effects of bind are preserved:
	 * \code
	 *   auto e1 = ftl::make_left<int>(string("abc"));
	 *   auto e2 = ftl::make_right<string>(10);
	 *   auto e3 = ftl::make_right<string>(string("123"));
	 *
	 *   auto r1 = e1 >> e2; // r1 == make_left<int>("abc")
	 *   auto r2 = e2 >> e3; // r2 == make_right<string>("123")
	 *
	 * \endcode
	 *
	 * \ingroup monad
	 */
	template<
			typename Mt,
			typename Mu,
			typename Mt_ = plain_type<Mt>,
			typename T = Value_type<Mt_>,
			typename = Requires<Monad<Mt_>()>,
			typename = Requires<
				std::is_same<Rebind<Mu,T>, Mt_>::value
			>
	>
	Mu operator>> (Mt&& m1, const Mu& m2) {
		return monad<Mt_>::bind(std::forward<Mt>(m1), [m2](const T&) {
			return m2;
		});
	}

	/**
	 * Sequence two monadic computations, return the first.
	 *
	 * Unlike what might be exptected, this operator is used to perform the
	 * computations `m1` and `m2` in _left-to-right_ order, and then return the
	 * result of `m1`.
	 * 
	 * Use case is when we have two computations that must be done in sequence,
	 * but it's only the first one that yields an interesting result. Most
	 * likely, the second one is only needed for a side effect of some kind. In
	 * some sense, this is a workaround to the lack of do-notation.
	 *
	 * \par Examples
	 *
	 * \code
	 *   auto m1 = ftl::value(8);
	 *   auto m2 = ftl::value(4);
	 *   ftl::maybe<int> m3;
	 *
	 *   auto r1 = m1 << m2 // r1 == value(8)
	 *   auto r2 = m1 << m3 // r2 == nothing
	 * \endcode
	 *
	 * \ingroup monad
	 */
	template<
			typename Mt,
			typename Mu,
			typename = Requires<Monad<Mt>()>,
			typename T = Value_type<Mt>,
			typename U = Value_type<Mu>,
			typename = Requires<
				std::is_same<Rebind<Mu,T>, Mt>::value
			>
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
			typename = Requires<Monad<Mt>()>,
			typename T = Value_type<Mt>,
			typename U = Value_type<Mu>,
			typename = Requires<
				std::is_same<Rebind<Mu,T>, Mt>::value
			>
	>
	Mt operator<< (Mt&& m1, Mu m2) {
		return monad<Mt>::bind(m1, [m2](T t) {
			return monad<Mu>::bind(m2, [t](const U&) {
				return monad<Mt>::pure(t);
			});
		});
	}

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _mbind : private _dtl::curried_binf<_mbind> {
		template<typename M, typename F, typename M_ = plain_type<M>>
		auto operator() (M&& m, F&& f) const
		-> decltype(monad<M_>::bind(std::forward<M>(m), std::forward<F>(f))) {
			return monad<M_>::bind(std::forward<M>(m), std::forward<F>(f));
		}

		using curried_binf<_mbind>::operator();
	} mbind{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Curried function object representing `monad::bind`.
	 *
	 * Has a calling type equivalent to
	 * \code
	 *   (M<T>, (T) -> M<U>) -> M<U>
	 * \endcode
	 * where `M` is an instance of \ref monadpg.
	 *
	 * Makes for much more convenient calling and passing to higher-order
	 * functions.
	 *
	 * \par Examples
	 *
	 * Trivial usage to sequence two maybes:
	 * \code
	 *   ftl::maybe<int> foo(string s);
	 *
	 *   ftl::maybe<int> exampe(ftl::maybe<string> ms) {
	 *       return ftl::mbind(ms, foo);
	 *   }
	 * \endcode
	 *
	 * \ingroup monad
	 */
	mbind;
#endif

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _mjoin {
		template<
				typename M,
				typename = Requires<Monad<plain_type<M>>()>
		>
		auto operator() (M&& m) const
		-> decltype(
			monad<Value_type<plain_type<M>>>::join(std::forward<M>(m))
		) {
			return monad<Value_type<plain_type<M>>>::join(
				std::forward<M>(m)
			);
		}
	} mjoin {};
#else
	struct ImplementationDefined {
	}
	/**
	 * Function object representing `monad::join`.
	 *
	 * Functions as if it were a function of type
	 * \code
	 *   (M<M<T>>) -> M<T>
	 * \endcode
	 *
	 * Proves concise calling syntax for the monadic join operation, allowing
	 * cleaner code both for regular use and for passing to higher-order
	 * functions.
	 *
	 * \par Examples
	 *
	 * Straight forward use to flatten a list of lists. No elements are
	 * discarded, they will appear in the resulting list in the same order
	 * as if a depth-first traversal was made of the original list.
	 * \code
	 *   std::list<int> flatten(const list<list<int>>& l) {
	 *       return ftl::mjoin(l);
	 *   }
	 * \endcode
	 *
	 * \ingroup monad
	 */
	mjoin;
#endif
}

#endif

