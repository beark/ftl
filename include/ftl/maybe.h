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
#ifndef FTL_MAYBE_H
#define FTL_MAYBE_H

#include "sum_type.h"
#include "prelude.h"
#include "concepts/monoid.h"
#include "concepts/monad.h"
#include "concepts/foldable.h"

namespace ftl {

	/**
	 * \defgroup maybe Maybe
	 *
	 * The maybe data type and associated operations.
	 *
	 * \code
	 *   #include <ftl/maybe.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref sum_type
	 * - \ref prelude
	 * - \ref monoid
	 * - \ref monad
	 * - \ref foldable
	 */

	/**
	 * A type used to indicate the absence of a value in `maybe`s.
	 *
	 * Note that the presence of this type is the one thing that uniquely
	 * identifies a particular instance of `sum_type` as a `maybe`. Hence,
	 * any sum type of a form equivalent to `sum_type<Nothing,T>`, for any type
	 * `T`, will behave as if it were a `maybe<T>`.
	 *
	 * \par Concepts
	 * - \ref fullycons
	 * - \ref eq
	 *
	 * \ingroup maybe
	 */
	struct Nothing;

	/**
	 * A data type that may hold a value, or nothing.
	 *
	 * Note that only the presence of `Nothing` distinguishes `maybe` from
	 * other aliases of `sum_type`. In other words, if you were to create
	 * additional aliases parameterised equivalently, they will be treated
	 * identically to `maybe` with regards to concepts, and similar.
	 *
	 * \par Concepts
	 * - \ref copycons, if `T` is
	 * - \ref movecons, if `T` is
	 * - \ref assignable, if `T` is
	 * - \ref eq, if `T` is
	 * - \ref orderable, if `T` is (`Nothing` always compares less than anything
	 *        else)
	 *
	 * \see sum_type
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	using maybe = sum_type<T,Nothing>;

	/**
	 * Empty sub-type of maybe.
	 *
	 * Can be implicitly casted to any maybe type.
	 *
	 * \par Concepts
	 * - \ref fullycons
	 * - \ref eq
	 */
	struct Nothing {
		template<typename T>
		constexpr operator maybe<T>() const noexcept {
			return maybe<T>{constructor<Nothing>()};
		}
	};


	constexpr bool operator== (Nothing, Nothing) noexcept {
		return true;
	}

	constexpr bool operator!= (Nothing, Nothing) noexcept {
		return false;
	}

	template<typename T, typename = Requires<Orderable<T>{}>>
	bool operator< (const maybe<T>& m1, const maybe<T>& m2) noexcept {
		if(m1.template is<T>()) {
			if(m2.template is<T>()) {
				return get<T>(m1) < get<T>(m2);
			}
		}
		else {
			if(m2.template is<T>()) {
				return true;
			}

		}

		return false;
	}

	template<typename T, typename = Requires<Orderable<T>{}>>
	bool operator<= (const maybe<T>& m1, const maybe<T>& m2) noexcept {
		return !(m1 > m2);
	}

	template<typename T, typename = Requires<Orderable<T>{}>>
	bool operator> (const maybe<T>& m1, const maybe<T>& m2) noexcept {
		if(m1.template is<T>()) {
			if(m2.template is<T>()) {
				return get<T>(m1) > get<T>(m2);
			}
		}
		else {
			return false;
		}

		return true;
	}

