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
#ifndef FTL_MAYBE_TRANS_H
#define FTL_MAYBE_TRANS_H

#include "maybe.h"

namespace ftl {
	/**
	 * \defgroup maybeT Maybe Transformer
	 *
	 * Module containing the maybe transformer monad.
	 *
	 * \code
	 *   #include <ftl/maybe_trans.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref maybe
	 */

	/**
	 * Maybe Transformer.
	 *
	 * This data type transforms any monad `M`, "stacking" it on top of the
	 * maybe monad. The point is to get the functionality of both `M` and
	 * maybe.
	 *
	 * \ingroup maybeT
	 */
	template<typename T, template<typename...> class M, typename...Ts>
	class maybeT {
	public:
		using value_type = typename parametrise<M,maybe<T>,Ts...>::type;

		explicit constexpr maybeT(const value_type& v)
		noexcept(std::is_nothrow_copy_constructible<value_type>::value)
		: mMaybe(v) {}

		explicit constexpr maybeT(value_type&& v)
		noexcept(std::is_nothrow_move_constructible<value_type>::value)
		: mMaybe(std::move(v)) {}

		template<typename...Args>
		maybeT(inplace_tag, Args&&...args)
		noexcept(std::is_nothrow_constructible<value_type,Args...>::value)
		: mMaybe{std::forward<Args>(args)...} {}

		value_type& operator* () noexcept {
			return mMaybe;
		}

		constexpr value_type& operator* () const noexcept {
			return mMaybe;
		}

		value_type* operator-> () noexcept {
			return &mMaybe;
		}

		constexpr value_type* operator-> () const noexcept {
			return &mMaybe;
		}
		
	private:
		value_type mMaybe;
	};

	template<template<typename...> class M, typename T, typename...Ts>
	struct functor<maybeT<T,M,Ts...>> {

	private:
		template<typename U>
		using M_ = typename parametrise<M,maybe<U>,Ts...>::type;

	public:
		/// Short-hand to refer to the maybe transformer's own full type.
		template<typename U>
		using mT = maybeT<U,M,Ts...>;

		/// Composition of `M`'s and maybe's map
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static mT<U> map(F&& f, const mT<T>& m) {
			return functor<M_<T>>::map(
				[f](const maybe<T>& t) {
					functor<maybe<T>>::map(f, t);
				},
				m.mMaybe
			);
		}
	};
}

#endif

