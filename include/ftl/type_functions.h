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
	 * writing, that is, to the author's knowledge, gcc (4.8 and later) and
	 * clang (3.2 and later). To tell gcc/clang your project uses C++11
	 * features, add `-std=c++11` to its compilation flags.
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
	 * `Value_type<ForwardIterable>`.
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

	template<class T>
	using plain_type = ::std::remove_cv_t<::std::remove_reference_t<T>>;

	namespace dtl_ {
		template<typename>
		struct decayed_result;

		template<typename F, typename...Ps>
		struct decayed_result<F(Ps...)> {
			using type = plain_type<typename std::result_of<F(Ps...)>::type>;
		};
	}

	template<bool, typename Then, typename>
	struct if_impl {
		using type = Then;
	};

	template<typename Then,typename Else>
	struct if_impl<false,Then,Else> {
		using type = Else;
	};

	/**
	 * Conditional type alias.
	 *
	 * If the compile-time predicate `Pred` evaluates to `true`, then `if_`
	 * will evaluate to the `Then` type. Otherwise, it will evaluate to the
	 * `Else` type.
	 *
	 * \par Examples
	 *
	 * \code
	 *   // type_alias will be an alias of type 'int', as maybe is a monad
	 *   using type_alias = ftl::if_<
	 *       ftl::Monad<ftl::maybe<int>>(),
	 *       int,
	 *       float
	 *   >;
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<bool Pred, typename Then, typename Else = void>
	using if_ = typename if_impl<Pred,Then,Else>::type;

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
	using result_of = typename dtl_::decayed_result<F>::type;

	/**
	 * Meta type used to store a variadic type sequence.
	 *
	 * \ingroup typelevel
	 */
	template<typename...Ts>
	struct type_seq {};

	namespace dtl_ {
		template<typename T, typename TSeq>
		struct prepend_type_impl;

		template<typename T, typename...Ts>
		struct prepend_type_impl<T,type_seq<Ts...>> {
			using type = type_seq<T,Ts...>;
		};
	}

	/**
	 * Prepends a single type to a `type_seq`.
	 *
	 * \par Examples
	 *
	 * \code
	 *   using bc = ftl::type_seq<B,C>;
	 *
	 *   using abc = prepend_type<A,bc>;
	 *
	 *   // std::is_same<ftl::type_seq<A,B,C>, abc>::value == true
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<class T, class TSeq>
	using prepend_type = typename dtl_::prepend_type_impl<T,TSeq>::type;

	namespace dtl_ {
		template<template<class> class F, class...Ts>
		struct map_types_impl;

		template<template<class> class F>
		struct map_types_impl<F> {
			using type = type_seq<>;
		};

		template<template<class> class F, class T, class...Ts>
		struct map_types_impl<F,T,Ts...> {
			using type =
				prepend_type<
					typename F<T>::type,
					typename map_types_impl<F,Ts...>::type
				>;
		};

		template<template<typename> class F, typename...Ts>
		struct map_types_impl<F,type_seq<Ts...>> {
			using type = typename map_types_impl<F,Ts...>::type;
		};

		template<template<typename,typename> class F, typename L1, typename L2>
		struct zip_types_impl;

		template<template<typename,typename> class F>
		struct zip_types_impl<F,type_seq<>,type_seq<>> {
			using type = type_seq<>;
		};

		template<template<typename,typename> class F, typename U, typename...Us>
		struct zip_types_impl<F,type_seq<>,type_seq<U,Us...>> {
			using type = type_seq<>;
		};

		template<template<typename,typename> class F, typename T, typename...Ts>
		struct zip_types_impl<F,type_seq<T,Ts...>,type_seq<>> {
			using type = type_seq<>;
		};

		template<
				template<typename,typename> class F,
				typename T, typename U,
				typename...Ts, typename...Us
		>
		struct zip_types_impl<F,type_seq<T,Ts...>,type_seq<U,Us...>> {
			using type = prepend_type<
				typename F<T,U>::type,
				typename zip_types_impl<F,type_seq<Ts...>,type_seq<Us...>>::type
			>;
		};
	}

	/**
	 * Maps a unary type level function to a variadic pack of types.
	 *
	 * The type level function `F` is assumed to follow the convention of using
	 * a member typedef `type` as result. I.e., `map_types` will "call" `F`
	 * like this:
	 *
	 * \code
	 *   typename F<T>::type
	 * \endcode
	 *
	 * The result type of `map_types` is a `type_seq` containing the transformed
	 * types in the order they originally appeared.
	 *
	 * Can also be called on a `type_seq` for the same result on the types
	 * contained within.
	 *
	 * \par Examples
	 *
	 * Using on variadic type pack
	 * \code
	 *   using type = map_types<std::remove_const, bool, const int, std::string>;
	 *   // type is:
	 *   // type_seq<bool,int,std::string>
	 * \endcode
	 *
	 * Using on a type_seq
	 * \code
	 *   using type = map_types<std::add_ptr, type_seq<int,bool,char>>;
	 *   // type is:
	 *   // type_seq<int*,bool*,char*>
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<template<typename> class F, typename...Ts>
	using map_types = typename dtl_::map_types_impl<F,Ts...>::type;

	/**
	 * Zips two type sequences using a binary type level function.
	 *
	 * The type level function `F` is assumed to follow the convention of using
	 * a member typedef `type` as result. I.e., `zip_types` will "call" `F`
	 * like this:
	 *
	 * \code
	 *   typename F<T,U>::type
	 * \endcode
	 *
	 * \par Examples
	 *
	 * \code
	 *   using type = zip_types<
	 *       is_same,
	 *       type_seq<int,bool,float>,
	 *       type_seq<char,bool>
	 *   >;
	 *   // type is:
	 *   // type_seq<integral_constant<bool,false>,integral_constant<bool,true>>
	 * \endcode
	 *
	 * \note Can not be called directly on variadic packs&mdash;they must be
	 *       encapsulated in `type_seq`s.
	 *
	 * \ingroup typelevel
	 */
	template<template<typename,typename> class F, typename Ts1, typename Ts2>
	using zip_types = typename dtl_::zip_types_impl<F,Ts1,Ts2>::type;

  template<class T, class...Ts>
    struct type_is_in : ::std::false_type {};

  template<class T, class...Ts>
  struct type_is_in<T, T, Ts...> : ::std::true_type {};

  template<class T, class U, class...Ts>
  struct type_is_in<T, U, Ts...> : type_is_in<T,Ts...> {};

  template<class...Ts>
  constexpr bool type_is_in_v = type_is_in<Ts...>::value;

	template<size_t I, typename T, typename...Ts>
	struct index_of_impl;

	template<size_t I, typename T, typename...Ts>
	struct index_of_impl<I,T,type_seq<T,Ts...>> {
		static constexpr size_t value = I;
	};

	template<size_t I, typename X, typename T, typename...Ts>
	struct index_of_impl<I,X,type_seq<T,Ts...>> {
		static constexpr size_t value
			= index_of_impl<I+1,X,type_seq<Ts...>>::value;
	};

	template<size_t I, typename T, typename...Ts>
	struct index_of_impl<I,T,T,Ts...> {
		static constexpr size_t value = I;
	};

	template<size_t I, typename X, typename T, typename...Ts>
	struct index_of_impl<I,X,T,Ts...> {
		static constexpr size_t value = index_of_impl<I+1,X,Ts...>::value;
	};

	/**
	 * Gets the index of a specific type in a pack or sequence.
	 *
	 * \tparam T the type to find
	 * \tparam Ts either a regular variadic pack to search, or a `type_seq` of
	 *            types to search.
	 *
	 * If the type is not among those given, a compile error will be generated.
	 *
	 * \par Examples
	 *
	 * With a plain variadic list of types
	 * \code
	 *   size_t index = ftl::index_of<int, char, float, int, string>::value;
	 *   // index == 2
	 * \endcode
	 *
	 * Searching a `type_seq`
	 * \code
	 *   size_t index = ftl::index_of<char, type_seq<char,int>>::value;
	 *   // index == 0
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<typename T, typename...Ts>
	struct index_of {
		static constexpr size_t value = index_of_impl<0,T,Ts...>::value;
	};

  template<typename T, typename...Ts>
  constexpr size_t index_of_v = index_of<T,Ts...>::value;

	template<size_t I, typename T, typename...Ts>
	struct type_at_impl : type_at_impl<I-1,Ts...> {};

	template<typename T, typename...Ts>
	struct type_at_impl<0,T,Ts...> {
		using type = T;
	};

	/**
	 * Gets the type in a variadic list at the given index.
	 *
	 * \tparam I zero based index
	 * \tparam Ts a variadic list of types to search
	 *
	 * \par Examples
	 *
	 * \code
	 *   using T = type_at<1,int,float,double>;
	 *   // T is an alias of float
	 * \endcode
	 *
	 * \ingroup typelevel
	 */
	template<size_t I, typename...Ts>
	using type_at = typename type_at_impl<I,Ts...>::type;

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
	template<typename T>
	struct inner_type { using type = T; };

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

	namespace dtl_ {
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
		using rebind = typename ::ftl::dtl_::default_rebind<T,U>::type;
	};

	template<typename T>
	struct parametric_type_traits<const T> {
		using value_type = typename parametric_type_traits<T>::value_type;

		template<typename U>
		using rebind =
			const typename parametric_type_traits<T>::template rebind<U>;
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
	struct is_same_template : std::false_type {};

	template<
			template<typename...> class T,
			template<typename...> class U,
			typename...Ts,
			typename...Us
	>
	struct is_same_template<T<Ts...>,U<Us...>>
		: std::is_same<T<Ts...>, U<Ts...>> {};

	template<
			bool x, bool y,
			template<bool, typename...> class T,
			template<bool, typename...> class U,
			typename...Ts,
			typename...Us
	>
	struct is_same_template<T<x, Ts...>, U<y, Us...>>
		: std::is_same<T<x, Ts...>, U<x, Ts...>> {};

}

#endif

