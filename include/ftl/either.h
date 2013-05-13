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
#include <stdexcept>

namespace ftl {

	namespace _dtl {
		enum tag_t {
			LEFT,
			LIMBO,
			RIGHT
		};
	}

	/**
	 * \defgroup either Either
	 *
	 * The either data type and associated concept instances.
	 *
	 * \code
	 *   #include <ftl/either.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - stdexcept
	 * - memory
	 * - functional
	 * - type_traits
	 * - ftl/monoid.h
	 * - ftl/monad.h
	 * - ftl/applicative.h
	 * - ftl/functor.h
	 * - ftl/function.h
	 * - ftl/type_functions.h
	 *
	 * \ingroup modules
	 */

	/**
	 * Tag used to distinguish which "side" to construct
	 *
	 * This is required because there might well be cases where `L` = `R`
	 *
	 * \ingroup either
	 */
	struct left_tag_t {};

	/**
	 * Tag used to distinguish which "side" to construct
	 *
	 * This is required because there might well be cases where `L` = `R`
	 *
	 * \ingroup either
	 */
	struct right_tag_t {};

	/**
	 * \brief Data type modelling a "one of" type.
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
	 * \li Applicative (in L)
	 * \li Monad (in L)
	 *
	 * \note Either is \em not DefaultConstructible.
	 *
	 * \tparam L The "left" type
	 * \tparam R The "right" type (sometimes used to signal an error)
	 *
	 * \ingroup either
	 * \ingroup functor
	 * \ingroup applicative
	 * \ingroup monad
	 */
	// TODO: Specialise for L = void
	template<typename L, typename R>
	class either {
	public:
		/// The type either implements various concepts on
		using value_type = L;

		either() = delete;

		/// Copy c-tor
		either(const either& e)
		noexcept(  std::is_nothrow_copy_constructible<L>::value
				&& std::is_nothrow_copy_constructible<R>::value)
		: tag(e.tag) {
			if(tag == _dtl::LEFT)
				new (&l) L(e.l);
			else if(tag == _dtl::RIGHT)
				new (&r) R(e.r);
		}

		/// Move c-tor
		either(either&& e)
		noexcept(  std::is_nothrow_move_constructible<L>::value
				&& std::is_nothrow_move_constructible<R>::value)
		: tag(e.tag) {
			switch(tag) {
			case _dtl::LEFT:
				new (&l) L(std::move(e.l));
				break;

			case _dtl::RIGHT:
				new (&r) R(std::move(e.r));
				break;
			default:
				break;
			}

			e.tag = _dtl::LIMBO;
		}

		/**
		 * Construct a left value
		 */
		constexpr either(left_tag_t, const L& left)
		noexcept(std::is_nothrow_copy_constructible<L>::value)
		: l(left), tag(_dtl::LEFT) {
		}

		/**
		 * Move a left value
		 */
		constexpr either(left_tag_t, L&& left)
		noexcept(std::is_nothrow_copy_constructible<L>::value)
		: l(std::move(left)), tag(_dtl::LEFT) {
		}

		/// Construct a right type value
		explicit constexpr either(right_tag_t, const R& right)
		noexcept(std::is_nothrow_copy_constructible<R>::value)
		: r(right), tag(_dtl::RIGHT) {
		}

		/// Move a right value
		explicit constexpr either(right_tag_t, R&& right)
		noexcept(std::is_nothrow_move_constructible<R>::value)
		: r(std::move(right)), tag(_dtl::RIGHT) {
		}

		~either() {
			if(tag == _dtl::LEFT)
				l.~L();
			else if(tag == _dtl::RIGHT)
				r.~R();
		}

		/**
		 *  Check if the either instance contains the left type.
		 */
		constexpr bool isLeft() const noexcept {
			return tag == _dtl::LEFT;
		}

		/**
		 *  Check if the either instance contains the right type.
		 */
		constexpr bool isRight() const noexcept {
			return tag == _dtl::RIGHT;
		}

		/**
		 *  Get reference to left value.
		 *
		 *  \note Undefined behaviour if the either instance is
		 *  really of type R.
		 */
		L& left() noexcept {
			return l;
		}

		/**
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

		/**
		 * Convenience operator when using either for error handling.
		 *
		 * \return true if the instance is of \em left type.
		 */
		explicit constexpr operator bool () const noexcept {
			return tag == _dtl::LEFT;
		}

		/**
		 * Alternate, concise way of accessing left values.
		 *
		 * \throws std::logic_error if called on a right type.
		 */
		L& operator* () {
			if(tag == _dtl::LEFT)
				return l;

			throw std::logic_error(
					"Attempting to access 'left' value of right type.");
		}

		/// \overload
		const L& operator* () const {
			if(tag == _dtl::LEFT)
				return l;

			throw std::logic_error(
					"Attempting to access 'left' value of right type.");
		}

