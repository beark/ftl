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
#include "type_functions.h"
#include "monoid.h"

namespace ftl {
	//
	// Unnamed private namespace for various tuple helpers
	namespace {

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
	 * \tparam Ts Each of the types must be an instance of monoid.
	 *   
	 */
	template<typename...Ts>
	struct monoid<std::tuple<Ts...>> {
		static std::tuple<Ts...> id() {
			return std::make_tuple(monoid<Ts>::id()...);
		}

		static std::tuple<Ts...> append(
				const std::tuple<Ts...>& t1,
				const std::tuple<Ts...>& t2) {

			auto ret = t1;
			tup<sizeof...(Ts)-1, std::tuple<Ts...>>::app(ret, t2);
			return ret;
		}
	};

	/**
	 * Functor instance for tuples.
	 *
	 * Applies the function to the first field in the tuple.
	 */
	template<
		typename F,
		typename A,
		typename B = typename decayed_result<F(A)>::type,
		typename...Ts>
	std::tuple<B, Ts...> fmap(const F& f, const std::tuple<A, Ts...>& t) {

		std::tuple<B, Ts...> ret;
		tup<sizeof...(Ts)-1, std::tuple<B, Ts...>>::fmap(f, t, ret);
		return ret;
	}

}

#endif

