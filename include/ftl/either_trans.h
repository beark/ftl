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

#include "prelude.h"
#include "either.h"
#include "concepts/monoid.h"

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
	 * - \ref prelude
	 * - \ref either
	 * - \ref monoid
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
	 * - \ref monoidapg, if `L` is a \ref monoidpg.
	 * - \ref foldablepg, if `M` is foldable.
	 *
	 * \tparam L The left type in the either values.
	 * \tparam M A complete monad type, e.g. `std::list<some_type>`.
	 *
	 * \ingroup eitherT
	 */
	template<typename L, typename M>
	class eitherT {
	public:
		/// Quick reference to the type concepts are implemented on
		using T = Value_type<M>;

		/// The transformed type `eitherT` wraps
		using Met = Rebind<M,either<L,T>>;

		/**
		 * Construct from an unwrapped equivalent of the transformed type.
		 */
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
		 *
		 * This can be used to "regain" some functionality of `M` that was
		 * "hidden" by wrapping it in `eitherT`.
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
		 *
		 * Completes the \ref deref concept.
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

	// eitherT's parametric traits are non-default.
	template<typename L, typename M>
	struct parametric_type_traits<eitherT<L,M>> {
		using value_type = Value_type<M>;

		template<typename T>
		using rebind = eitherT<L,Rebind<M,T>>;
	};

	/**
	 * Monad instance for `eitherT`.
	 *
	 * In essence, composes the basic monadic operations of `M` with
	 * `ftl::either`.
	 *
	 * \ingroup eitherT
	 */
	template<typename L, typename M>
	struct monad<eitherT<L,M>>
	: deriving_join<in_terms_of_bind<eitherT<L,M>>>
	, deriving_apply<in_terms_of_bind<eitherT<L,M>>> {
		using T = typename eitherT<L,M>::T;

		template<typename U>
		using M_ = Rebind<M,U>;

		/** 
		 * Type define to make remaining type signatures easier to read.
		 *
		 * `M_` in this case can be thought of as the unparametrised base monad
		 * `M` (i.e., the type parameter of `M_` is applied as the concept
		 * parameter of `M` in a `Rebind` call).
		 */
		template<typename U>
		using eT = eitherT<L,M_<U>>;

		static eT<T> pure(const T& t) {
			return eT<T>{monad<M_<either<L,T>>>::pure(
					make_right<L>(t))};
		}

		static eT<T> pure(T&& t) {
			return eT<T>{monad<M_<either<L,T>>>::pure(
					make_right<L>(std::move(t)))};
		}

		/**
		 * Functorial mapping.
		 *
		 * In essence, a composition of `functor<either>::map` and 
		 * `functor<M>::map`. Formulated in a more wordy sense, "drills down" to
		 * right values embedded in either, embedded in `M`.
		 *
		 * \tparam F must satisfy \ref fn`<U(T)>`, where `T` is the concept
		 *           parameter of `M` and `U` is any type that can be contained
		 *           in an `M` _and_ in an `either`.
		 */
		template<typename F, typename U = result_of<F(T)>>
		static eT<U> map(F f, const eT<T>& e) {
			return eT<U>{
				[f](const either<L,T>& e) { return f % e; } % *e
			};
		}

		/// \overload
		template<typename F, typename U = result_of<F(T)>>
		static eT<U> map(F f, eT<T>&& e) {
			return eT<U>{
				[f](either<L,T>&& e) {return f % std::move(e);} % std::move(*e)
			};
		}

		/**
		 * Monadic bind.
		 *
		 * Uses `M`'s bind operation on top of `either`'s. 
		 */
		template<typename F, typename U = Value_type<result_of<F(T)>>>
		static eT<U> bind(const eT<T>& e, F&& f) {
			using monad_t = result_of<F(T)>;

			return bind_helper<monad_t>::bind(e, std::forward<F>(f));
		}

		/// \overload
		template<typename F, typename U = typename result_of<F(T)>::T>
		static eT<U> bind(eT<T>&& e, F&& f) {
			using monad_t = result_of<F(T)>;

			return bind_helper<monad_t>::bind(std::move(e), std::forward<F>(f));
		}

		static constexpr bool instance = true;

	private:
		// Helper struct required to implement automatic lift and hoist
		template<typename M2>
		struct bind_helper {
			using U = Value_type<M2>;

			// Automatic lift when binding to operations in M_
			template<
					typename F,
					typename = Requires<std::is_same<Rebind<M,U>, M2>::value>
			>
			static eT<U> bind(const eT<T>& e, F f) {
				return eT<U>{
					*e >>= [f](const either<L,T>& e) {
						if(e) {
							return aPure<either<L,U>>() % f(*e);
						}
						else {
							return monad<M_<either<L,U>>>::pure(
								make_left<U>(e.left())
							);
						}
					}
				};
			}

			template<
					typename F,
					typename = Requires<std::is_same<Rebind<M,U>, M2>::value>
			>
			static eT<U> bind(eT<T>&& e, F f) {
				return eT<U>{
					std::move(*e) >>= [f](either<L,T>&& e) {
						if(e)
							return aPure<either<L,U>>() % f(std::move(*e));
						else {
							return monad<M_<either<L,U>>>::pure(
								make_left<U>(e.left())
							);
						}
					}
				};
			}
		};

		// Normal case, we're binding with a computation in eitherT
		template<typename M2>
		struct bind_helper<eitherT<L,M2>> {
			using U = Value_type<M2>;

			template<typename F>
			static eT<U> bind(const eT<T>& e, F f) {
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

			template<typename F>
			static eT<U> bind(eT<T>&& e, F f) {
				return eT<U>{
					std::move(*e) >>= [f](either<L,T>&& e) -> M_<either<L,U>> {
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
		};

		// Automatic hoisting of plain either
		template<typename U>
		struct bind_helper<either<L,U>> {

			template<typename F>
			static eT<U> bind(const eT<T>& e, F f) {
				return e >>= [f](const T& t) {
					return eT<U>{
						monad<M_<either<L,U>>>::pure(
							make_right<L>(t) >>= f
						)
					};
				};
			}

			template<typename F>
			static eT<U> bind(eT<T>&& e, F f) {
				return std::move(e) >>= [f](T&& t) {
					return eT<U>{
						monad<M_<either<L,U>>>::pure(
							make_right<L>(std::move(t)) >>= f
						)
					};
				};
			}
		};
	};

	// Forward declarations
	template<typename> struct foldable;
	template<typename> struct deriving_fold;
	template<typename> struct deriving_foldMap;

	/**
	 * Foldable instance for eitherT.
	 *
	 * \tparam M must be \ref foldablepg
	 */
	template<typename L, typename M>
	struct foldable<eitherT<L,M>>
	: deriving_foldMap<eitherT<L,M>>, deriving_fold<eitherT<L,M>> {

		using T = Value_type<M>;
		using Met = typename eitherT<L,M>::Met;

		template<
				typename F,
				typename U,
				typename = Requires<
					std::is_same<U, result_of<F(U,T)>>::value
				>
		>
		static U foldl(F f, U z, const eitherT<L,M>& me) {
			return foldable<Met>::foldl(
				[f](U z, const either<L,T>& e){
					if(e)
						return f(z, *e);

					else
						return z;
				},
				z,
				*me
			);
		}

		template<
				typename F,
				typename U,
				typename = Requires<
					std::is_same<U, result_of<F(T,U)>>::value
				>
		>
		static U foldr(F f, U z, const eitherT<L,M>& me) {
			return foldable<Met>::foldr(
				[f](const either<L,T>& e, U z){
					if(e)
						return f(*e, z);

					else
						return z;
				},
				z,
				*me
			);
		}

		static constexpr bool instance = foldable<M>::instance;
	};

	/**
	 * EitherT's monoidal alternative instance.
	 *
	 * \tparam L must be a monoid for this instance to be available.
	 *
	 * \ingroup eitherT
	 */
	template<typename L, typename M>
	struct monoidA<eitherT<L,M>> {
		using T = Value_type<M>;
		using Met = typename eitherT<L,M>::Met;

		/**
		 * Invoke the failure state.
		 *
		 * Failing embeds a left value of `monoid<L>`'s identity element with
		 * `monad<M>::pure`.
		 */
		static eitherT<L,M> fail() {
			return eitherT<L,M>{
				monad<Met>::pure(make_left<T>(monoid<L>::id()))
			};
		}

		/**
		 * Evaluate two alternatives.
		 *
		 * If `e1` wraps a right value, it is instantly returned. Otherwise,
		 * `e2` is checked for rightness. If both `e1` and `e2` wrap left
		 * values, they are combined using `monoid<L>::append` and a new left
		 * value (embedded as if by `monad<M>::pure`) is returned.
		 */
		static eitherT<L,M> orDo(const eitherT<L,M>& e1, eitherT<L,M> e2) {
			return eitherT<L,M> {
				*e1 >>= [e2](const either<L,T>& e) -> Met {
					if(e) {
						return monad<Met>::pure(e);
					}
					else {
						return liftM(
							[e](const either<L,T>& e2) -> either<L,T> {
								if(e2)
									return e2;
								else
									return make_left<T>(e.left() ^ e2.left());
							},
							*e2
						);
					}
				}
			};
		}

		static constexpr bool instance = monoid<L>::instance;
	};

}	

#endif