	template<typename T, typename = Requires<Orderable<T>{}>>
	bool operator>= (const maybe<T>& m1, const maybe<T>& m2) noexcept {
		return !(m1 < m2);
	}


#ifndef DOCUMENTATION_GENERATOR
	constexpr struct just_ {
		template<typename T, typename T0 = plain_type<T>>
		constexpr maybe<T0> operator()(T&& t) const
		noexcept(std::is_nothrow_constructible<T0,T>::value) {
			return maybe<T0>{constructor<T0>(), std::forward<T>(t)};
		}
	} just{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Convenience constructor for a `maybe` containing a value.
	 *
	 * Acts like a function of type
	 * \code
	 *   (T) -> maybe<T>
	 * \endcode
	 *
	 * Perfectly forwards the argument to `maybe<T>`'s, and therefore `T`'s
	 * constructor.
	 *
	 * \ingroup maybe
	 */
	just;
#endif

	// TODO: C++14: change to a constexpr template variable
	/**
	 * Convenience function to create an empty `maybe` value.
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	constexpr maybe<T> nothing() noexcept {
		return maybe<T>{constructor<Nothing>()};
	}

	template<typename T>
	struct parametric_type_traits<maybe<T>> {
		using value_type = T;

		template<typename U>
		using rebind = maybe<U>;
	};

	/**
	 * Maybe's monad instance.
	 *
	 * An intuitive way to reason about `maybe`'s monad implementation is that
	 * it behaves as if it were a regular container, such as `list` or `vector`,
	 * except it can at most hold one value. The monadic operations then become
	 * easier to remember.
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	struct monad<maybe<T>> {
		/**
		 * Embeds a value in the `maybe` context/container.
		 *
		 * Exactly equivalent to invoking `just` on the value.
		 */
		static constexpr maybe<T> pure(const T& t)
		noexcept(std::is_nothrow_copy_constructible<T>::value) {
			return just(t);
		}

		/// \overload
		static constexpr maybe<T> pure(T&& t)
		noexcept(std::is_nothrow_move_constructible<T>::value) {
			return just(std::move(t));
		}

		// TODO: C++14: capture f with forwarding instead of copy
		/**
		 * Maybe maps a function to a contained value.
		 *
		 * From the perspective of considering maybe a 0 or 1 element container,
		 * behaves exactly as expected: if "empty" (`Nothing`), the function is
		 * never called and the result is another "empty" container; if there is
		 * an element, `f` is applied on it and the result is embedded as if by
		 * `pure`.
		 *
		 * \par Examples
		 *
		 * Successful computation:
		 * \code
		 *   auto x = just(12);
		 *   auto y = fmap([](int x){ return std::to_string(x); }, x);
		 *   // y == just(std::string("12"))
		 * \endcode
		 *
		 * Computation with failure:
		 * \code
		 *   auto x = nothing<int>();
		 *   auto r = fmap([](int x){ return std::to_string(x); }, x);
		 *   // y == nothing<std::string>()
		 * \endcode
		 */
		template<typename F, typename U = result_of<F(T)>>
		static constexpr maybe<U> map(F f, const maybe<T>& m) {
			return m.match(
				[f](const T& t){ return just(f(t)); },
				[](Nothing){ return Nothing{}; }
			);
		}

		// TODO: when sum_type has r-value overload on match, move stuff
		/// \overload
		template<typename F, typename U = result_of<F(T)>>
		static constexpr maybe<U> map(F f, maybe<T>&& m) {
			return m.match(
				[f](T& t){ return just(f(t)); },
				[](Nothing){ return Nothing{}; }
			);
		}

		// TODO: Move version for mf when sum_type has r-value match
		// TODO: C++14: capture m by copy/move as appropriate
		/**
		 * Apply a contained function to a contained value and embed the result.
		 *
		 * Very similar to `map`, except this time the function is embedded in a
		 * `maybe` as well.
		 *
		 * \par Examples
		 *
		 * Successful computation:
		 * \code
		 *   auto f = just([](int x){ return 2*x; });
		 *   auto m = just(10);
		 *   auto r = aapply(f, m);
		 *   // r == just(20)
		 * \endcode
		 *
		 * Computation with failure:
		 * \code
		 *   auto f = just([](int x){ return 2*x; });
		 *   auto m = nothing<int>();
		 *   auto r = aapply(f, m);
		 *   // r == nothing<int>()
		 * \endcode
		 */
		template<typename F, typename U = result_of<F(T)>>
		static constexpr maybe<U> apply(const maybe<F>& mf, maybe<T> m) {
			return mf.match(
				[m](const F& f){ return fmap(f, m); },
				[](Nothing){ return Nothing{}; }
			);
		}

		// TODO: C++14: capture f by forwarding
		/// \overload
		template<typename F, typename U = Value_type<result_of<F(T)>>>
		static constexpr maybe<U> bind(const maybe<T>& m, F f) {
			return m.match(
				[f](const T& t){ return f(t); },
				[](Nothing){ return Nothing{}; }
			);
		}

		/**
		 * Extract `m`'s value and forward to `f` if non-`Nothing`.
		 *
		 * \par Examples
		 *
		 * Successful computation:
		 * \code
		 *   auto f = [](int x){ return std::to_string(x); };
		 *   auto m = just(10);
		 *   auto r = mbind(m, f);
		 *   // r == just(std::string("10"))
		 * \endcode
		 *
		 * Computation with failure:
		 * \code
		 *   auto f = [](int x){ return nothing<std::string>(); };
		 *   auto m = just(10);
		 *   auto r = mbind(m, f);
		 *   // r == nothing<std::string>()
		 * \endcode
		 */
		template<typename F, typename U = Value_type<result_of<F(T)>>>
		static constexpr maybe<U> bind(maybe<T>&& m, F f) {
			return m.match(
				[f](T& t){ return f(std::move(t)); },
				[](Nothing){ return Nothing{}; }
			);
		}

		// TODO: Move version for m when sum_type has r-value match
		/// \overload
		static constexpr maybe<T> join(const maybe<maybe<T>>& m) {
			return m.match(
				[](maybe<T> m){ return m; },
				[](Nothing) { return Nothing{}; }
			);
		}

		static constexpr bool instance = true;
	};

