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
	 *     fmap(id, t)          <=> t
	 *   \endcode
	 * - **Preservation of composition**
	 *   \code
	 *     fmap(compose(f, g), t)  <=> compose(fmap(f), fmap(g))(t)
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
	 * - \ref prelude
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
	 * Predicate to check if a particular type is an instance of Functor.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<
	 *       typename F,
	 *       typename = Requires<Functor<F>{}>
	 *   >
	 *   void myFunc(const F& f);
	 * \endcode
	 *
	 * \ingroup functor
	 */
	template<typename F>
	struct Functor {
		static constexpr bool value = functor<F>::instance;

		constexpr operator bool() const noexcept {
			return value;
		}
	};

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
	 * \par Examples
	 *
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
	 * Convenience operator for `fmap`.
	 *
	 * As expected,
	 * \code
	 *   a % b <=> ftl::fmap(a, b)
	 * \endcode
	 * assuming `a` and `b` are both values of a type implementing the Functor
	 * concept.
	 *
	 * When using `operator%` to map functions, `std::mem_fn` will be
	 * automatically applied if it's a member function pointer.
	 *
	 * \par Examples
	 *
	 * Classic list example:
	 * \code
	 *   auto f = [](int x){ return x+1; };
	 *   std::list<int> l = {1,2,3};
	 *
	 *   auto r = f % l;
	 *   // r == {2,3,4]
	 * \endcode
	 *
	 * Mapping a member function:
	 * \code
	 *   struct example {
	 *       maybe<int> foo();
	 *   };
	 *
	 *   maybe<example> m = ...;
	 *   auto r = &example::foo % m;
	 * \endcode
	 *
	 * \ingroup functor
	 */
	template<
		typename F,
		typename Fn,
		typename F_ = plain_type<F>,
		typename = Requires<
			Functor<F_>()
#ifndef DOCUMENTATION_GENERATOR
			&& !std::is_member_function_pointer<Fn>::value
#endif
		>
	>
	auto operator% (Fn&& fn, F&& f)
	-> decltype(functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f))) {
		return functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f));
	}

	template<
		typename F,
		typename R,
		typename Fn,
		typename F_ = plain_type<F>,
		typename = Requires<
			Functor<F_>()
			&& !std::is_member_function_pointer<Fn>::value
		>
	>
	auto operator% (R (Fn::*fn)(), F&& f)
	-> decltype(functor<F_>::map(std::mem_fn(fn), std::forward<F>(f))) {
		return functor<F_>::map(std::mem_fn(fn), std::forward<F>(f));
	}

	template<
		typename F,
		typename R,
		typename Fn,
		typename F_ = plain_type<F>,
		typename = Requires<
			Functor<F_>()
			&& !std::is_member_function_pointer<Fn>::value
		>
	>
	auto operator% (R (Fn::*fn)() const, F&& f)
	-> decltype(functor<F_>::map(std::mem_fn(fn), std::forward<F>(f))) {
		return functor<F_>::map(std::mem_fn(fn), std::forward<F>(f));
	}

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _fmap : make_curried_n<2,_fmap> {
		template<
				typename Fn,
				typename F,
				typename F_ = plain_type<F>,
				typename = Requires<
					!std::is_member_function_pointer<Fn>::value
					&& !std::is_same<
						result_of<Fn(Value_type<F_>)>,
						void
					>::value
				>
		>
		auto operator() (Fn&& fn, F&& f) const
		-> decltype(functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f))){

			static_assert(Functor<F_>(), "F is not an instance of Functor");

			return functor<F_>::map(std::forward<Fn>(fn), std::forward<F>(f));
		}

		template<
				typename Fn,
				typename F,
				typename F_ = plain_type<F>,
				typename = Requires<
					!std::is_member_function_pointer<Fn>::value
					&& std::is_same<
						result_of<Fn(Value_type<F_>)>,
						void
					>::value
				>
		>
		void operator() (Fn&& fn, F&& f) const {
			static_assert(Functor<F_>(), "F is not an instance of Functor");

			mapM<F_>::apply(std::forward<Fn>(fn), std::forward<F>(f));
		}

		template<
				typename R,
				typename Fn,
				typename F,
				typename F_ = plain_type<F>,
				typename = Requires<
					std::is_member_function_pointer<Fn>::value
				>
		>
		void operator() (R (Fn::*fn)(), F&& f) const {
			static_assert(Functor<F_>(), "F is not an instance of Functor");

			functor<F_>::map(std::mem_fn(fn), std::forward<F>(f));
		}

		template<
				typename R,
				typename Fn,
				typename F,
				typename F_ = plain_type<F>,
				typename = Requires<
					std::is_member_function_pointer<Fn>::value
				>
		>
		void operator() (R (Fn::*fn)() const, F&& f) const {
			static_assert(Functor<F_>(), "F is not an instance of Functor");

			functor<F_>::map(std::mem_fn(fn), std::forward<F>(f));
		}

		using make_curried_n<2,_fmap>::operator();

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
	} fmap{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Function object representing `functor::map`.
	 *
	 * Behaves as if it were a curried function of type
	 * \code
	 *   ((T) -> U, F<T>) -> F<U>
	 * \endcode
	 *
	 * Should `U == void`, `fmap` will instead have the function type
	 * \code
	 *   ((T) -> void, F<T>) -> void
	 * \endcode
	 *
	 * Makes for a good alternative to `ftl::operator%` for those who prefer
	 * to keep their code clear of potentially confusing operators.
	 *
	 * \par Examples
	 *
	 * Using `fmap` to map a function with side effects and no return value:
	 * \code
	 *   std::list<int> l{1,2,3};
	 *
	 *   ftl::fmap([](int x){ std::cout << x << ", "; }, l);
	 *   // Output: "1, 2, 3,  "
	 * \endcode
	 *
	 * Mapping a function to eithers:
	 * \code
	 *   ftl::either<string,int> e1 = ftl::make_right<string>(10);
	 *   ftl::either<string,int> e2 = ftl::make_left<int>(string("blah"));
	 *
	 *   auto plusOne = [](int x){ return x+1; };
	 *   auto r1 = ftl::fmap(plusOne, e1); // r1 == make_right(11)
	 *   auto r2 = ftl::fmap(plusOne, e2); // r2 == make_left("blah")
	 * \endcode
	 *
	 * As parameter to a higher-order function:
	 * \code
	 *   list<list<int>> l{list<int>{1,2}, list<int>{3,4}};
	 *
	 *   auto plusOne = [](int x){ return x+1; };
	 *
	 *   // As fmap is curried, we can apply it to one argument at a time
	 *   auto r = ftl::fmap(ftl::fmap(plusOne), l); // r == {{2,3}, {4,5}}
	 * \endcode
	 *
	 * \ingroup functor
	 */
	fmap;
#endif // DOCUMENTATION_GENERATOR
}

#endif

