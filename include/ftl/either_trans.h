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
#ifndef FTL_EITHER_TRANS_H
#define FTL_EITHER_TRANS_H

#include "either.h"

namespace ftl {
	/**
	 * \defgroup eitherT Either Transformer
	 *
	 * The either transformer and its concept instances.
	 *
	 * \code
	 *   #include <ftl/either_trans.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref either
	 */

	/**
	 * The either transformer.
	 *
	 * Transforms any given monad, `M`, so that it also acts like the either
	 * monad. In reality, this is done by embedding an `ftl::either<L,T>` as the
	 * concept parameter of `M`, where `T` is `M`'s original concept parameter.
	 *
	 * In other words, `ftl::eitherT<a,std::list<b>>` becomes what is basically
	 * `std::list<ftl::either<a,b>>`, except monadic operations on the transform
	 * work on `b` instead of `ftl::either<a,b>`.
	 *
	 * \par Concepts
	 * - \ref fullycons, if the transformed type (as mentioned above) is.
	 * - \ref assignable, as above.
	 * - \ref deref to the unwrapped transformed type.
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 *
	 * \tparam L The left type in the either values.
	 * \tparam M A complete monad type, e.g. `std::list<some_type>`.
	 */
	template<typename L, typename M>
	class eitherT {
	public:
		/// Easy reference to type concepts are implemented on
		using T = concept_parameter<M>;

		/// The transformed type `eitherT` wraps
		using Met = typename re_parametrise<M,either<L,T>>::type;

		/// Construct from an unwrapped equivalent of the transformed type
		explicit constexpr eitherT(const Met& m)
		noexcept(std::is_nothrow_copy_constructible<Met>::value)
		: mEither(m) {}

		/// \overload
		explicit constexpr eitherT(Met&& m)
		noexcept(std::is_nothrow_move_constructible<Met>::value)
		: mEither(std::move(m)) {}

		/**
		 * In-place construction.
		 *
		 * Forwards args to `Met`'s constructor.
		 */
		template<typename...Args>
		eitherT(inplace_tag, Args&&...args)
		noexcept(std::is_nothrow_constructible<Met,Args...>::value)
		: mEither{std::forward<Args>(args)...} {}

		/**
		 * Unwraps the inner, transformed, monad.
		 */
		Met& operator* () noexcept {
			return mEither;
		}

		/// \overload
		const Met& operator* () const noexcept {
			return mEither;
		}

		/**
		 * Access members of the wrapped monad.
		 */
		Met* operator-> () noexcept {
			return &mEither;
		}

		/// \overload
		const Met* operator-> () const noexcept {
			return &mEither;
		}

	private:
		Met mEither;
	};

	/// Re-parametrising an eitherT requires non-default actions.
	template<typename M, typename L, typename U>
	struct re_parametrise<eitherT<L,M>,U> {
		using type = eitherT<L,typename re_parametrise<M,U>::type>;
	};

	/// eitherT's parametric traits are non-default.
	template<typename L, typename M>
	struct parametric_type_traits<eitherT<L,M>> {
		using concept_parameter = concept_parameter<M>;
	};

	/**
	 * Monad instance for eitherT.
	 *
	 * In essence, composes the basic monadic operations of `M` with
	 * `ftl::either`.
	 */
	template<typename L, typename M>
	struct monad<eitherT<L,M>> {
		using T = typename eitherT<L,M>::T;

		template<typename U>
		using M_ = typename re_parametrise<M,U>::type;

		template<typename U>
		using eT = eitherT<L,M_<U>>;

		static eT<T> pure(T&& t) {
			return eT<T>{monad<M_<either<L,T>>>::pure(
					make_right<L>(std::forward<T>(t)))};
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static eT<U> map(F&& f, const eT<T>& e) {
			return eT<U>{monad<M_<either<L,T>>>::map(
				[f](const either<L,T>& e) {
					return monad<either<L,T>>::map(f, e);
				},
				*e
			)};
		}

		/**
		 * Monadic bind.
		 *
		 * Uses `M`'s bind operation on top of `either`'s. 
		 */
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type::T
		>
		static eT<U> bind(const eT<T>& e, F&& f) {
			return eT<U>{
				*e >>= [f](const either<L,T>& e) {
					if(e)
						return *f(*e);
					else {
						return monad<M_<either<L,U>>>::pure(
							make_left<U>(e.left())
						);
					}
				}
			};
		}

		/// \overload
		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type::T
		>
		static eT<U> bind(eT<T>&& e, F&& f) {
			return eT<U>{
				std::move(*e) >>= [f](const either<L,T>& e) {
					if(e)
						return *f(std::move(*e));
					else {
						return monad<M_<either<L,U>>>::pure(
							make_left<U>(e.left())
						);
					}
				}
			};
		}

		static constexpr bool instance = true;
	};
}	

#endif