	/**
	 * Foldable implementation for `maybe`.
	 *
	 * Behaves as if `maybe` was a container of 0 or 1 elements. In other words,
	 * folding `Nothing` is exactly the same as folding an empty list, and
	 * folding on `just(something)` is the same as folding a list with one
	 * element.
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	struct foldable<maybe<T>>
	: deriving_foldMap<maybe<T>>, deriving_fold<maybe<T>> {
		template<typename F, typename U>
		static constexpr plain_type<U> foldl(F&& f, U&& z, const maybe<T>& m)
		noexcept(noexcept(f(std::declval<plain_type<U>>(), std::declval<T>())))
		{
			static_assert(
				std::is_convertible<
					typename std::result_of<F(U,T)>::type,U
				>::value,
				"The result of F(U,T) must be convertible to U"
			);

			return m.template is<Nothing>()
				? z
				: std::forward<F>(f)(std::forward<U>(z), get<T>(m));
		}

		template<typename F, typename U>
		static constexpr plain_type<U> foldr(F&& f, U&& z, const maybe<T>& m)
		noexcept(noexcept(f(std::declval<plain_type<U>>(), std::declval<T>())))
		{
			static_assert(
				std::is_convertible<
					typename std::result_of<F(T,U)>::type,U
				>::value,
				"The result of F(T,U) must be convertible to U"
			);

			return m.template is<Nothing>()
				? z
				: std::forward<F>(f)(get<T>(m), std::forward<U>(z));
		}

		static constexpr bool instance = true;
	};

	/**
	 * An optional computation.
	 *
	 * If `f` fails, the `optional` computation as a whole "succeeds" but yields
	 * `Nothing`, whereas it otherwise yields `just(x)` where `x` is the
	 * computed result of `f`.
	 *
	 * \tparam F must be an instance of `ftl::monoidA`
	 *
	 * \par Examples
	 *
	 * \ingroup maybe
	 */
	template<
			typename F,
			typename T = Value_type<F>,
			typename = Requires<MonoidAlt<F>{}>
	>
	Rebind<F,maybe<T>> optional(const F& f) {
		using Fm = Rebind<F,maybe<T>>;
		return just % f | applicative<Fm>::pure(nothing<T>());
	}
}

#endif

