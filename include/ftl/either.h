/*
 * Copyright (c) 2013, 2016 Björn Aili
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

#include "sum_type.h"
#include "concepts/orderable.h"
#include "concepts/monad.h"

namespace ftl {

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
	 * - \ref sum_type
	 * - \ref monad
	 * - \ref orderable
	 */

	template<typename T>
	struct Left;

	/**
	 * One of the inner types of an either instance.
	 *
	 * \see Identity
	 *
	 * \ingroup either
	 */
	template<typename T>
	using Right = Identity<T>;

	/**
	 * \brief Data type modelling a "one of" type.
	 *
	 * Put simply, an instance of `either<L,R>` can store a value of
	 * either type L, or type R, but not both at the same time.
	 *
	 * Perhaps most commonly, `either` is used as a means of error handling,
	 * sometimes even as a substitute for exceptions (using the monadic
	 * combinators). Simply put, a function that can somehow fail may choose
	 * to return `either< ErrorType, ResultType >`. It is de facto standard that
	 * `Right` values are returned on success and `Left` values on failure, and
	 * either's monad instance reflects this.
	 *
	 * While `Left` is strongly typed and does not allow neither implicit casts
	 * from, nor to its inner type, `Right` does implicitly cast _to_ its
	 * inner type. It is also implicitly convertible to both `R&` and
	 * `R const&`.
	 *
	 * \par Type Traits & Concepts
	 *
	 * Either is technically the same as `sum_type<Left<L>,Right<R>>` and thus
	 * satisfies all the same basic traits and concepts. In addition, `either`
	 * satisfies the following concepts:
	 * - \ref functor (in `R`)
	 * - \ref applicative (in `R`)
	 * - \ref monad (in `R`)
	 *
	 * \par Examples
	 *
	 * Basic pattern matching:
	 * \code
	 *   either<int,int> e = make_right<int>(10);
	 *
	 *   int x = e.match(
	 *       [](Right<int> r){ return 2*r; },
	 *       [](Left<int> l){ return *l; }
	 *   );
	 *   // x == 20
	 * \endcode
	 *
	 * Effectful pattern matching:
	 * \code
	 *   either<int,int> e = make_right<int>(10);
	 *
	 *   e.match(
	 *       [](Right<int>& r){ *r *= 2; },
	 *       [](Left<int>& l){ ++(*l); }
	 *   );
	 *   // e == make_right<int>(20)
	 * \endcode
	 *
	 * \see sum_type
	 *
	 * \ingroup either
	 */
	template<typename L, typename R>
	class either : public sum_type<Left<L>,Right<R>>
	{
		using sum_type<Left<L>,Right<R>>::sum_type;
	};

	template<typename L, typename R>
	struct parametric_type_traits<either<L,R>> {
		using value_type = R;

		template<typename S>
		using rebind = either<L,S>;
	};

	/**
	 * One of the inner types of an either instance.
	 *
	 * For any type `U`, `Left<T>` is implicitly convertible to `either<T,U>`.
	 * This will copy or move the value out of the containing `Left`
	 * appropriately.
	 *
	 * \par Type Traits & Concepts
	 *
	 * - `Left<T>` has the same storage layout properties as `T`
	 * - `Left<T>` is a literal type, iff `T` is
	 * - `Left<T>` is constructible and assignable in the same manners `T` is
	 * - `Left<T>` has the same destructor properties as `T`
	 * - \ref deref, to `T`
	 * - \ref eq, iff `T` is
	 *
	 * \ingroup either
	 */
	template<class T>
	struct Left
	{
		using value_type = T;

		Left() = default;
		Left(const Left&) = default;
		Left(Left&&) = default;
		explicit constexpr Left(const T& t) : val(t) {}
		explicit constexpr Left(T&& t) : val(std::move(t)) {}
		~Left() = default;

		template<typename R>
		constexpr operator either<T,R>() const& noexcept
		{
			return either<T,R>{type<Left<T>>, val};
		}

		template<typename R>
		constexpr operator either<T,R>() && noexcept
		{
			return either<T,R>{type<Left<T>>, std::move(val)};
		}

		T& operator* () noexcept
		{
			return val;
		}

		constexpr const T& operator* () const noexcept
		{
			return val;
		}

		T* operator-> () noexcept
		{
			return std::addressof(val);
		}

		constexpr const T* operator-> () const noexcept
		{
			return std::addressof(val);
		}

		Left& operator= (const Left&) = default;
		Left& operator= (Left&&) = default;

		T val;
	};

	template<typename T, typename = Requires<Eq<T>::value>>
	constexpr auto operator== (const Left<T>& lhs, const Left<T>& rhs) noexcept
	-> decltype(std::declval<T>() == std::declval<T>())
	{
		return lhs.val == rhs.val;
	}

	template<typename T, typename = Requires<Eq<T>::value>>
	constexpr auto operator!= (const Left<T>& lhs, const Left<T>& rhs) noexcept
	-> decltype(std::declval<T>() != std::declval<T>())
	{
		return lhs.val != rhs.val;
	}

	/**
	 * Smart constructor of left values.
	 *
	 * Note that `L` can be deduced by the compiler by the value passed to
	 * make_left, hence you need only provide the `R` template parameter.
	 *
	 * \par Examples
	 *
	 * Basic usage:
	 * \code
	 *   either<int,float> e = make_left<float>(12);
	 * \endcode
	 *
	 * With type deduction:
	 * \code
	 *   auto e = make_left<float>(12);
	 * \endcode
	 *
	 * \ingroup either
	 */
	template<typename R, typename L, typename L0 = ::std::decay_t<L>>
	constexpr either<L0,R> make_left(L&& l)
	noexcept(std::is_nothrow_constructible<L0,L>::value) {
		return either<L0,R>{type<Left<L0>>, std::forward<L>(l)};
	}

	/**
	 * Smart constructor of right values.
	 *
	 * Note that `R` can be deduced by the compiler by the value passed to
	 * make_right, hence you need only provide the `L` template parameter.
	 *
	 * \par Examples
	 *
	 * Basic usage:
	 * \code
	 *   either<int,float> e = make_right<int>(12.f);
	 * \endcode
	 *
	 * With type deduction:
	 * \code
	 *   auto e = make_right<int>(12.f);
	 * \endcode
	 *
	 * \ingroup either
	 */
	template<typename L, typename R, typename R0 = ::std::decay_t<R>>
	constexpr either<L,R0> make_right(R&& r)
	noexcept(std::is_nothrow_constructible<R0,R>::value) {
		return either<L,R0>{type<Right<R0>>, std::forward<R>(r)};
	}

	/**
	 * Monad implementation for either.
	 *
	 * \ingroup either
	 */
	template<typename L, typename T>
	struct monad<either<L,T>>
	: deriving_join<in_terms_of_bind<either<L,T>>>
	, deriving_apply<in_terms_of_bind<either<L,T>>>
	{
		/**
		 * Embeds a value as a right value.
		 *
		 * \par Examples
		 *
		 * \code
		 *   either<int,string> e = monad<either<int,string>>::pure("foo");
		 *
		 *   // Exactly equivalent of
		 *   auto e = make_right<int>(std::string("foo"));
		 * \endcode
		 */
		static constexpr either<L,T> pure(const T& t)
		noexcept(std::is_nothrow_copy_constructible<T>::value) {
			return make_right<L>(t);
		}

		/// \overload
		static constexpr either<L,T> pure(T&& t)
		noexcept(std::is_nothrow_move_constructible<T>::value) {
			return make_right<L>(std::move(t));
		}

		/**
		 * Apply `f` to right values.
		 *
		 * If `e` is a left value, it's simply passed on without any
		 * modification. However, if `e` is a right value, `f` is applied and
		 * its result is what's passed on.
		 *
		 * \par Examples
		 *
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
		static either<L,U> map(F f, const either<L,T>& e)
		{
			return e.match(
				[f](const Right<T>& r){ return make_right<L>(f(*r)); },
				[](const Left<L>& l){ return make_left<U>(*l); }
			);
		}

		/// \overload
		template<typename F, typename U = result_of<F(T)>>
		static either<L,U> map(F f, either<L,T>&& e)
		{
			return e.match(
				[f](Right<T>& r){ return make_right<L>(f(std::move(*r))); },
				[](Left<L>& l){ return make_left<U>(std::move(*l)); }
			);
		}

		/**
		 * Bind `e` with the monadic computation `f`.
		 *
		 * If `e` is a right value, then it's extracted and passed to `f`,
		 * the result of which is the monadic action returned by `bind`. If
		 * `e` is a left value however, `f` is never invoked; the left value is
		 * simply passed on.
		 *
		 * \tparam F must satisy \ref fn`<either<L,U>(T)>` where `U` is any
		 *           type that can be contained in an `either`.
		 */
		template<typename F, typename U = Value_type<result_of<F(T)>>>
		static either<L,U> bind(const either<L,T>& e, F f)
		{
			return e.match(
				[](const Left<L>& l){ return make_left<U>(*l); },
				[f](const Right<T>& r){ return f(*r); }
			);
		}

		/// \overload
		template<typename F, typename U = Value_type<result_of<F(T)>>>
		static either<L,U> bind(either<L,T>&& e, F f)
		{
			return e.match(
				[](Left<L>& l){ return make_left<U>(std::move(*l)); },
				[f](Right<T>& r){ return f(std::move(*r)); }
			);
		}

		static constexpr bool instance = true;
	};
}

#endif

