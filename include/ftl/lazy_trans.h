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
#ifndef FTL_LAZY_TRANS_H
#define FTL_LAZY_TRANS_H

#include "prelude.h"
#include "lazy.h"

namespace ftl {
	/**
	 * \defgroup lazyT Lazy Transformer
	 *
	 * \code
	 *   #include <ftl/lazy_trans.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref prelude
	 * - \ref lazy
	 */

	template<typename M>
	class lazyT {
	public:
		using T = concept_parameter<M>;
		using Mlt = typename re_parametrise<M,lazy<T>>::type;

		explicit constexpr lazyT(const Mlt& l)
		noexcept(std::is_nothrow_copy_constructible<Mlt>::value)
		: mLazy(l) {}

		explicit constexpr lazyT(Mlt&& l)
		noexcept(std::is_nothrow_copy_constructible<Mlt>::value)
		: mLazy(std::move(l)) {}

		template<typename...Args>
		lazyT(inplace_tag, Args&&...args)
		noexcept(std::is_nothrow_constructible<M,Args...>::value)
		: mLazy{std::forward<Args>(args)...} {}

		Mlt& operator* () noexcept {
			return mLazy;
		}

		const Mlt& operator* () const noexcept {
			return mLazy;
		}

		Mlt* operator-> () noexcept {
			return &mLazy;
		}

		const Mlt* operator-> () const noexcept {
			return &mLazy;
		}

	private:
		Mlt mLazy;
	};

	template<typename M, typename U>
	struct re_parametrise<lazyT<M>,U> {
		using type = lazyT<typename re_parametrise<M,U>::type>;
	};

	template<typename M>
	struct parametric_type_traits<lazyT<M>> {
		using parameter_type = concept_parameter<M>;
	};

	template<typename M>
	struct monad<lazyT<M>> {
		using T = typename lazyT<M>::T;

		template<typename U>
		using M_ = typename re_parametrise<M,U>::type;

		template<typename U>
		using lT = lazyT<M_<U>>;

		static lT<T> pure(T t) {
			function<T> f = [t](){ return t; };
			return lT<T>{monad<M_<lazy<T>>>::pure(lazy<T>(f))};
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static lT<U> map(F f, const lT<T>& l) {
			return lT<U>{
				[f](const lazy<T>& l){ return f % l; } % l
			};
		}

		static constexpr bool instance = true;
	};
}

#endif

