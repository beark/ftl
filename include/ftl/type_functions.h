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
	 * \defgroup modules Modules
	 *
	 * The various top level modules of the library.
	 *
	 * Each module can be completely brought into scope by a single `#include`
	 * statement. There is, however, some inter-dependence between the modules
	 * and thus, the various additional headers that will be included by each
	 * module shall be documented under the apropriate section.
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
	 * None.
	 *
	 * \ingroup modules
	 */

	/**
	 * Get the basic, "undecorated" return type of a function.
	 *
	 * This is really only a composition of std::decay and std::result_of,
	 * provided for convenience.
	 *
	 * \ingroup typelevel
	 */
	template<typename>
	struct decayed_result;

	template<typename F, typename...Ps>
	struct decayed_result<F(Ps...)> {
		using type =
			typename std::decay<typename std::result_of<F(Ps...)>::type>::type;
	};

	/**
	 * Meta type used to store a type sequence.
	 *
	 * \ingroup typelevel
	 */
	template<typename...Ts>
	struct type_seq {};

	/**
	 * Concatenates two variadic lists together.
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
	 * \ingroup typelevel
	 */
	template<typename T, typename...Ts>
	using get_last = get_nth<sizeof...(Ts), T, Ts...>;

	/**
	 * Get the first N elements in a type sequence
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
	 * Take all elements except the last
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
	 * \tparam N The number of types to drop
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
	 * A number sequence
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
	 * \ingroup typelevel
	 */
	template<size_t Z, size_t N, size_t...S> struct gen_seq : gen_seq<Z,N-1,N,S...> {};

	template<size_t Z, size_t...S> struct gen_seq<Z,Z,S...> {
		using type = seq<Z,S...>;
	};
}

#endif