		/**
		 * Concise way of accessing member of left values.
		 *
		 * \throws std::logic_error if called one a right type.
		 */
		L* operator-> () {
			if(tag == _dtl::LEFT)
				return &l;

			throw std::logic_error(
					"Attempting to access 'left' value of right type.");
		}

		/// \overload
		const L* operator-> () const {
			if(tag == _dtl::LEFT)
				return &l;

			throw std::logic_error(
					"Attempting to access 'left' value of right type.");
		}

		const either& operator= (const either& e) {
			// Deal with self assignment
			if(this == &e)
				return *this;

			switch(tag) {
			case _dtl::LEFT:
				if(e.tag == _dtl::LEFT) 
					l = e.l;

				else {
					l.~L();
					tag = e.tag;

					if(tag == _dtl::RIGHT) {
						new (&r) R(e.r);
					}
				}
				break;

			case _dtl::RIGHT:
				if(e.tag == _dtl::RIGHT)
					r = e.r;

				else {
					r.~R();
					tag = e.tag;

					if(tag == _dtl::LEFT) {
						new (&l) L(e.l);
					}
				}
				break;

			case _dtl::LIMBO:
				tag = e.tag;

				if(tag == _dtl::RIGHT) {
					r = e.r;
				}

				else if(tag == _dtl::LEFT) {
					l = e.l;
				}
			}

			return *this;
		}

		const either& operator= (either&& e) {
			switch(tag) {
			case _dtl::LEFT:
				if(e.tag == _dtl::LEFT)
					l = std::move(e.l);

				else {
					l.~L();
					tag = e.tag;

					if(e.tag == _dtl::RIGHT) {
						new (&r) R(std::move(e.r));
					}

				}
				e.tag = _dtl::LIMBO;
				break;

			case _dtl::RIGHT:
				if(e.tag == _dtl::RIGHT)
					r = std::move(e.r);

				else {
					r.~R();
					tag = e.tag;

					if(e.tag == _dtl::LEFT)
						new (&l) L(std::move(e.l));
				}
				e.tag = _dtl::LIMBO;
				break;

			case _dtl::LIMBO:
				break;
			}

			return *this;
		}

		constexpr bool operator== (const either& e) {
			return tag == e.tag && tag != _dtl::LIMBO
				? (tag == _dtl::LEFT ? l == e.l : r == e.r)
				: false;
		}

		constexpr bool operator!= (const either& e) {
			return !(*this == e);
		}

	private:
		union {
			L l;
			R r;
		};

		_dtl::tag_t tag = _dtl::LIMBO;
	};

	/**
	 * Monad implementation for either.
	 *
	 * \ingroup functor
	 * \ingroup applicative
	 * \ingroup monad
	 * \ingroup either
	 */
	template<>
	struct monad<either> {
		template<typename A, typename R>
		static either<A,R> pure(const A& a) {
			return either<A,R>(left_tag_t(), a);
		}

		template<typename A, typename R>
		static either<A,R> pure(A&& a) {
			return either<A,R>(left_tag_t(), std::move(a));
		}

		template<
			typename F,
			typename A,
			typename R,
			typename B = typename decayed_result<F(A)>::type>
		static either<B,R> map(F f, const either<A,R>& e) {
			if(e.isLeft())
				return either<B,R>(left_tag_t(), f(e.left()));
			else
				return either<B,R>(right_tag_t(), e.right());
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
				return either<B,R>(right_tag_t(), e.right());
		}

		static constexpr bool instance = true;
	};

	/**
	 * Smart constructor of left values.
	 *
	 * Note that `L` can be deduced by the compiler by the value passed to
	 * make_left, hence you need only provide the `R` template parameter.
	 *
	 * example:
	 * \code
	 *   either<int,float> e = make_left<float>(12);
	 * \endcode
	 *
	 * \ingroup either
	 */
	template<
		typename R,
		typename _L,
		typename L = typename std::decay<_L>::type>
	constexpr either<L,R> make_left(_L&& l)
	noexcept(std::is_nothrow_constructible<either<L,R>,L>::value) {
		return either<L,R>(left_tag_t(), std::forward<_L>(l));
	}

	/**
	 * Smart constructor of right values.
	 *
	 * Note that `R` can be deduced by the compiler by the value passed to
	 * make_right, hence you need only provide the `L` template parameter.
	 *
	 * example:
	 * \code
	 *   either<int,float> e = make_right<int>(12.f);
	 * \endcode
	 *
	 * \ingroup either
	 */
	template<
		typename L,
		typename _R,
		typename R = typename std::decay<_R>::type>
	constexpr either<L,R> make_right(_R&& r)
	noexcept(std::is_nothrow_constructible<either<L,R>,R>::value) {
		return either<L,R>(right_tag_t(), std::forward<_R>(r));
	}

}

#endif

