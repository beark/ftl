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
#ifndef FTL_FOLDABLE_H
#define FTL_FOLDABLE_H

#include "monoid.h"
#include "../prelude.h"
#include "common.h"

namespace ftl {
	/**
	 * \page foldablepg Foldable
	 *
	 * Abstraction of data structures that can be folded to some accumulated
	 * value.
	 *
	 * Despite what one may be lead to believe, this concept does not quite
	 * correspond to the mathematical notion of a catamorphism. Unlike a _true_
	 * catamorphism, the `Foldable` concept can only traverse structures
	 * "linearly". I.e., it is impossible to use a fold to build an isomorphic
	 * representation of a tree.
	 *
	 * For a concrete definition of what an instance must fulfill, see the
	 * `ftl::foldable` interface.
	 *
	 * \see \ref foldable (module)
	 */

	/**
	 * \defgroup foldable Foldable
	 *
	 * Interface and utilities relating to the \ref foldablepg concept.
	 *
	 * \code
	 *   #include <ftl/concepts/foldable.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref prelude
	 * - \ref monoid
	 */

	/**
	 * \interface foldable
	 *
	 * Concrete definition of the foldable concept.
	 *
	 * The most typical types that are instances of foldable are containers of
	 * different sorts, in which case the fold operation is usually a
	 * generalisation of a "sum" operation on the elements of the container.
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct foldable {
		/// Typedef for easier reference to the type to fold on
		using T = Value_type<F>;

// Just as in monad, we don't want the compiler to find these, but the API
// reference generator should.
#ifdef DOCUMENTATION_GENERATOR
		/**
		 * Fold a structure containing a monoidal type.
		 *
		 * Examples:
		 * \code
		 *   std::vector<ftl::sum_monoid<int>> v{sum(2), sum(4), sum(10)};
		 *   ftl::fold(v); // sum(16)
		 * \endcode
		 *
		 * \tparam T must be a \ref monoid
		 */
		template<typename = Requires<Monoid<T>()>>
		static T fold(const F& f);

		/**
		 * Map each element to a monoid and fold the result.
		 *
		 * Example:
		 * \code
		 *   std::list<int> l{2,2,2};
		 *
		 *   ftl::foldMap(ftl::prod<int>, l); // prod(8)
		 * \endcode
		 *
		 * \tparam Fn must satisfy \ref fn`<`\ref monoidpg`(A)>`
		 */
		template<
				typename Fn,
				typename M = typename std::result_of<Fn(T)>::type,
				typename = Requires<Monoid<M>()>
		>
		static M foldMap(Fn&& fn, const F& f);

		/**
		 * Right associative fold.
		 *
		 * Example:
		 * \code
		 *   std::forward_list<int> l{4, 8, 5};
		 *
		 *   // (4 - (8 - (5 - 3))) = -2
		 *   ftl::foldr([](int x, int y){ return x-y; }, 3, l);
		 * \endcode
		 *
		 * \param fn Binary folding function
		 * \param z Initial "zero" value to start/end the fold
		 * \param f Structure to fold
		 *
		 * \tparam Fn must satisfy \ref fn`<U(T,U)>`
		 */
		template<
				typename Fn,
				typename U,
				typename = Requires<
					std::is_convertible<U, result_of<Fn(T,U)>>::value
				>
		>
		static U foldr(Fn&& fn, U&& z, const F& f);

		/**
		 * Left associative fold.
		 *
		 * Example:
		 * \code
		 *   std::forward_list<int> l{4, 8, 5};
		 *
		 *   // (((3 - 4) - 8) - 5) = -14
		 *   ftl::foldl([](int x, int y){ return x-y; }, 3, l);
		 * \endcode
		 *
		 *
		 * \param fn Binary folding function
		 * \param z Zero value to combine first element with
		 * \param f Structure to fold
		 *
		 * \tparam Fn must satisfy \ref fn`<U(U,T)>`
		 */
		template<
				typename Fn,
				typename U,
				typename = Requires<
					std::is_convertible<U, result_of<Fn(U,T)>>::value
				>
		>
		static U foldl(Fn&& fn, U&& z, const F& f);

#endif

