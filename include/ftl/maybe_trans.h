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
	 * \par Concepts
	 * - \ref fullycons, if the transformed type is.
	 * - \ref assignable, as above.
	 * - \ref deref to the unwrapped transformed type.
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 * - \ref foldable, if `M` is Foldable.
	 *
	 * \ingroup maybeT
	 */
	template<typename M>
	class maybeT {
	public:
		using T = concept_parameter<M>;
		using Mmt = typename re_parametrise<M,maybe<T>>::type;

		explicit constexpr maybeT(const Mmt& m)
		noexcept(std::is_nothrow_copy_constructible<Mmt>::value)
		: mMaybe(m) {}

		explicit constexpr maybeT(Mmt&& m)
		noexcept(std::is_nothrow_move_constructible<M>::value)
		: mMaybe(std::move(m)) {}

		template<typename...Args>
		maybeT(inplace_tag, Args&&...args)
		noexcept(std::is_nothrow_constructible<Mmt,Args...>::value)
		: mMaybe{std::forward<Args>(args)...} {}

		Mmt& operator* () noexcept {
			return mMaybe;
		}

		const Mmt& operator* () const noexcept {
			return mMaybe;
		}

		Mmt* operator-> () noexcept {
			return &mMaybe;
		}

		const Mmt* operator-> () const noexcept {
			return &mMaybe;
		}
		
	private:
		Mmt mMaybe;
	};

	/**
	 * ftl::maybeT specialisation of re_parametrise.
	 *
	 * Necessary because maybeT isn't parametrised in quite the default manner.
	 */
	template<typename M, typename U>
	struct re_parametrise<maybeT<M>,U> {
		/// Simply re-parametrises the inner/base monad.
		using type = maybeT<typename re_parametrise<M,U>::type>;
	};

	/**
	 * Parametric type traits for ftl::maybeT.
	 */
	template<typename M>
	struct parametric_type_traits<maybeT<M>> {
		/// The concept parameter of a maybeT is the same as its base monad's.
		using parameter_type = concept_parameter<M>;
	};

	/**
	 * `maybeT`'s monad instance.
	 *
	 * "Stacks" `M` on top of `maybe`.
	 */
	template<typename M>
	struct monad<maybeT<M>> {
		/// Shorthand for the concept parameter
		using T = typename maybeT<M>::T;

		template<typename U>
		using M_ = typename re_parametrise<M,U>::type;

		/// Easy reference to maybeT of various parameterisations.
		template<typename U>
		using mT = maybeT<M_<U>>;

		/// Uses `M`'s pure to create a value (as opposed to `nothing`)
		static mT<T> pure(T&& t) {
			return mT<T>{monad<M_<maybe<T>>>::pure(value(std::forward<T>(t)))};
		}

		/// Composition of `M`'s and maybe's map
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static mT<U> map(F f, const mT<T>& m) {
			return mT<U>{
				[f](const maybe<T>& t) { return f % t; } % *m
			};
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static mT<U> map(F f, mT<T>&& m) {
			return mT<U>{
				[f](const maybe<T>& t) { return f % t; } % std::move(*m)
			};
		}

		/**
		 * Monadic bind operation.
		 *
		 * Binds `M`'s bind operation with an expression that uses `maybe`'s
		 * bind, to get the composition of the two.
		 *
		 * Note that FTL provides automatic lifting of monadic operations, so
		 * `F` may well be a monadic action in `M`, not just `maybeT<M>`.
		 */
		template<
				typename F,
				typename U =
					concept_parameter<typename decayed_result<F(T)>::type>
		>
		static mT<U> bind(const mT<T>& m, F&& f) {

			using monad_t = typename decayed_result<F(T)>::type;

			return bind_helper<monad_t>::bind(m, std::forward<F>(f));
		}

		/**
		 * \overload
		 */
		template<
				typename F,
				typename U =
					concept_parameter<typename decayed_result<F(T)>::type>
		>
		static mT<U> bind(mT<T>&& m, F&& f) {

			using monad_t = typename decayed_result<F(T)>::type;

			return bind_helper<monad_t>::bind(std::move(m), std::forward<F>(f));
		}

		static constexpr bool instance = true;

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
			static mT<U> bind(const mT<T>& m, F f) {
				return mT<U>{
					*m >>= [f](const maybe<T>& m) {
						if(m)
							return f(*m);
						else
							return monad<M_<maybe<U>>>::pure(maybe<U>{});
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
			static mT<U> bind(mT<T>&& m, F f) {
				return mT<U>{
					std::move(*m) >>= [f](maybe<T>&& m) {
						if(m)
							return f(std::move(*m));
						else
							return monad<M_<maybe<U>>>::pure(maybe<U>{});
					}
				};
			}
		};

		template<typename M2>
		struct bind_helper<maybeT<M2>> {
			using U = concept_parameter<M2>;

			template<typename F>
			static mT<U> bind(const mT<T>& m, F f) {
				return mT<U>{
					*m >>= [f](const maybe<T>& m) {
						if(m)
							return *f(*m);
						else
							return monad<M_<maybe<U>>>::pure(maybe<U>{});
					}
				};
			}

			template<typename F>
			static mT<U> bind(mT<T>&& m, F f) {
				return mT<U>{
					std::move(*m) >>= [f](maybe<T>&& m) {
						if(m)
							return *f(std::move(*m));
						else
							return monad<M_<maybe<U>>>::pure(maybe<U>{});
					}
				};
			}
		};

	};
}

#endif

