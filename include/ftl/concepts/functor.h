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

#include "../type_functions.h"
#include "../function.h"
#include "../prelude.h"
#include "common.h"

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
	 *     map(id, t)          <=> t
	 *   \endcode
	 * - **Preservation of composition**
	 *   \code
	 *     map(compose(f, g), t)  <=> compose(curry(map)(f), curry(map)(g))(t)
	 *   \endcode
	 *
	 * For a detailed description of what exactly an instance needs to
	 * implement, see the ftl::functor interface.
	 *
	 * \see \ref functor (module)
	 */

	/**
	 * \defgroup functor Functor
	 *
	 * Module containg the \ref functorpg concept and related functions.
	 *
	 * \code
	 *   #include <ftl/concepts/functor.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref typelevel
	 * - <ftl/function.h>
	 */

	/**
	 * \interface functor
	 *
	 * Struct that must be specialised to implement the functor concept.
	 *
	 * When specialising, it is important to remember that the described
	 * interface here is the _default_, and an instance is encouraged to include
	 * additional overloads of the functions for optimization reasons and
	 * similar. For types that cannot be copied, for instance, it makes sense
	 * that all of the functor interface functions accept an rvalue reference.
	 *
	 * \ingroup functor
	 */
	template<typename F_>
	struct functor {
		/// Convenient access to the type `F_` is parametrised on.
		using T = Value_type<F_>;

		/**
		 * Clean way of referring to differently parametrised `F`s.
		 */
		template<typename U>
		using F = Rebind<F_,U>;

		/**
		 * Maps a function to the contained value(s).
		 *
		 * Default implementation is to invoke `applicative<F>::map`.
		 */
		template<typename Fn, typename U = result_of<Fn(T)>>
		static F<U> map(Fn&& fn, const F<T>& f) {
			return applicative<F_>::map(
					std::forward<Fn>(fn), f);
		}

		/// \overload
		template<typename Fn, typename U = result_of<Fn(T)>>
		static F<U> map(Fn&& fn, F<T>&& f) {
			return applicative<F_>::map(std::forward<Fn>(fn), std::move(f));
		}

		/**
		 * Compile time check whether a type is a functor.
		 *
		 * Because all applicative functors are functors, `F` is an instance
		 * of functor if it is an instance of applicative.
		 */
		static constexpr bool instance = applicative<F_>::instance;
	};

	/**
	 * Concepts lite-compatible predicate for functor instances.
	 *
	 * Can be used already for similar purposes by way of SFINAE.
	 *
	 * Example usage:
	 * \code
	 *   template<
	 *       typename F,
	 *       typename = Requires<Functor<F>()>
	 *   >
	 *   void myFunc(const F& f);
	 * \endcode
	 *
	 * \ingroup functor
	 */
	template<typename F>
	constexpr bool Functor() noexcept {
		return functor<F>::instance;
	}

	template<typename F>
	struct deriving_map;

	/**
	 * Implementation of `functor::map`, inheritable by many containers.
	 *
	 * Provides a default implementation of `fmap` that can be inherited by any
	 * container type fulfilling the following:
	 * - Must be \ref fwditerable
	 * - There must exist a method, `emplace_back(T&&)`, behaving semantically
	 *   equivalent of e.g. the `std::list::emplace_back` of the same signature.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   struct functor<Container<T>>
	 *   : deriving_map<back_insertable_container<Container<T>>> {
	 *       static constexpr bool instance = true;
	 *   };
	 * \endcode
	 *
	 * \note This is only an implementation of `map`, it does not include the
	 *       `instance` constant, even though it is sufficient for a full
	 *       functor implementation.
	 *
	 * \ingroup functor
	 */
	template<typename F_>
	struct deriving_map<back_insertable_container<F_>> {
		using T = Value_type<F_>;

		template<typename U>
		using F = Rebind<F_,U>;

		template<typename Fn, typename U = result_of<Fn(T)>>
		static F<U> map(Fn&& fn, const F<T>& f) {
			F<U> result;
			for(auto& e : f) {
				result.emplace_back(fn(e));
			}

			return result;
		}

		template<
				typename Fn, typename U = result_of<Fn(T)>,
				typename = Requires<
					!std::is_same<U,T>::value
					|| (!std::is_copy_assignable<T>::value
					&& !std::is_move_assignable<T>::value)
				>
		>
		static F<U> map(Fn&& fn, F<T>&& f) {
			F<U> result;
			for(auto& e : f) {
				result.emplace_back(fn(std::move(e)));
			}

			return result;
		}

		template<
				typename Fn,
				typename = Requires<
					std::is_same<result_of<Fn(T)>,T>::value
					&& (std::is_copy_assignable<T>::value
					|| std::is_move_assignable<T>::value)
				>
		>
		static F<T> map(Fn&& fn, F<T>&& f) {
			for(auto& e : f) {
				e = fn(std::move(e));
			}

			return f;
		}
	};

	/**
	 * Convenience operator for `functor::map`.
	 *
	 * This operator perfectly forwards the parameters, so it works regardless
	 * of r-valueness.
	 *
	 * Example usage:
	 * \code
	 *   MyFunctor<SomeType> foo(
	 *       const MyFunctor<OtherType>& f,
	 *       ftl::function<SomeType,OtherType> fn) {
	 *
	 *       using ftl::operator%;
	 *       return fn % f;
	 *   }
	 *
	 *   // Equivalent operator-less version
	 *   MyFunctor<SomeType> foo(
	 *       const MyFunctor<OtherType>& f,
	 *       ftl::function<SomeType,OtherType> fn) {
	 *
	 *       return functor<MyFunctor<OtherType>>::map(fn, f);
	 *   }
	 * \endcode
	 *
	 * \ingroup functor
	 */
	template<
		typename F,
		typename Fn,
		typename F_ = plain_type<F>,
		typename = Requires<Functor<F_>()>>
	auto operator% (Fn&& fn, F&& f)
	-> decltype(functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f))) {
		return functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f));
	}

	/**
	 * Convenience function object.
	 *
	 * Provided to make it easier to pass `functor::map` as parameter to
	 * higher order functions, as one might otherwise have to wrap such calls
	 * in a lambda to deal with the ambiguity in face of overloads.
	 *
	 * `fMap` values may be invoked using curried calling style, should such
	 * be wanted.
	 *
	 * Note that, unlike when invoking `functor::map` directly, it is possible
	 * to map a function returning `void`. In such cases, `fMap` behaves like
	 * `mapM_` of Haskell: the function is applied purely for its side effects,
	 * and no values are stored or returned.
	 *
	 * \ingroup functor
	 */
	struct fMap
