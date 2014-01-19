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
#ifndef FTL_IMPL_TUP_APPLY_H
#define FTL_IMPL_TUP_APPLY_H

#include <tuple>
#include "../type_functions.h"

namespace ftl {
	// A number of helpers for tuple_apply
	namespace _dtl {
		template<typename F, typename...Ts, size_t...S>
		constexpr auto tup_apply_helper(seq<S...>, F&& f, const std::tuple<Ts...>& t)
		-> typename std::result_of<F(Ts...)>::type {
			return std::forward<F>(f)(std::get<S>(t)...);
		}

		template<typename F, typename...Ts, size_t...S>
		constexpr auto tup_apply_helper(seq<S...>, F&& f, std::tuple<Ts...>&& t)
		-> typename std::result_of<F(Ts...)>::type {
			return std::forward<F>(f)(std::get<S>(std::move(t))...);
		}

		template<typename F, typename Tuple>
		constexpr auto tup_apply(F&& f, Tuple&& tuple)
		-> decltype(
				tup_apply_helper(
					gen_seq<0,std::tuple_size<plain_type<Tuple>>::value-1>{},
					std::forward<F>(f),
					std::forward<Tuple>(tuple)
				)
		) {
			return tup_apply_helper(
				gen_seq<0,std::tuple_size<plain_type<Tuple>>::value-1>{},
				std::forward<F>(f),
				std::forward<Tuple>(tuple)
			);
		}
	}
}
#endif


