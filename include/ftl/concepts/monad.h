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
	 * Concepts lite-compatible check for monad instances.
	 *
	 * Naturally, this predicate can already be used in conjunction with SFINAE
	 * to hide particular functions, methods, and classes/structs if a template
	 * parameter is not a Monad.
	 *
	 * Example usage:
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
	 * Example:
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
	 * Inheritable implementation of `monad<M>::join`.
	 *
	 * Monad implementations (as in, the template specialisations of monad<M>)
	 * may inherit this struct to get a default implementation of `join`, in
	 * terms of `bind`.
	 *
	 * Both const reference and r-value reference versions are generated.
	 *
	 * \tparam M the template specialisation that is to derive `join`.
	 *
	 * Example:
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
	 * Inheritable implementation of `monad<M>::map`.
	 *
	 * Implementations of `monad<M>` may inherit this struct to have a default
	 * implementation of `map` included. The default might not be the most
	 * performant version possible of `map`.
	 *
	 * Both const reference and r-value reference versions are generated.
	 *
	 * \tparam M the monad specialisation that is to derive `map`
	 *
	 * Example:
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
	 * \tparam M The same type monad<M> is being specialised for.
	 *
	 * Example:
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
	 * Note that this `monad::bind` implementation is done in terms of
	 * `functor::map`, hence there must be a user defined implementation of
	 * that.
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
		 * Example:
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
				typename U = Value_type<Cu>,
				typename = Requires<ForwardIterable<Cu>()>
		>
		static M<U> bind(const M<T>& m, F&& f) {
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
				typename U = Value_type<Cu>,
				typename = Requires<
					ForwardIterable<Cu>()
				>
		>
		static M<U> bind(M<T>&& m, F&& f) {
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
	 * \tparam M The same type monad<M> is being specialised for.
	 *
	 * Example:
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
	 * Example:
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
	 * Convenience operator for monad::bind.
	 *
	 * The purpose is to make it more easy to read and write monadic code. All
	 * arguments are perfectly forwarded, for cases where it matters if the
	 * const reference or r-value reference version of `bind` is triggered.
	 *
	 * Example usage:
	 * \code
	 *   MyMonad<int> foo(float);
	 *
	 *   MyMonad<int> bar(const MyMonad<float>& m) {
	 *       using ftl::operator>>=;
	 *       return m >>= foo;
	 *       
	 *       // Equivalent code without using operator:
	 *       // return ftl::monad<MyMonad<float>>::bind(m, foo);
	 *   }
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
	 *
	 * In other words, `nothing` propagates down the sequence in the case of
	 * `maybe` (and the same goes for left values with `either`), and so on.
	 *
	 * Example usage:
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
	 * This operator is used to perform the computations `m1` and `m2`
	 * in left-to-right order, and then return the result of `m1`.
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

	/**
	 * Lifts a function into M.
	 *
	 * The function f is lifted into the monadic computation M. Or in other
	 * words, the value M<A> is unwrapped, passed to `f`, and its result
	 * rewrapped.
	 *
	 * \ingroup monad
	 */
	template<
			typename Mt,
			typename F,
			typename T = Value_type<Mt>,
			typename = Requires<Monad<Mt>()>,
			typename U = result_of<F(T)>,
			typename Mu = Rebind<Mt,U>
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
	 * versions. I.e., in many cases where one might want to pass `monad::bind`
	 * as a parameter to a function, it is not possible due to ambiguous
	 * overloads. In these cases, one could either use a lambda, or&mdash;more
	 * concisely&mdash;this function object.
	 *
	 * \ingroup monad
	 */
	struct mBind
#ifndef DOCUMENTATION_GENERATOR
	: private _dtl::curried_binf<mBind>
#endif
	{
		template<typename M, typename F, typename M_ = plain_type<M>>
		auto operator() (M&& m, F&& f) const
		-> decltype(monad<M_>::bind(std::forward<M>(m), std::forward<F>(f))) {
			return monad<M_>::bind(std::forward<M>(m), std::forward<F>(f));
		}

		using curried_binf<mBind>::operator();
	};

	/**
	 * Compile time instance of mBind.
	 *
	 * Makes higher order passing of monad<T>::bind even more convenient.
	 *
	 * Example usage:
	 * \code
	 *   void foo() {
	 *      bar(mbind);
	 *   }
	 *
	 *   // Alternative, less concise option:
	 *   template<typename M, typename F>
	 *   void baz() {
	 *       bar([](const M& m, const F& f){ return m >>= f; });
	 *   }
	 * \endcode
	 *
	 * \ingroup monad
	 */
	constexpr mBind mbind{};

	/**
	 * Convenience function object.
	 *
	 * Provided to make it easier to treat monadic join as a first class
	 * function, even though many/all monad instances have overloaded
	 * versions. I.e., in many cases where one might want to pass `monad::join`
	 * as a parameter to a function, it is not possible due to ambiguous
	 * overloads. In these cases, one could either use a lambda, or&mdash;more
	 * concisely&mdash;this function object.
	 *
	 * \ingroup monad
	 */
	struct mJoin {
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
	};

	/**
	 * Compile time instance of mJind.
	 *
	 * Makes higher order passing of `monad<T>::join` even more convenient.
	 *
	 * Example usage:
	 * \code
	 *   void foo() {
	 *      bar(mjoin);
	 *   }
	 *
	 *   // Alternative, less concise option:
	 *   template<typename M>
	 *   void baz() {
	 *       bar([](const M& m){ return monad<M>::join(m); });
	 *   }
	 * \endcode
	 *
	 * \ingroup monad
	 */
	constexpr mJoin mjoin{};
}

#endif

