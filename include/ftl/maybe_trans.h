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
	 * - \ref monoida
	 * - \ref foldable, if `M` is Foldable.
	 *
	 * \ingroup maybeT
	 */
	template<typename M>
	class maybeT {
	public:
		using T = Value_type<M>;
		using Mmt = Rebind<M,maybe<T>>;

		/**
		 * Default construction.
		 *
		 * Initialises the wrapped value with monad::pure(nothing).
		 */
		maybeT() : mMaybe(monad<Mmt>::pure(maybe<T>{})) {}

		maybeT(const maybeT&)
		noexcept(std::is_nothrow_copy_constructible<Mmt>::value) = default;

		maybeT(maybeT&&)
		noexcept(std::is_nothrow_move_constructible<Mmt>::value) = default;

		~maybeT() = default;

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
	 * Parametric type traits for ftl::maybeT.
	 */
	template<typename M>
	struct parametric_type_traits<maybeT<M>> {
		/// The concept parameter of a maybeT is the same as its base monad's.
		using value_type = Value_type<M>;

		template<typename T>
		using rebind = maybeT<Rebind<M,T>>;
	};

	/**
	 * `maybeT`'s monad instance.
	 *
	 * "Stacks" `M` on top of `maybe`.
	 *
	 * \ingroup maybeT
	 */
	template<typename M>
	struct monad<maybeT<M>>
	: deriving_join<in_terms_of_bind<maybeT<M>>>
	, deriving_apply<in_terms_of_bind<maybeT<M>>> {
		/// Shorthand for the concept parameter
		using T = typename maybeT<M>::T;

		template<typename U>
		using M_ = Rebind<M,U>;

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
				typename U = result_of<F(T)>
		>
		static mT<U> map(F f, const mT<T>& m) {
			return mT<U>{
				[f](const maybe<T>& t) { return f % t; } % *m
			};
		}

		template<
				typename F,
				typename U = result_of<F(T)>
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
				typename U = Value_type<result_of<F(T)>>
		>
		static mT<U> bind(const mT<T>& m, F&& f) {

			using monad_t = result_of<F(T)>;

			return bind_helper<monad_t>::bind(m, std::forward<F>(f));
		}

		/**
		 * \overload
		 */
		template<
				typename F,
				typename U = Value_type<result_of<F(T)>>
		>
		static mT<U> bind(mT<T>&& m, F&& f) {

			using monad_t = result_of<F(T)>;

			return bind_helper<monad_t>::bind(std::move(m), std::forward<F>(f));
		}

		static constexpr bool instance = true;

	private:
		template<typename M2>
		struct bind_helper {
			
			using U = Value_type<M2>;

			template<
					typename F,
					typename = Requires<std::is_same<Rebind<M,U>, M2>::value>
			>
			static mT<U> bind(const mT<T>& m, F f) {
				return mT<U>{
					*m >>= [f](const maybe<T>& m) {
						if(m)
							return aPure<maybe<U>>() % f(*m);
						else
							return monad<M_<maybe<U>>>::pure(maybe<U>{});
					}
				};
			}

			template<
					typename F,
					typename = Requires<std::is_same<Rebind<M,U>, M2>::value>
			>
			static mT<U> bind(mT<T>&& m, F f) {
				return mT<U>{
					std::move(*m) >>= [f](maybe<T>&& m) {
						if(m)
							return aPure<maybe<U>>() % f(std::move(*m));
						else
							return monad<M_<maybe<U>>>::pure(maybe<U>{});
					}
				};
			}
		};

		template<typename M2>
		struct bind_helper<maybeT<M2>> {
			using U = Value_type<M2>;

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

	/**
	 * Monoidal alternative instance for maybeT.
	 *
	 * \ingroup maybeT
	 */
	template<typename M>
	struct monoidA<maybeT<M>> {

		/// Embeds a `nothing` in `M`.
		static maybeT<M> fail() {
			using Mmt = typename maybeT<M>::Mmt;
			using T = Value_type<M>;

			return monad<Mmt>::pure(maybe<T>{});
		}

		/**
		 * Performs the monadic computation `mm1`. If it fails, `mm2` is
		 * returned, otherwise the result is (re-wrapped).
		 */
		static maybeT<M> orDo(const maybeT<M>& mm1, maybeT<M> mm2) {

			using T = Value_type<M>;
			using Mmt = typename maybeT<M>::Mmt;

			return maybeT<M> {
				*mm1 >>= [mm2](const maybe<T>& m) -> Mmt {
					if(m)
						return monad<Mmt>::pure(m);

					else
						return *mm2;
				}
			};
		}

		static constexpr bool instance = monad<M>::instance;
	};

	// Forward declarations
	template<typename> struct foldable;
	template<typename> struct deriving_fold;
	template<typename> struct deriving_foldMap;

	/**
	 * Foldable instance for maybeT.
	 *
	 * Elements in `M` that are `nothing` are simply skipped, for all
	 * other elements, the fold function is applied to their unwrapped value.
	 *
	 * \tparam M must be \ref foldablepg
	 */
	template<typename M>
	struct foldable<maybeT<M>>
	: deriving_fold<maybeT<M>>, deriving_foldMap<maybeT<M>> {

		using T = Value_type<M>;
		using Mmt = typename maybeT<M>::Mmt;

		template<
				typename F,
				typename U,
				typename = Requires<std::is_same<U, result_of<F(T,U)>>::value>
		>
		static U foldl(F f, U z, const maybeT<M>& mT) {
			return foldable<Mmt>::foldl(
				[f](U z, const maybe<T>& m) {
					if(m)
						return f(z, *m);

					return z;
				},
				z,
				*mT
			);
		}

		template<
				typename F,
				typename U,
				typename = Requires<std::is_same<U, result_of<F(T,U)>>::value>
		>
		static U foldr(F f, U z, const maybeT<M>& mT) {
			return foldable<Mmt>::foldr(
				[f](const maybe<T>& m, U z){
					if(m)
						return f(*m, z);

					else
						return z;
				},
				z,
				*mT
			);
		}


		static constexpr bool instance = foldable<M>::instance;
	};
}

#endif

