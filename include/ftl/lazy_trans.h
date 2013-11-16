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
	 * The lazyT transformer and its concept instances.
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
	 * essentially `Rebind<M,lazy<T>>`, except the functor series
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
		using T = Value_type<M>;

		/// Convenient access to underlying type
		using Mlt = Rebind<M,lazy<T>>;

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
		 * Forwards `args` to the underlying type's constructor.
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

	template<typename M>
	struct parametric_type_traits<lazyT<M>> {
		using value_type = Value_type<M>;

		template<typename T>
		using rebind = lazyT<Rebind<M,T>>;
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
	struct monad<lazyT<M>>
	: deriving_join<in_terms_of_bind<lazyT<M>>>
	, deriving_apply<in_terms_of_bind<lazyT<M>>> {
		/// Concept parameter of the particular instance.
		using T = typename lazyT<M>::T;

		/// Type alias to simplify signatures involving the base monad.
		template<typename U>
		using M_ = Rebind<M,U>;

		/// Type alias to simplify signatures involving the lazy transformer.
		template<typename U>
		using lT = lazyT<M_<U>>;

		/**
		 * Wrap a value in a lazyT.
		 *
		 * Essentially equivalent of invoking `M`'s pure on a deferred
		 * computation that will result in `t`.
		 */
		static lT<T> pure(T t) {
			function<T()> f = [t](){ return t; };
			return lT<T>{monad<M_<lazy<T>>>::pure(lazy<T>(f))};
		}

		/**
		 * Functorial mapping.
		 *
		 * Nests `lazy`'s map inside `M`'s.
		 */
		template<typename F, typename U = result_of<F(T)>>
		static lT<U> map(F f, const lT<T>& l) {
			return lT<U>{
				[f](const lazy<T>& l){ return f % l; } % *l
			};
		}

		/// \overload
		template<typename F, typename U = result_of<F(T)>>
		static lT<U> map(F f, lT<T>&& l) {
			return lT<U>{
				// Cannot move contents of lazy; should be immutable
				[f](lazy<T>&& l){ return f % l; } % std::move(*l)
			};
		}

		/**
		 * Sequence two lazyT computations.
		 *
		 * Includes automatic lifting of `f`s that return the untransformed
		 * type, i.e., `M_<U>`.
		 *
		 * Example:
		 * \code
		 *   lazyT<maybe<int>> foo(int);
		 *   maybe<int> bar(int);
		 *   
		 *   lazyT<maybe<int>> lmi = ...;
		 *
		 *   // Both of these work (using the >>= operator for bind), the
		 *   // second one by utilising the automatic lifting.
		 *   auto r1 = lmi >>= foo;
		 *   auto r2 = lmi >>= bar;
		 * \endcode
		 */
		template<typename F, typename U = Value_type<result_of<F(T)>>>
		static lT<U> bind(const lT<T>& l, F&& f) {
			using monad_t = result_of<F(T)>;
			return bind_helper<monad_t>::bind(l, std::forward<F>(f));
		}

		template<
				typename F,
				typename U =
					Value_type<result_of<F(T)>>
		>
		static lT<U> bind(lT<T>&& l, F&& f) {
			using monad_t = result_of<F(T)>;
			return bind_helper<monad_t>::bind(std::move(l), std::forward<F>(f));
		}

		static constexpr bool instance = monad<M>::instance;

	private:
		template<typename M2>
		struct bind_helper {
			using U = Value_type<M2>;

			template<
					typename F,
					typename = Requires<
						std::is_same<Rebind<M,U>, M2>::value
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
					typename = Requires<
						std::is_same<Rebind<M,U>, M2>::value
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
			using U = Value_type<M2>;

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

