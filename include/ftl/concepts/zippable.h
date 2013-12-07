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
#ifndef FTL_ZIPPABLE_H
#define FTL_ZIPPABLE_H

#include <tuple>
#include "../prelude.h"
#include "common.h"

namespace ftl {
	/**
	 * \page zippablepg Zippable
	 *
	 * Data structures that can be element-wise "zipped" together.
	 *
	 * The classic example of a data type that is Zippable is the list. Zipping
	 * two lists together would produce a third list, where each element is a
	 * pair containing the corresponding elements of the original lists.
	 *
	 * \par Laws
	 * - `zipWith(const_, a, b) == a`, for any `a` and `b` where
	 *   `length(b) >= length(a)`
	 * - Given the zipped value `z = zipWith(f, a, b)`, for any `f` lacking
	 *   side effects, then `length(z) == min(length(a), length(b))`
	 *
	 * \see \ref zippable (module)
	 */

	/**
	 * \defgroup zippable Zippable
	 *
	 * Functions and interface relating to the \ref zippablepg concept.
	 *
	 * \code
	 *   #include <ftl/concepts/zippable.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - `<tuple>`
	 * - \ref prelude
	 */

	/**
	 * \interface zippable
	 *
	 * Concrete definition of the zippable concept.
	 *
	 * The most typical types that are instances of zippable are containers of
	 * different sorts, in which case the `zip` operation simply iterates one
	 * element at a time from each container and "zips" them together, commonly
	 * with the tuple constructor.
	 *
	 * Note that any \ref applicablepg can be made zippable with the
	 * following `zipWith` implementation:
	 * \code
	 *   auto zipWith(F f, const A& a, const A& b) {
	 *       // aapply(fmap(f, a), b)
	 *       return f % a * b;
	 *   }
	 * \endcode
	 *
	 * \ingroup zippable
	 */
	template<typename Z_>
	struct zippable {
		/// Element/value type of the zippable
		using T = Value_type<Z_>;

		template<typename U>
		using Z = Rebind<Z_,U>;

// Just as in monad, we don't want the compiler to find these, but the API
// reference generator should.
#ifdef DOCUMENTATION_GENERATOR

		/**
		 * Zips together two zippables using a function.
		 *
		 * Picks elements from `z1` and `z2` one at a time and combines them
		 * using `f`. The result is simply the collection of these combinations.
		 */
		template<typename U, typename F, typename V = result_of<F(T,U)>>
		static Z<V> zipWith(F f, const Z<T>& z1, const Z<U>& z2);

#endif

		/// Compile time constant to check if a type is an instance.
		static constexpr bool instance = false;
	};

	/**
	 * Concepts lite-compatible check for zippable instances.
	 *
	 * Can also be used with SFINAE, for much the same purposes.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<
	 *       typename Z,
	 *       typename = Requires<Zippable<Z>()>
	 *   >
	 *   void foo(const Z& z) {
	 *       // Perform zips on z
	 *   }
	 * \endcode
	 *
	 * \ingroup zippable
	 */
	template<typename Z>
	constexpr bool Zippable() noexcept {
		return zippable<Z>::instance;
	}

	template<typename>
	struct deriving_zippable {};

	/**
	 * Inhertiable zippable implementation for many container types.
	 *
	 * `Z`  must fulfull the following contract:
	 * - \ref defcons
	 * - \ref fwditerable
	 * - Must satisfy the `has_push_back` type trait
	 *
	 * Any type satisfying this contract may derive its `zippable` instance
	 * from this implementation. Note that a manual implementation can be
	 * preferable due to details that cannot be known by this generalised
	 * implementation.
	 *
	 * Note that deriving this implementation results in a `zipWith` that for
	 * second parameter accepts an instance of any type that satisfies
	 * \ref fwditerable. In other words, it would be possible to zip a list
	 * deriving this implementation with e.g. a `maybe<SomeType>`.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct zippable<Container<T>>
	 *   : deriving_zippable<back_insertable_container<Container<T>>> {};
	 * \endcode
	 *
	 * \ingroup zippable
	 */
	template<typename Z>
	struct deriving_zippable<back_insertable_container<Z>> {
		using T = Value_type<Z>;

		template<typename U>
		using Z_ = Rebind<Z,U>;

		template<
				typename F, typename Iterable,
				typename U = result_of<F(T,Value_type<Iterable>)>,
				typename = Requires<
					ForwardIterable<Iterable>()
				>
		>
		static Z_<U> zipWith(F f, const Z_<T>& z, const Iterable& i) {
			auto it1 = z.begin();
			auto it2 = i.begin();

			Z_<U> result;

			while(it1 != z.end() && it2 != i.end()) {
				result.push_back(f(*it1, *it2));
				++it1; ++it2;
			}

			return result;
		}

		static constexpr bool instance = true;

	};

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _zipWith : private _dtl::curried_ternf<_zipWith>
	{
		template<
				typename F, typename Z, typename I,
				typename = Requires<Zippable<Z>()>
		>
		auto operator() (F&& f, const Z& z, const I& i) const
		-> decltype(zippable<Z>::zipWith(std::forward<F>(f), z, i)) {
			return zippable<Z>::zipWith(std::forward<F>(f), z, i);
		}

		using curried_ternf<_zipWith>::operator();
	} zipWith{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Function object representing `zippable::zipWith`.
	 *
	 * Behaves as if it were a curried function of type
	 * \code
	 *   ((T,U) -> V, Z<T>, Z<U>) -> Z<V>
	 * \endcode
	 *
	 * Allows a clean and terse call syntax of the concept method.
	 *
	 * \par Examples
	 *
	 * \code
	 *   auto z = ftl::zipWith(foo, std::list{...}, std::list{...});
	 * \endcode
	 *
	 * \ingroup zippable
	 */
	zipWith;
#endif

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _zip : private _dtl::curried_binf<_zip> {
	private:
		template<typename T, typename U>
		struct mktup {
			std::tuple<T,U> operator() (const T& t, const U& u) const {
				return std::make_tuple(t, u);
			}

			std::tuple<T,U> operator() (T&& t, const U& u) const {
				return std::make_tuple(std::move(t), u);
			}

			std::tuple<T,U> operator() (const T& t, U&& u) const {
				return std::make_tuple(t, std::move(u));
			}

			std::tuple<T,U> operator() (T&& t, U&& u) const {
				return std::make_tuple(std::move(t), std::move(u));
			}
		};

	public:
		template<
				typename Z, typename I,
				typename = Requires<Zippable<Z>()>
		>
		auto operator() (const Z& z, const I& i) const
		-> decltype(zippable<Z>::zipWith(mktup<Value_type<Z>,Value_type<I>>{}, z, i)) {

			return zippable<Z>::zipWith(mktup<Value_type<Z>,Value_type<I>>{}, z, i);
		}

		using curried_binf<_zip>::operator();
	} zip{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Function object essentially equivalent of `zipWith(std::make_tuple)`.
	 *
	 * Functionally equivalent of a curried function of type
	 * \code
	 *   (Z<T>, Z<U>) -> Z<tuple<T,U>>
	 * \endcode
	 *
	 * In other words, zipping two lists with this function object results in
	 * a list containing pairs of elements.
	 *
	 * \par Examples
	 *
	 * \code
	 *   auto pairs = ftl::zip(list<int>{1,2,3}, list<string>{"a","b"});
	 *   // list<tuple<int,string>>{make_tuple(1,"a"), make_tuple(2,"b")}
	 * \endcode
	 *
	 * \ingroup zippable
	 */
	zip;
#endif
}

#endif


