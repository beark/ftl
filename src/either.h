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
#ifndef FTL_EITHER_H
#define FTL_EITHER_H

#include "monad.h"

namespace ftl {

	/*!
	 * Data type modelling a "one of" type.
	 *
	 * Put simply, an instance of either<L,R> can store a value of \em
	 * either type L, or type R, but not both at the same time.
	 *
	 * Either fulfills the following concepts if, and only if,
	 * \em both of its sub-types also do:
	 *  
	 * \li CopyConstructible
	 * \li MoveConstrutible
	 * \li Assignable
	 * \li EqComparable
	 *
	 * Either fulfills the following concepts regardless of its sub-types:
	 *
	 * \li Functor (in L)
	 * \li Monad (in L)
	 *
	 * \note Either is \em not DefaultConstructible.
	 *
	 * \note L and R may \em not be the same type.
	 *
	 * \tparam L The "left" type of the disjunction
	 *
	 * \tparam R The "right" type of the disjunction
	 */
	template<typename L, typename R>
	class either {
	public:
		/// The type either implements various concepts on
		using value_type = L;

		either() = delete;

		either(const either& e)
		noexcept(  std::is_nothrow_copy_constructible<L>::value
				&& std::is_nothrow_copy_constructible<R>::value)
		: isL(e.isL) {
			if(isL)
				new (&l) L(e.l);
			else
				new (&r) R(e.r);
		}

		either(either&& e)
		noexcept(  std::is_nothrow_move_constructible<L>::value
				&& std::is_nothrow_move_constructible<R>::value)
		: isL(e.isL) {
			if(isL)
				new (&l) L(std::move(e.l));
			else
				new (&r) R(std::move(e.r));
		}

		explicit constexpr either(const L& left)
		noexcept(std::is_nothrow_copy_constructible<L>::value)
		: l(left), isL(true) {
		}

		explicit constexpr either(L&& left)
		noexcept(std::is_nothrow_move_constructible<L>::value)
		: l(std::move(left)), isL(true) {
		}

		explicit constexpr either(const R& right)
		noexcept(std::is_nothrow_copy_constructible<R>::value)
		: r(right), isL(false) {
		}

		explicit constexpr either(R&& right)
		noexcept(std::is_nothrow_move_constructible<R>::value)
		: r(std::move(right)), isL(false) {
		}

		~either() {
			if(isL)
				l.~L();
			else
				r.~R();
		}

		/*!
		 *  Check if the either instance contains the left type.
		 */
		constexpr bool isLeft() noexcept {
			return isL;
		}

		/*!
		 *  Check if the either instance contains the right type.
		 */
		constexpr bool isRight() noexcept {
			return !isL;
		}

		/*!
		 *  Get reference to left value.
		 *
		 *  \note Undefined behaviour if the either instance is
		 *  really of type R.
		 */
		L& left() noexcept {
			return l;
		}

		/*!
		 *  Get reference to right value.
		 *
		 *  \note Undefined behaviour if the either instance is
		 *  really of type L.
		 */
		R& right() noexcept {
			return r;
		}

		/// \overload
		constexpr L left() const noexcept {
			return l;
		}

		/// \overload
		constexpr R right() const noexcept {
			return r;
		}

		const either& operator= (const either& e) {
			if(isL) {
				if(e.isL) 
					l = e.l;

				else {
					l.~L();
					new (&r) R(e.r);
					isL = false;
				}
			}

			else {
				if(!e.isL)
					r = e.r;

				else {
					r.~R();
					new (&l) L(e.l);
					isL = true;
				}
			}

			return *this;
		}

		const either& operator=(either&& e) {
			if(isL) {
				if(e.isL)
					l = std::move(e.l);

				else {
					l.~L();
					new (&r) R(std::move(e.r));
					isL = false;
				}
			}

			else {
				if(!e.isL)
					r = std::move(e.r);

				else {
					r.~R();
					new (&l) L(std::move(e.l));
					isL = true;
				}
			}

			return *this;
		}

		const either& operator= (const L& left) {
			if(!isL) {
				r.~R();
				isL = true;
				new (&l) L(left);
			}

			else
				l = left;

			return *this;
		}

		const either& operator= (L&& left) {
			if(!isL) {
				r.~R();
				isL = true;
				new (&l) L(std::move(left));
			}

			else
				l = std::move(left);

			return *this;
		}

		const either& operator= (const R& right) {
			if(isL) {
				l.~L();
				isL = false;
				new (&r) R(right);
			}
			
			else
				r = right;

			return *this;
		}

		const either& operator= (R&& right) {
			if(isL) {
				l.~L();
				isL = false;
				new (&r) R(std::move(right));
			}

			else
				r = std::move(right);

			return *this;
		}

		constexpr bool operator== (const either& e) {
			return isLeft()
				? e.isLeft() && l == e.l
				: !e.isLeft() && r == e.r;
		}

		constexpr bool operator!= (const either& e) {
			return !(*this == e);
		}

	private:
		union {
			L l;
			R r;
		};

		bool isL;
	};

	/**
	 * Monad implementation for either.
	 */
	template<>
	struct monad<either> {
		template<typename A, typename R>
		static either<A,R> pure(const A& a) {
			return either<A,R>(a);
		}

		template<typename A, typename R>
		static either<A,R> pure(A&& a) {
			return either<A,R>(std::move(a));
		}

		template<
			typename F,
			typename R,
			typename A,
			typename B = typename decayed_result<F(A)>::type::value_type>
		static either<B,R> bind(const either<A,R>& e, F f) {
			if(e.isLeft())
				return f(e.left());
			else
				return either<B,R>(e.right());
		}
	};

	/**
	 * Functor implementation for either.
	 *
	 * \note either is a functor in L, unlike in Haskell.
	 */
	template<
		typename F,
		typename L,
		typename R,
		typename L2 = typename decayed_result<F(L)>::type>
	either<L2, R> fmap(const F& f, const either<L, R>& e) {

		if(e.isLeft())
			return either<L2, R>(f(e.left()));
		else
			return either<L2, R>(e.right());
	}

	/// \overload
	template<
		typename L,
		typename R,
		typename Mr,
		typename...Ps>
	either<L, R>& fmap(Mr (L::*method) (Ps...), const either<L, R>& e, Ps&&...ps) {
		if(e.isLeft()) {
			(e.left().*method)(ps...);
		}
		else
			return e;
	}

}

#endif

