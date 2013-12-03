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
#ifndef FTL_TYPELEVEL_H
#define FTL_TYPELEVEL_H

#include <type_traits>
#include <cstddef>

namespace ftl {
	/**
	 * \mainpage
	 *
	 * \section Introduction
	 * FTL is a pure template library (i.e., there is no linking to it,
	 * there are only headers to include) that draws heavy inspiration from
	 * Haskell and its libraries.
	 *
	 * \section Goals
	 * The primary goal of the FTL is to give users of C++ access to the various
	 * abstractions commonly available in functional languages. The secondary
	 * goal is to accomplish the above without disrupting or overthrowing the
	 * existing stdlib, instead preferring to _extend_ it whenver possible.
	 *
	 * \section Usage
	 * Because FTL is a pure template library, there is no need to link to
	 * anything special or jump through other hoops to get things to work.
	 * Merely include the header of the desired module(s), either by adding
	 * the include/ directory of the FTL library to your compiler's include
	 * path, or by placing the desired headers anywhere you like that is
	 * already in the include path. The former is the recommended practice.
	 *
	 * To compile a project using FTL, you need to make sure your compiler
	 * supports the required C++11 features and is set to use them. As of this
	 * writing, that is, to the author's knowledge, _only_ gcc versions 4.7 and
	 * later. To tell gcc your project uses C++11 features, add `-std=c++11` to
	 * its compilation flags.
	 */

	/**
	 * \page fullycons FullyConstructible
	 *
	 * Types with a "full" set of constructors.
	 *
	 * Any type that has the full set of standard constructors, i.e., implements
	 * \ref defcons, \ref copycons, and \ref movecons.
	 */

	/**
	 * \page fwditerable ForwardIterable
	 *
	 * Any container-like type that can be iterated linearly in one direction.
	 *
	 * In short, anything that provides a forward iterable interface. More
	 * formally, this means that `std::begin(forwardIterable)`, and
	 * `std::end(forwardIterable)` are both valid expressions returning some
	 * iterator like object that satisfies \ref deref to the type
	 * `concept_parameter<ForwardIterable>`.
	 */

	/**
	 * \page deref Dereferenceable
	 *
	 * Types that are dereferenceable.
	 *
	 * A type is dereferenceable if it has pointer-like semantics. That is, at
	 * minimum, it must define `operator*` (the unary version), and
	 * `operator->`.
	 *
	 * The behaviour of these should be as expected, in the context of the type
	 * in question. I.e., `operator->` should allow access to methods of some
	 * wrapped type (if there are several, which one it applies to should be
	 * well defined and documented).
	 */

	/**
	 * \page empty EmptyState
	 *
	 * Types that may have an invalid or "empty" state.
	 *
	 * In practice, anything that has an implicit or explicit `operator bool`
	 * defined. This includes the primitive types `bool`, `T*`, etc. in
	 * addition to types that actually define such an operator.
	 *
	 * The main purpose of naming a type an instance of this concept is to
	 * inform users of the type that there exists an easy way to check for
	 * the empty state:
	 * \code
	 *   if(someValue)
	 * \endcode
	 */

	/**
	 * \page assignable Assignable
	 *
	 * Types that can be assigned to.
	 *
	 * Any type that is both \ref copyassignable and \ref moveassignable.
	 */

	/**
	 * \defgroup typelevel Type Level Functions
	 *
	 * A collection of "functions" working at a type level.
	 *
	 * \code
	 *   #include <ftl/type_functions.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - <type_traits>
	 * - <cstddef>
	 */

	template<typename T>
	using plain_type = typename std::decay<T>::type;

	namespace _dtl {
		template<typename>
		struct decayed_result;

		template<typename F, typename...Ps>
		struct decayed_result<F(Ps...)> {
			using type = plain_type<typename std::result_of<F(Ps...)>::type>;
		};
	}

	/**
	 * Get the basic, "undecorated" return type of a function.
	 *
	 * This is really only a composition of std::decay and std::result_of,
	 * provided for convenience.
	 *
	 * Example:
	 * \code
	 *   template<typename F>
	 *   void foo() {
	 *       // t will be the decayed type of whatever F returns when called
	 *       // with an int
	 *       ftl::result_of<F(int)> t;
	 *   }
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<typename F>
	using result_of = typename _dtl::decayed_result<F>::type;

	/**
	 * Meta type used to store a variadic type sequence.
	 *
	 * \ingroup typelevel
	 */
	template<typename...Ts>
	struct type_seq {};

