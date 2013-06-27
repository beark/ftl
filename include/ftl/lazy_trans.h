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

	/**
	 * The lazy transformer.
	 *
	 * Transforms the monad instance `M`, applied to `T`, into what is
	 * essentially `re_parametrise<M,lazy<T>>::type`, except the functor series
	 * of operations are lifted so that they work on `T` instead of `lazy<T>`
	 * as they would in the former case.
	 *
	 * \par Concepts
	 * - \ref fullycons, if the underlying type is.
	 * - \ref deref to the underlying type
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 *
	 * \tparam M must be a \ref monad
	 *
	 * \ingroup lazyT
	 */
	template<typename M>
	class lazyT {
	public:
		/// Convenient access to parameter type
		using T = concept_parameter<M>;

		/// Convenient access to underlying type
		using Mlt = typename re_parametrise<M,lazy<T>>::type;

		/// Construct from underlying type
		explicit constexpr lazyT(const Mlt& l)
		noexcept(std::is_nothrow_copy_constructible<Mlt>::value)
		: mLazy(l) {}

		/// \overload
		explicit constexpr lazyT(Mlt&& l)
		noexcept(std::is_nothrow_copy_constructible<Mlt>::value)
		: mLazy(std::move(l)) {}

		/**
		 * Inplace constructor
		 *
		 * Forwards `args` to the underlying types constructor.
		 */
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

	/**
	 * Monad instance of lazyT.
	 *
	 * Behaves as the underlying type's monad instance, except all the values
	 * are wrapped in `lazy`.
	 *
	 * \ingroup lazyT
	 */
	template<typename M>
	struct monad<lazyT<M>> {
		/// Concept parameter of the particular instance
		using T = typename lazyT<M>::T;

		template<typename U>
		using M_ = typename re_parametrise<M,U>::type;

		template<typename U>
		using lT = lazyT<M_<U>>;

		/**
		 * Wrap a value in a lazyT.
		 *
		 * Essentially equivalent of invoking `M`'s pure on a deferred
		 * computation that will result in `t`.
		 */
		static lT<T> pure(T t) {
			function<T> f = [t](){ return t; };
			return lT<T>{monad<M_<lazy<T>>>::pure(lazy<T>(f))};
		}

		/**
		 * Functorial mapping.
		 *
		 * Nests `lazy`'s map inside `M`'s.
		 */
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static lT<U> map(F f, const lT<T>& l) {
			return lT<U>{
				[f](const lazy<T>& l){ return f % l; } % *l
			};
		}

		/// \overload
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static lT<U> map(F f, lT<T>&& l) {
			return lT<U>{
				// Cannot move contents of lazy; should be immutable
				[f](lazy<T>&& l){ return f % l; } % std::move(*l)
			};
		}

		/**
		 * Sequence two lazyT computations.
		 */
		template<
				typename F,
				typename U =
					concept_parameter<typename decayed_result<F(T)>::type>
		>
		static lT<U> bind(const lT<T>& l, F&& f) {
			using monad_t = typename decayed_result<F(T)>::type;
			return bind_helper<monad_t>::bind(l, std::forward<F>(f));
		}

		template<
				typename F,
				typename U =
					concept_parameter<typename decayed_result<F(T)>::type>
		>
		static lT<U> bind(lT<T>&& l, F&& f) {
			using monad_t = typename decayed_result<F(T)>::type;
			return bind_helper<monad_t>::bind(std::move(l), std::forward<F>(f));
		}

		static constexpr bool instance = monad<M>::instance;

	private:
		template<typename M2>
		struct bind_helper {
			using U = concept_parameter<M2>;

			template<
					typename F,
					typename = typename std::enable_if<
						std::is_same<
							typename re_parametrise<M,U>::type,
							M2
						>::value
					>
			>
			static lT<U> bind(const lT<T>& l, F f) {
				return lT<U>{
					*l >>= [f](const lazy<T>& l) {
						return aPure<lazy<U>>() % f(*l);
					}
				};
			}

			template<
					typename F,
					typename = typename std::enable_if<
						std::is_same<
							typename re_parametrise<M,U>::type,
							M2
						>::value
					>
			>
			static lT<U> bind(lT<T>&& l, F f) {
				return lT<U>{
					std::move(*l) >>= [f](lazy<T>&& l) {
						return aPure<lazy<U>>() % f(*l);
					}
				};
			}
		};

		template<typename M2>
		struct bind_helper<lazyT<M2>> {
			using U = concept_parameter<M2>;

			template<typename F>
			static lT<U> bind(const lT<T>& l, F f) {
				return lT<U>{
					*l >>= [f](const lazy<T>& l) {
						return *f(*l);
					}
				};
			}

			template<typename F>
			static lT<U> bind(lT<T>&& l, F f) {
				return lT<U>{
					std::move(*l) >>= [f](lazy<T>&& l) {
						return *f(*l);
					}
				};
			}
		};

		// TODO: automatic hoisting too
	};
}

#endif

