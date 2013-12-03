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

#include <stdexcept>
#include "concepts/orderable.h"
#include "concepts/monad.h"

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
	 * The following additional headers and modules are included by this module.
	 * - <stdexcept>
	 * - \ref monad
	 * - \ref orderable
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
	 * Put simply, an instance of `either<L,R>` can store a value of
	 * either type L, or type R, but not both at the same time.
	 *
	 * Either fulfills the following concepts if, and only if,
	 * _both_ of its sub-types also do:
	 *  
	 * - \ref movecons
	 * - \ref assignable
	 * - \ref eq
	 * - \ref orderablepg (compares left to right)
	 *
	 * Either fulfills the following concepts regardless of its sub-types:
	 *
	 * - \ref copycons
	 * - \ref deref
	 * - \ref empty (when containing an `L`)
	 * - \ref functor (in R)
	 * - \ref applicative (in R)
	 * - \ref monad (in R)
	 *
	 * \note Either is _not_ \ref defcons.
	 *
	 * \tparam L The "left" type, must satisfy \ref copycons
	 * \tparam R The "right" type , must satisfy \ref copycons
	 *
	 * \ingroup either
	 */
	// TODO: Specialise for R = void
	template<typename L, typename R>
	class either {
	public:
		/// Convenient access to Left type
		using left_type = L;

		/// Convenient access to Right type
		using right_type = R;

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
		constexpr either(right_tag_t, const R& right)
		noexcept(std::is_nothrow_copy_constructible<R>::value)
		: r(right), tag(_dtl::RIGHT) {
		}

		/// Move a right value
		constexpr either(right_tag_t, R&& right)
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
		 * \return true if the instance is of \em right type.
		 */
		explicit constexpr operator bool () const noexcept {
			return tag == _dtl::RIGHT;
		}

		/**
		 * Alternate, concise way of accessing right values.
		 *
		 * \throws std::logic_error if called on a left type.
		 */
		R& operator* () {
			if(tag == _dtl::RIGHT)
				return r;

			throw std::logic_error(
					"Attempting to access 'right' value of left type.");
		}

		/// \overload
		const R& operator* () const {
			if(tag == _dtl::RIGHT)
				return r;

			throw std::logic_error(
					"Attempting to access 'right' value of right left.");
		}

		/**
		 * Concise way of accessing member of right values.
		 *
		 * \throws std::logic_error if called on a left type.
		 */
		R* operator-> () {
			if(tag == _dtl::RIGHT)
				return &r;

			throw std::logic_error(
					"Attempting to access 'right' value of left type.");
		}

		/// \overload
		const R* operator-> () const {
			if(tag == _dtl::RIGHT)
				return &r;

			throw std::logic_error(
					"Attempting to access 'right' value of left type.");
		}

		either& operator= (const either& e) {
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

		either& operator= (either&& e) {
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

		constexpr bool operator== (const either& e) const noexcept {
			return tag == e.tag && tag != _dtl::LIMBO
				? (tag == _dtl::LEFT ? l == e.l : r == e.r)
				: false;
		}

		constexpr bool operator!= (const either& e) const noexcept {
			return !(*this == e);
		}

	private:
		union {
			L l;
			R r;
		};

		_dtl::tag_t tag = _dtl::LIMBO;
	};

	template<typename L, typename R>
	struct parametric_type_traits<either<L,R>> {
		using value_type = R;

		template<typename S>
		using rebind = either<L,S>;
	};

	/**
	 * Monad implementation for either.
	 *
	 * \ingroup either
	 */
	template<typename L, typename T>
	struct monad<either<L,T>>
	: deriving_join<in_terms_of_bind<either<L,T>>>
	, deriving_apply<in_terms_of_bind<either<L,T>>> {

		/**
		 * Embeds a value as a right value.
		 *
		 * Example:
		 * \code
		 *   either<int,string> e = monad<either<int,string>>::pure("foo");
		 *
		 *  // Exactly equivalent of
		 *  auto e = make_right<int>(std::string("foo"));
		 * \endcode
		 */
		static constexpr either<L,T> pure(const T& t)
		noexcept(std::is_nothrow_copy_constructible<T>::value) {
			return either<L,T>(right_tag_t(), t);
		}

		/// \overload
		static constexpr either<L,T> pure(T&& t)
		noexcept(std::is_nothrow_move_constructible<T>::value) {
			return either<L,T>(right_tag_t(), std::move(t));
		}

		/**
		 * Apply `f` to right values.
		 *
		 * If `e` is a left value, it's simply passed on without any
		 * modification. However, if `e` is a right value, `f` is applied and
		 * its result is what's passed on.
		 *
		 * Example:
		 * \code
		 *   auto e1 = ftl::make_right<int>(string("hello"));
		 *   auto e2 = ftl::make_left<string>(0);
		 *
		 *   // e3 == ftl::make_right<int>(string("hello world!"));
		 *   auto e3 = ftl::fmap([](string s){ return s + " world!"; }, e1);
		 *   
		 *   // e4 == ftl::make_left<string>(0);
		 *   auto e4 = ftl::fmap([](string s){ return s + " world!"; }, e2);
		 * \endcode
		 */
		template<typename F, typename U = result_of<F(T)>>
		static either<L,U> map(F&& f, const either<L,T>& e) {
			if(e)
				return either<L,U>(right_tag_t(), std::forward<F>(f)(*e));
			else
				return either<L,U>(left_tag_t(), e.left());
		}

		/// \overload
		template<typename F, typename U = result_of<F(T)>>
		static either<L,U> map(F&& f, either<L,T>&& e) {
			if(e)
				return either<L,U>(
						right_tag_t(),
						std::forward<F>(f)(std::move(*e))
				);
			else
				return either<L,U>(left_tag_t(), std::move(e.left()));
		}

		/**
		 * Bind `e` with the monadic computation `f`.
		 *
		 * If `e` is a right value, then it's extracted and passed to `f`,
		 * the result of which is the monadic action returned by `bind`. If
		 * `e` is a left value however, `f` is never invoked; `e.left()` is
		 * simply passed on.
		 *
		 * \tparam F must satisy \ref fn`<either<L,U>(T)>` where `U` is any
		 *           type that can be contained in an `either`.
		 */
		template<
				typename F,
				typename U = typename result_of<F(T)>::right_type
		>
		static either<L,U> bind(const either<L,T>& e, F&& f) {
			if(e)
				return std::forward<F>(f)(*e);
			else
				return either<L,U>(left_tag_t(), e.left());
		}

		/// \overload
		template<
				typename F,
				typename U = typename result_of<F(T)>::right_type
		>
		static either<L,U> bind(either<L,T>&& e, F&& f) {
			if(e)
				return std::forward<F>(f)(std::move(*e));
			else
				return either<L,U>(left_tag_t(), std::move(e.left()));
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
			typename L_,
			typename L = typename std::decay<L_>::type
	>
	constexpr either<L,R> make_left(L_&& l)
	noexcept(std::is_nothrow_constructible<either<L,R>,L>::value) {
		return either<L,R>(left_tag_t(), std::forward<L_>(l));
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
			typename R_,
			typename R = plain_type<R_>
	>
	constexpr either<L,R> make_right(R_&& r)
	noexcept(std::is_nothrow_constructible<either<L,R>,R_>::value) {
		return either<L,R>(right_tag_t(), std::forward<R_>(r));
	}

	/**
	 * Less than comparison of two equivalently parametrised eithers.
	 *
	 * Left values are always smaller than right values, and when both lean the
	 * same way, the comparison operator of that type is used.
	 *
	 * Example:
	 * \code
	 *   either<int,char> el4 = ftl::make_left<char>(4);
	 *   either<int,char> el5 = ftl::make_left<char>(5);
	 *   either<int,char> er1 = ftl::make_right<int>(1);
	 *   either<int,char> er2 = ftl::make_right<int>(2);
	 *
	 *   // These comparisons all result in true
	 *   el4 < el5 && el4 < er1 && er1 < er2;
	 *
	 *   // These all result in false
	 *   el5 < el4 && er1 < el4 && er2 < er1 && el4 < el4;
	 * \endcode
	 *
	 * \ingroup either
	 */
	template<
			typename L, typename R,
			typename = Requires<Orderable<L>() && Orderable<R>()>
	>
	constexpr bool operator< (const either<L,R>& e1, const either<L,R>& e2)
	noexcept {
		return e1.isLeft()
			? (e2.isLeft() ? e1.left() < e2.right() : true)
			: (e2.isRight() ? e1.right() < e2.right() : false);
	}

	/**
	 * Greater than comparison of two equivalently parametrised eithers.
	 *
	 * Left values are always smaller than right values, and when both lean the
	 * same way, the comparison operator of that type is used.
	 *
	 * Example:
	 * \code
	 *   either<int,char> el4 = ftl::make_left<char>(4);
	 *   either<int,char> el5 = ftl::make_left<char>(5);
	 *   either<int,char> er1 = ftl::make_right<int>(1);
	 *   either<int,char> er2 = ftl::make_right<int>(2);
	 *
	 *   // These comparisons all result in true
	 *   el5 > el4 && er1 > el4 && er2 > er1;
	 *
	 *   // These all result in false
	 *   el4 > el5 && el4 > er1 && er1 > er2 && el4 > el4;
	 *
	 * \endcode
	 *
	 * \ingroup either
	 */
	template<
			typename L, typename R,
			typename = Requires<Orderable<L>() && Orderable<R>()>
	>
	constexpr bool operator> (const either<L,R>& e1, const either<L,R>& e2)
	noexcept {
		return e1.isLeft()
			? (e2.isLeft() ? e1.left() > e2.right() : false)
			: (e2.isRight() ? e1.right() > e2.right() : true);
	}

}

#endif