	/**
	 * Concatenates two type_seqs.
	 *
	 * Example:
	 * \code
	 *   // ts has type type_seq<char,int,bool,float>
	 *   typename concat_type_seqs<
	 *       type_seq<char,int>,
	 *       type_seq<bool,float>
	 *   >::type ts;
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<typename L, typename R>
	struct concat_type_seqs {};

	template<typename...Ls, typename...Rs>
	struct concat_type_seqs<type_seq<Ls...>,type_seq<Rs...>> {
		using type = type_seq<Ls...,Rs...>;
	};

	/**
	 * Repeat a type N times.
	 *
	 * Example:
	 * \code
	 *   // ts will be of type type_seq<int,int,int>
	 *   typename repeat<int,3>::type ts;
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<typename T, size_t N>
	struct repeat : concat_type_seqs<
					typename repeat<T,N/2>::type,
					typename repeat<T,N-N/2>::type> {};

	template<typename T>
	struct repeat<T,0> {
		using type = type_seq<>;
	};

	template<typename T>
	struct repeat<T,1> {
		using type = type_seq<T>;
	};

	/**
	 * Get the Nth type in a type sequence.
	 *
	 * Example:
	 * \code
	 *   // t will be of type float
	 *   typename get_nth<1,bool,float,int>::type t;
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<size_t N, typename T, typename...Ts>
	struct get_nth : get_nth<N-1,Ts...> {};

	template<typename T, typename...Ts>
	struct get_nth<0,T,Ts...> {
		using type = T;
	};

	template<size_t N, typename T, typename...Ts>
	struct get_nth<N,type_seq<T,Ts...>> : get_nth<N-1,type_seq<Ts...>> {};

	template<typename T, typename...Ts>
	struct get_nth<0,type_seq<T,Ts...>> {
		using type = T;
	};

	/**
	 * Get the final element in a type sequence
	 *
	 * Example:
	 * \code
	 *   // t will be of type float
	 *   typename get_last<bool,int,float>::type t;
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<typename T, typename...Ts>
	using get_last = get_nth<sizeof...(Ts), T, Ts...>;

	/**
	 * Get the first N elements in a type sequence
	 *
	 * Example:
	 * \code
	 *   // ts will be of type type_seq<char,bool>
	 *   typename take_types<2,char,bool,int,float>::type ts;
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<size_t N, typename T, typename...Ts>
	struct take_types : concat_type_seqs<
						type_seq<T>,
						typename take_types<N-1,Ts...>::type> {};

	template<typename T, typename...Ts>
	struct take_types<1,T,Ts...> {
		using type = type_seq<T>;
	};

	/**
	 * Take all elements except the last one.
	 *
	 * Example:
	 * \code
	 *   // ts will be of type type_seq<char,bool>
	 *   typename take_init<char,bool,int>::type ts;
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<typename...Ts>
	struct take_init {
		using type = typename take_types<sizeof...(Ts)-1, Ts...>::type;
	};

	template<typename T>
	struct take_init<T> {
		using type = type_seq<>;
	};

	template<typename T, typename U>
	struct take_init<T,U> {
		using type = type_seq<T>;
	};

	/**
	 * Drops a number of types from a type sequence.
	 *
	 * The types are dropped in left-to-right order.
	 *
	 * \tparam N The number of types to drop
	 * \tparam Ts The sequence ot drop from
	 *
	 * Example:
	 * \code
	 *   // ts will be of type type_seq<char,double>
	 *   typename drop_types<2,int,float,char,double>::type ts;
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<size_t N, typename...Ts>
	struct drop_types {};

	template<size_t N, typename T, typename...Ts>
	struct drop_types<N,T,Ts...> : drop_types<N-1, Ts...> {};

	template<typename T, typename...Ts>
	struct drop_types<0,T,Ts...> {
		using type = type_seq<T,Ts...>;
	};

	template<size_t N>
	struct drop_types<N> {
		using type = type_seq<>;
	};

	template<template<typename...> class To, typename From>
	struct copy_variadic_args {};

	template<
		template<typename...> class To,
		template<typename...> class From,
		typename...Ts>
	struct copy_variadic_args<To, From<Ts...>> {
		using type = To<Ts...>;
	};

	/**
	 * A number sequence.
	 *
	 * \ingroup typelevel
	 */
	template<size_t...> struct seq {};