		/// Compile time constant to check if a type is an instance.
		static constexpr bool instance = false;
	};

	/**
	 * Concepts lite-compatible check for foldable instances.
	 *
	 * Can also be used with SFINAE, for much the same purposes.
	 *
	 * \par Examples
	 *
	 * Using implicit bool conversion:
	 * \code
	 *   template<
	 *       typename F,
	 *       typename = Requires<Foldable<F>{}>
	 *   >
	 *   void foo(const F& f) {
	 *       // Perform folds on f
	 *   }
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct Foldable {
		static constexpr bool value = foldable<F>::instance;

		constexpr operator bool() const noexcept {
			return value;
		}
	};

	template<typename>
	struct deriving_foldable {};
	
	/**
	 * An inheritable implementation of `foldable::foldl`.
	 *
	 * Any type that satisfies \ref fwditerable may have their \ref foldablepg
	 * instance simply inherit this implementation of `foldl` instead of
	 * implementing it manually.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct foldable<MyContainer<T>> : deriving_foldl<MyContainer<T>> {
	 *       // Rest of implementation
	 *   };
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct deriving_foldl {
		static_assert(
			ForwardIterable<F>(),
			"F does not satisfy ForwardIterable"
		);

		using T = Value_type<F>;

		template<typename Fn, typename U>
		static U foldl(Fn&& fn, U z, const F& f) {
			static_assert(
				std::is_convertible<
					typename std::result_of<Fn(U,T)>::type,U
				>::value,
				"The result of Fn(U,T) must be convertible to U"
			);

			for(auto& e : f) {
				z = fn(z, e);
			}

			return z;
		}

		template<typename Fn, typename U>
		static U foldl(Fn&& fn, U z, F&& f) {
			static_assert(
				std::is_convertible<
					typename std::result_of<Fn(U,T)>::type,U
				>::value,
				"The result of Fn(U,T) must be convertible to U"
			);

			for(auto& e : f) {
				z = fn(z, std::move(e));
			}

			return z;
		}
	};

	/**
	 * An inheritable implementation of `foldable::foldr`.
	 *
	 * Any type that satisfies ReverseIterable may have their \ref foldablepg
	 * instance simply inherit this implementation of `foldr` instead of
	 * implementing it manually.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct foldable<MyContainer<T>> : deriving_foldr<MyContainer<T>> {
	 *       // Rest of implementation
	 *   };
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct deriving_foldr {
		using T = Value_type<F>;

		template<typename Fn, typename U>
		static U foldr(Fn&& fn, U z, const F& f) {
			static_assert(
				std::is_convertible<
					typename std::result_of<Fn(T,U)>::type,U
				>::value,
				"The result of Fn(T,U) must be convertible to U"
			);

			for(auto it = f.rbegin(); it != f.rend(); ++it) {
				z = fn(*it, z);
			}

			return z;
		}

		template<typename Fn, typename U>
		static U foldr(Fn&& fn, U z, F&& f) {
			static_assert(
				std::is_convertible<
					typename std::result_of<Fn(T,U)>::type,U
				>::value,
				"The result of Fn(T,U) must be convertible to U"
			);

			for(auto it = f.rbegin(); it != f.rend(); ++it) {
				z = fn(std::move(*it), z);
			}

			return z;
		}
	};

	/**
	 * Inheritable implementation of `foldable::foldMap`.
	 *
	 * Foldable instances implementing `foldable::foldl` can simply inherit from
	 * this struct to get `foldable::foldMap` for "free". Naturally, it works
	 * together with `ftl::deriving_foldl`.
	 *
	 * \par Examples
	 *
	 * \code
	 *     template<>
	 *     struct foldable<MyFoldable> : deriving_foldMap<MyFoldable> {
	 *         // Implementation of foldl and foldr
	 *     };
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct deriving_foldMap {
		template<
				typename Fn,
				typename T = Value_type<F>,
				typename M = result_of<Fn(T)>
		>
		static M foldMap(Fn fn, const F& f) {
			static_assert(
				Monoid<M>(),
				"The result of Fn(T) is not an instance of Monoid."
			);

			return foldable<F>::foldl(
					[fn](const T& a, const M& b) {
						return monoid<M>::append(
							fn(a),
							b);
					},
					monoid<M>::id(),
					f);
		}
	};

	/**
	 * Inheritable implementation of foldable::fold.
	 *
	 * Foldable specialisations implementing foldable::foldMap can inherit from
	 * this struct to get `foldable::fold` for "free".
	 *
	 * It is entirely possible for a foldable implementation to use both 
	 * `deriving_foldMap<F>` and `deriving_fold<F>`, even in reverse order.
	 *
	 * \par Examples
	 *
	 * \code
	 *   template<typename T>
	 *   struct foldable<myListType<T>>
	 *   : deriving_foldMap<myListType<T>>, deriving_fold<myListType<T>> {
	 *       
	 *       // Implementations of foldl and foldr
	 *   };
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct deriving_fold {
		template<typename M = Value_type<F>>
		static M fold(const F& f) {
			static_assert(Monoid<M>(), "M must satisfy Monoid");

			return foldable<F>::foldMap(id, f);
		}
	};

	/**
	 * Inheritable implementation of the _full_ \ref foldablepg concept.
	 *
	 * Requires that the instance type is BidirectionalIterable.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   struct foldable<MyContainer<T>>
	 *   : deriving_foldable<bidirectional_iterable<MyContainer<T>>> {};
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	template<typename F>
	struct deriving_foldable<bidirectional_iterable<F>>
	: deriving_foldl<F>, deriving_foldr<F>, deriving_fold<F>
	, deriving_foldMap<F> {
		static constexpr bool instance = true;
	};

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _fold {
		template<
				typename F,
				typename M = Value_type<F>,
				typename = Requires<Foldable<F>() && Monoid<M>()>
		>
		M operator() (const F& f) const {
			return foldable<F>::fold(f);
		}

	} fold {};
#else
	struct ImplementationDefined {
	}
	/**
	 * Convenience function object for `foldable<T>::fold`.
	 *
	 * Behaves as though it was a function of type
	 * \code
	 *   (F<M>) -> M
	 * \endcode
	 * where `M` satisfies \ref monoidpg.
	 *
	 * Example:
	 * \code
	 *   list<list<ftl::sum_monoid<int>>> l{
	 *       {ftl::sum(1), ftl::sum(2)},
	 *       {ftl::sum(3), ftl::sum(4)}
	 *   };
	 *
	 *   auto r = ftl::fmap(ftl::fold, l);
	 *   // r == {sum(3), sum(7)}
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	fold;
#endif

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _foldMap : public _dtl::curried_binf<_foldMap> {
		template<
				typename F,
				typename T = Value_type<F>,
				typename Fn,
				typename = Requires<Foldable<F>{}>
		>
		auto operator() (Fn&& fn, const F& f) const
		-> decltype(foldable<F>::foldMap(std::forward<Fn>(fn), f)) {
			return foldable<F>::foldMap(std::forward<Fn>(fn), f);
		}

		using curried_binf<_foldMap>::operator();
	} foldMap {};
#else
	struct ImplementationDefined {
	}
	/**
	 * Function object representing `foldable::foldMap`.
	 *
	 * Behaves like a curried function of type
	 * \code
	 *   ((A) -> M, F<A>) -> M
	 * \endcode
	 *
	 * Makes it generally a lot cleaner to call `foldMap`, as well as
	 * easier to pass to higher order functions.
	 *
	 * \par Examples
	 *
	 * \code
	 *   auto v = vector<int>{3,3,4};
	 *
	 *   auto r = ftl::foldMap(ftl::sum<int>, v);
	 *   // r == ftl::sum(10)
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	foldMap;
#endif

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _foldr : public _dtl::curried_ternf<_foldr> {
		template<
				typename F,
				typename Fn,
				typename U,
				typename T = Value_type<F>,
				typename = Requires<Foldable<F>{}>,
				typename = Requires<
					std::is_same<plain_type<U>, result_of<Fn(T,U)>>::value
				>
		>
		plain_type<U> operator() (Fn&& fn, U&& z, const F& f) const {
			return foldable<F>::foldr(std::forward<Fn>(fn), std::forward<U>(z), f);
		}

		using curried_ternf<_foldr>::operator();
	} foldr {};
#else
	struct ImplementationDefined {
	}
	/**
	 * Convenience function object for `foldable::foldr`.
	 *
	 * Behaves as if it were a curried function of type
	 * \code
	 *   ((T,U) -> U, U, F<T>) -> U
	 * \endcode
	 *
	 * Provides easy and concise calling, unlike explicitly looking up the
	 * instance implementation as `foldable<instance>::foldr`.
	 *
	 * A right fold is, as the name implies, right associative. Which is to say,
	 * the operation `foldr(f, z, {1,2,3})` is equivalent to:
	 * \code
	 *   f(1, f(2, f(3, z)));
	 * \endcode
	 *
	 * \par Examples
	 *
	 * Summing the elements of a list:
	 * \code
	 *   std::list<int> l = {1, 2, 3};
	 *   int sum = ftl::foldr(std::plus<int>(), 0, l);
	 *   // sum == 6
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	foldr;
#endif

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _foldl : public _dtl::curried_ternf<_foldl> {
		template<
				typename F,
				typename Fn,
				typename U,
				typename T = Value_type<F>,
				typename = Requires<Foldable<F>{}>,
				typename = Requires<
					std::is_same<plain_type<U>, result_of<Fn(U,T)>>::value
				>
		>
		plain_type<U> operator() (Fn&& fn, U&& z, const F& f) const {
			return foldable<F>::foldl(std::forward<Fn>(fn), std::forward<U>(z), f);
		}

		using curried_ternf<_foldl>::operator();
	} foldl{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Convenience function object representing `foldable::foldl`.
	 *
	 * Behaves as if it were a curried function of type
	 * \code
	 *   ((U,T) -> U, U, F<T>) -> U
	 * \endcode
	 *
	 * A left fold is of course left-associative, meaning that the operation
	 * `foldl(f, z, {1,2,3})` is equivalent to:
	 * \code
	 *   f(f(f(z, 1), 2), 3);
	 * \endcode
	 *
	 * \par Examples
	 *
	 * Summing the elements of a list:
	 * \code
	 *   std::list<int> l = {1, 2, 3};
	 *   int sum = ftl::foldl(std::plus<int>(), 0, l);
	 *   // sum == 6
	 * \endcode
	 *
	 * Reversing a list:
	 * \code
	 *   std::list<int> l = {1, 2, 3};
	 *   auto rCons = [](list<int> xs, int x){ xs.push_front(x); return xs; };
	 *
	 *   auto reverseL = ftl::foldl(rCons, list<int>{}, l);
	 *   // reverseL == {3, 2, 1}
	 * \endcode
	 *
	 * \ingroup foldable
	 */
	foldl;
#endif
}

#endif