#ifndef DOCUMENTATION_GENERATOR
	: private _dtl::curried_binf<fMap>
#endif
	{
		template<
				typename Fn,
				typename F,
				typename F_ = plain_type<F>,
				typename = Requires<
					!std::is_same<
						result_of<Fn(Value_type<F_>)>,
						void
					>::value
				>
		>
		auto operator() (Fn&& fn, F&& f) const
		-> decltype(functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f))){
			return functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f));
		}

		template<
				typename Fn,
				typename F,
				typename F_ = plain_type<F>,
				typename = Requires<
					std::is_same<
						result_of<Fn(Value_type<F_>)>,
						void
					>::value
				>
		>
		void operator() (Fn&& fn, F&& f) const {
			mapM<F_>::apply(std::forward<Fn>(fn), std::forward<F>(f));
		}

		using _dtl::curried_binf<fMap>::operator();

	private:
		template<typename F>
		struct mapM {
			template<typename Fn>
			static void apply(Fn fn, const F& f) {
				functor<F>::map(
					[fn](const Value_type<F>& t) -> int {
						fn(t);
						return 0;
					},
					f
				);
			}

			template<typename Fn>
			static void apply(Fn fn, F&& f) {
				functor<F>::map(
					[fn](Value_type<F>&& t) -> int {
						fn(std::move(t));
						return 0;
					},
					std::move(f)
				);
			}
		};
	};

	/**
	 * Compile time instance of fMap.
	 *
	 * Makes it even more convenient to pass `functor::map` as parameter to
	 * higher order functions. Also makes for a good alternative to
	 * `ftl::operator%` for those who prefer to keep their code clear of
	 * potentially confusing operators.
	 *
	 * \par Example uses
	 * Using `fmap` to map a function with side effects and no return value
	 * \code
	 *   std::list<int> l{1,2,3};
	 *
	 *   ftl::fmap([](int x){ std::cout << x << ", "; }, l);
	 * \endcode
	 * Output:
	 * \code
	 *   1, 2, 3, 
	 * \endcode
	 *
	 * As parameter to a higher order function
	 * \code
	 *   template<
	 *       typename F1, typename F2,
	 *       typename T, typename U = result_of<F1(F2,T)>
	 *   >
	 *   U foo(const F1& f1, const F2& f2, const T& t) {
	 *       return f1(f2, t);
	 *   }
	 *
	 *   MyFunctor<SomeType> bar(MyFunctor<SomeType> myFunctor) {
	 *       // Really a no-op, but demonstrates how convenient fmap can be
	 *       return foo(fmap, id, myFunctor);
	 *   }
	 * \endcode
	 *
	 * \ingroup functor
	 */
	constexpr fMap fmap{};
}

#endif