	/**
	 * Generate a sequence of numbers.
	 *
	 * \tparam Z The first number in the sequence.
	 * \tparam N The final number in the sequence.
	 *
	 * Example:
	 * \code
	 *   // S is of type seq<0,1,2,3,4,5>
	 *   typename gen_seq<0,5>::type S;
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<size_t Z, size_t N, size_t...S> struct gen_seq : gen_seq<Z,N-1,N,S...> {};

	template<size_t Z, size_t...S> struct gen_seq<Z,Z,S...> {
		using type = seq<Z,S...>;
	};

	/**
	 * Find the first contained type of some parametrised type.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   void foo(const T& t) {
	 *       typename inner_type<T>::type x = ...;
	 *   }
	 *
	 *   void bar() {
	 *       // x in foo will be an int in this invocation
	 *       foo(std::vector<int>{});
	 *   }
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<typename...>
	struct inner_type;

	template<template<typename> class Tt, typename T>
	struct inner_type<Tt<T>> {
		using type = T;
	};

	template<
			template<typename,typename...> class Tt,
			typename T,
			typename...Ts>
	struct inner_type<Tt<T,Ts...>> {
		using type = T;
	};

	namespace _dtl {
		template<typename T, typename>
		struct default_rebind {
			using type = T;
		};

		template<template<typename> class Tt, typename T, typename U>
		struct default_rebind<Tt<T>,U> {
			using type = Tt<U>;
		};

		template<
				template<typename...> class Tt,
				typename T, typename U, typename...Ts
		>
		struct default_rebind<Tt<T,Ts...>,U> {
			using type = Tt<U,Ts...>;
		};
	}

	/**
	 * Traits for various parametric types.
	 *
	 * In most cases, the default should suffice, but it is possible to
	 * specialise if required.
	 *
	 * \ingroup typelevel
	 */
	template<typename T>
	struct parametric_type_traits {
		/**
		 * Gets the type parameter of `T` on which concepts apply.
		 *
		 * Many of the concepts in FTL are parametric in one type, e.g.
		 * ftl::functor<ftl::maybe<U>> is parametrised by `U`. This type level
		 * function should return that type.
		 *
		 * The only time this trait needs to be specialised is when the type
		 * parameter `T` uses in concepts is _not_ the left-most in `T`'s
		 * parameter list.
		 *
		 * Example demonstrating semantics:
		 * \code
		 *   // x is of type int
		 *   typename parametric_type_traits<either<string,int>::value_type x;
		 * \endcode
		 */
		using value_type = typename inner_type<T>::type;

		/**
		 * Specialisable type function to rebind a parametric type.
		 *
		 * The purpose of this type function is to swap the `value_type` of a
		 * parametric type. This is useful when writing templated functions that
		 * deal with genericised parameters, such as a function from a
		 * `functor<T>` to `functor<U>` (i.e., the concrete types could be
		 * `maybe<int>` and `maybe<float>` for some instantiation).
		 *
		 * Example:
		 * \code
		 *   // x could be e.g. list<int>, if M is a list of some type T
		 *   parametric_type_traits<M>::template rebind<int> x;
		 * \endcode
		 */
		template<typename U>
		using rebind = typename _dtl::default_rebind<T,U>::type;
	};

	/**
	 * Convenient way of getting the primary concept type of a parametric type.
	 *
	 * This is equivalent of `parametric_type_traits<T>::value_type`, but makes
	 * for much more concise code.
	 *
	 * \ingroup typelevel
	 */
	template<typename T>
	using Value_type =
		typename parametric_type_traits<T>::value_type;

	/**
	 * Alias of `parametric_type_traits<X>::template rebind<T>`.
	 *
	 * \ingroup typelevel
	 */
	template<typename X, typename T>
	using Rebind = typename parametric_type_traits<X>::template rebind<T>;

	/**
	 * Check if a type is just a parametrised version of some templated base.
	 *
	 * Example:
	 * \code
	 *   std::cout << std::boolalpha;
	 *   std::cout << is_base_template<maybe<int>,maybe>::value << std::endl;
	 *   std::cout << is_base_template<either<int,int>,maybe>::value << std::endl;
	 *   
	 *   // The above outputs:
	 *   // true
	 *   // false
	 * \endcode
	 */
	template<
			typename T,
			template<typename...> class Tb
	>
	struct is_base_template {
		static constexpr bool value = false;
	};

	template<
			template<typename...> class Tb,
			typename...Ts
	>
	struct is_base_template<Tb<Ts...>,Tb> {
		static constexpr bool value = true;
	};

	/**
	 * Check if two parametric types are the same base.
	 *
	 * Example:
	 * \code
	 *   std::cout << std::boolalpha;
	 *   std::cout << is_same_template<maybe<int>,maybe<float>>::value << std::endl;
	 *   std::cout << is_same_template<maybe<int>,std::vector<int>>::value << std::endl;
	 *   // The above outputs:
	 *   // true
	 *   // false
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<typename T, typename U>
	struct is_same_template;

	template<
			template<typename...> class T,
	   		template<typename...> class U,
			typename...Ts,
			typename...Us
	>
	struct is_same_template<T<Ts...>,U<Us...>> {
		static constexpr bool value = std::is_same<T<Ts...>, U<Ts...>>::value;
	};

}

#endif

