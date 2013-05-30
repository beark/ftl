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
#ifndef FTL_TUPLE_H
#define FTL_TUPLE_H

#include <tuple>
#include "monoid.h"
#include "applicative.h"

namespace ftl {

	/**
	 * \defgroup tuple Tuple
	 *
	 * Concept instances and utility functions for std::tuple.
	 *
	 * \code
	 *   #include <ftl/tuple.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - <tuple>
	 * - \ref monoid
	 * - \ref applicative
	 */

	// Private namespace for various tuple helpers
	namespace _dtl {

		template<std::size_t N, typename T>
		struct tup {
			static void app(T& ret, const T& t2) {
				tup<N-1, T>::app(ret, t2);
				std::get<N>(ret) = std::get<N>(ret) ^ std::get<N>(t2);
			}

			template<typename F, typename O>
			static void fmap(const F& f, T& ret, const O& o) {
				tup<N-1,T>::fmap(f, ret, o);
				std::get<N>(ret) = std::get<N>(o);
			}
		};

		template<typename T>
		struct tup<0, T> {
			static void app(T& ret, const T& t2) {
				std::get<0>(ret) = std::get<0>(ret) ^ std::get<0>(t2);
			}

			template<typename F, typename O>
			static void fmap(const F& f, T& ret, const O& o) {
				std::get<0>(ret) = f(std::get<0>(o));
			}
		};

		template<
			typename F,
			typename...Ts,
			size_t...S>
		auto tup_apply(seq<S...>, const F& f, const std::tuple<Ts...>& t)
		-> typename std::result_of<F(Ts...)>::type {
			return f(std::get<S>(t)...);
		}

		template<
			typename F,
			typename...Ts,
			size_t...S>
		auto tup_apply(seq<S...>, const F& f, std::tuple<Ts...>&& t)
		-> typename std::result_of<F(Ts...)>::type {
			return f(std::get<S>(t)...);
		}

		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type,
			typename...Ts,
			size_t...S>
		std::tuple<B,Ts...> apply_on_first(
				const std::tuple<F,Ts...>& t1,
				const std::tuple<A,Ts...>& t2,
				seq<S...>) {

			auto f = std::get<0>(t1);
			return std::tuple<B,Ts...>(
					f(std::get<0>(t2)),
					monoid<
						typename std::decay<decltype(std::get<S>(t1))>::type
					>::append(
						std::get<S>(t1), std::get<S>(t2))...
					);
		}

		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type,
			typename...Ts>
		std::tuple<B,Ts...> applicative_implementation(
				const std::tuple<F,Ts...>& t1,
				const std::tuple<A,Ts...>& t2) {
			return apply_on_first(
					t1,
					t2,
					typename gen_seq<1,sizeof...(Ts)>::type());
		}

		template<typename...>
		struct allMonoids {
		};

		template<>
		struct allMonoids<> {
			static constexpr bool value = true;
		};

		template<typename T, typename...Ts>
		struct allMonoids<T,Ts...> {
			static constexpr bool value
				= monoid<T>::instance && allMonoids<Ts...>::value;
		};

	}

	/**
	 * Implementation of monoid for tuples.
	 *
	 * Basically, id will simply generate a tuple of id:s. That is, a call
	 * to
	 * \code
	 *   monoid<std::tuple<t1, t2, ..., tN>>::id();
	 * \endcode
	 * is equivalent to
	 * \code
	 *   std::make_tuple(
	 *       monoid<t1>::id(),
	 *       monoid<t2>::id(),
	 *       ...,
	 *       monoid<tN>::id());
	 * \endcode
	 *
	 * In a similar fashion, the combining operation is applied to all the
	 * fields in the tuples, like so:
	 * \code
	 *   tuple1 ^ tuple2
	 *   <=>
	 *   std::make_tuple(
	 *       std::get<0>(tuple1) ^ std::get<0>(tuple2),
	 *       std::get<1>(tuple1) ^ std::get<1>(tuple2),
	 *       ...,
	 *       std::get<N>(tuple1) ^ std::get<N>(tuple2))
	 * \endcode
	 *
	 * \tparam Ts Each of the types must be an instance of \ref monoid.
	 *
	 * \ingroup tuple
	 */
	template<typename...Ts>
	struct monoid<std::tuple<Ts...>> {
		static auto id()
		-> typename std::enable_if<
				_dtl::allMonoids<Ts...>::value,
				std::tuple<Ts...>>::type {
			return std::make_tuple(monoid<Ts>::id()...);
		}

		static auto append(
				const std::tuple<Ts...>& t1,
				const std::tuple<Ts...>& t2)
		-> typename std::enable_if<
				_dtl::allMonoids<Ts...>::value,
				std::tuple<Ts...>>::type {

			auto ret = t1;
			_dtl::tup<sizeof...(Ts)-1, std::tuple<Ts...>>::app(ret, t2);
			return ret;
		}

		static constexpr bool instance = _dtl::allMonoids<Ts...>::value;
	};

	/**
	 * Functor instance for tuples.
	 *
	 * Separate from the applicative instance because tuples are always
	 * functors, but only applicative ones if the remaining types are all
	 * monoids.
	 *
	 * \ingroup tuple
	 */
	template<typename T, typename...Ts>
	struct functor<std::tuple<T,Ts...>> {
		/// Apply `f` to first element in the tuple
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		std::tuple<U,Ts...> map(F&& f, const std::tuple<T,Ts...>& t) {

			std::tuple<U,Ts...> ret;
			_dtl::tup<sizeof...(Ts)-1, std::tuple<U,Ts...>>::fmap(
					std::forward<F>(f), t, ret);
			return ret;
		}
	};

	/**
	 * Applicative instance for tuples.
	 *
	 * Note that this requires a monoid instance for every type in the tuple
	 * except the first one.
	 *
	 * \ingroup tuple
	 */
	template<typename T, typename...Ts>
	struct applicative<std::tuple<T,Ts...>> {

		static std::tuple<T,Ts...> pure(T&& a) {
			return std::make_tuple(std::forward<T>(a), monoid<Ts>::id()...);
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static std::tuple<U,Ts...> map(F&& f, const std::tuple<T,Ts...>& t) {
			return functor<std::tuple<T,Ts...>>::map(std::forward<F>(f), t);
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static std::tuple<U,Ts...> apply(
				const std::tuple<F,Ts...>& tfn,
				const std::tuple<T,Ts...>& t) {
			return _dtl::applicative_implementation(tfn, t);
		}

		static constexpr bool instance = true;
	};

	/**
	 * Invoke a function using a tuple's fields as parameters.
	 *
	 * \ingroup tuple
	 */
	template<typename F, typename...Ts>
	auto apply(F&& f, const std::tuple<Ts...>& t)
	-> typename std::result_of<F(Ts...)>::type {
		using indices_t = typename gen_seq<0,sizeof...(Ts)-1>::type;
		return _dtl::tup_apply(indices_t(), std::forward<F>(f), t);
	}

	/**
	 * \overload
	 *
	 * \ingroup tuple
	 */
	template<typename F, typename...Ts>
	auto apply(F&& f, std::tuple<Ts...>&& t)
	-> typename std::result_of<F(Ts...)>::type {
		using indices_t = typename gen_seq<0,sizeof...(Ts)-1>::type;
		return _dtl::tup_apply(indices_t(), std::forward<F>(f), std::move(t));
	}

}

#endif

