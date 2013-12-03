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
#ifndef FTL_LAZY_H
#define FTL_LAZY_H

#include <memory>
#include "prelude.h"
#include "concepts/monoid.h"
#include "either.h"

namespace ftl {
	/**
	 * \defgroup lazy Lazy
	 *
	 * The lazy data type and its concept instances.
	 *
	 * \code
	 *   #include <ftl/lazy.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref prelude
	 * - \ref monoid
	 * - \ref either
	 */

	/**
	 * Enumeration of the states a lazy computation can be in.
	 *
	 * Mainly used in combination with ftl::lazy::valueStatus().
	 *
	 * \ingroup lazy
	 */
	enum class value_status {
		/// The computation still has not been performed
		deferred,
		/// The value is computed and ready
		ready
	};

	/**
	 * The lazy data type.
	 *
	 * Wraps a value of type `T`, causing its evaluation to be deferred until
	 * such a time as it is required.
	 *
	 * To reduce the number of times a particular computation is made, copies
	 * (as created by e.g. the copy constructor) of a particular `lazy` all
	 * refer to a shared object representing either the computed value, or the
	 * computation that will yield the value. Hence, the computation will only
	 * be made _once_ for every set of copies ultimately derived from the same
	 * source.
	 *
	 * If no instance of a particular computation ever forces it, then it simply
	 * won't be evaluated at all.
	 *
	 * As a convenience, there is a specialisation of `lazy` for `bool` that
	 * allows contextual conversions of `lazy<bool>` to `bool`, allowing
	 * expressions such as `if(lazyBool) doSomething();`. This will force
	 * evaluation.
	 *
	 * \note Lazy values are immutable. Bypassing this with some creative
	 *       casting may result in undefined behaviour.
	 *
	 * \par Concepts
	 * - \ref copycons
	 * - \ref movecons
	 * - \ref assignable (note that assigning to a `lazy` does not change or
	 *        force the underlying computation, it merely changes _what_
	 *        computation that particular `lazy` encapsulates.
	 * - \ref deref to `T` (_forces_ evaluation).
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
	 * - \ref eq, if `T` is EqualityComparable.
	 * - \ref orderable, if `T` is Orderable.
	 * - \ref monoid, if `T` is a Monoid.
	 *
	 * \ingroup lazy
	 */
	template<typename T>
	class lazy {
	public:
		lazy() = delete;
		lazy(const lazy&) = default;
		lazy(lazy&&) = default;
		~lazy() = default;

		/**
		 * Construct from a no-argument function object.
		 *
		 * In essence, whenever the value is first _forced_, the function
		 * object will be invoked to compute it. Any subsequent calls to methods
		 * that would normally force evaluation will simply use the now computed
		 * value.
		 */
		explicit lazy(const function<T()>& f)
		: val(new either<function<T()>,T>(make_left<T>(f)))
		{}

		/**
		 * Get a reference to the value.
		 *
		 * This method forces evaluation.
		 */
		const T& operator*() const {
			force();
			return **val;
		}

		/**
		 * Access members of the lazy value.
		 *
		 * This method forces evaluation.
		 */
		const T* operator->() const {
			if(*val)
				return &(**val);

			force();
			return &(**val);
		}

		lazy& operator= (const lazy&) = default;
		lazy& operator= (lazy&&) = default;

		/**
		 * Check the state of the deferred computation.
		 *
		 * \return value_status::deferred if computation has not yet been run,
		 *         and value_status::ready if it has.
		 */
		value_status status() const {
			if(*val)
				return value_status::ready;

			return value_status::deferred;
		}

	private:
		void force() const {
			if(*val)
				return;

			*val = make_right<function<T()>>(val->left()());
		}

		mutable std::shared_ptr<either<function<T()>,T>> val;
	};

	// Bool specialisation to allow contextual conversion
	template<>
	class lazy<bool> {
	public:
		lazy() = delete;
		lazy(const lazy&) = default;
		lazy(lazy&&) = default;
		~lazy() = default;

		explicit lazy(const function<bool()>& f)
		: val(new either<function<bool()>,bool>(make_left<bool>(f)))
		{}

		const bool& operator*() const {
			force();
			return **val;
		}

		const bool* operator->() const {
			if(*val)
				return &(**val);

			force();
			return &(**val);
		}

		lazy& operator= (const lazy&) = default;
		lazy& operator= (lazy&&) = default;

		explicit operator bool() {
			force();
			return **val;
		}

		value_status status() const {
			if(*val)
				return value_status::ready;

			return value_status::deferred;
		}

	private:
		void force() const {
			if(*val)
				return;

			*val = make_right<function<bool()>>(val->left()());
		}

		mutable std::shared_ptr<either<function<bool()>,bool>> val;
	};

	/**
	 * Create a lazy computation from an arbitrary function.
	 *
	 * Currently, all parameters are _copied_ when `defer` is called. If you
	 * want to call by reference on some parameter, you should use `std::cref`
	 * (the use of `std::ref` is not encouraged, because it allows mutation
	 * of the parameter and all lazy computations are assumed to be pure, in
	 * the sense that they should have no side effects, nor contain any state).
	 *
	 * \note `f` is assumed to be of unary or greater arity. If a deferred zero
	 *       argument computation is desired, use lazy's constructor directly.
	 *
	 * \ingroup lazy
	 */
	template<
			typename F,
			typename...Args,
			typename T = result_of<F(Args...)>
	>
	lazy<T> defer(F f, Args&&...args) {
		// TODO: C++14: _move_ tuple of args into lambda
		// TODO: Make this work with zero-argument fs
		auto t = std::make_tuple(std::forward<Args>(args)...);
		return lazy<T>{[f,t]() {
				return tuple_apply(f, t);
		}};
	}

	/**
	 * Equality comparison.
	 *
	 * Does not evaluate `l1` or `l2` until the returned computation is
	 * itself evaluated.
	 *
	 * \tparam T must have an `operator==`.
	 *
	 * \ingroup lazy
	 */
	template<typename T>
	lazy<bool> operator==(lazy<T> l1, lazy<T> l2) {
		return lazy<bool>{[l1,l2]() {
			return *l1 == *l2;
		}};
	}

	/**
	 * Not equal comparison.
	 *
	 * Does not evaluate `l1` or `l2` until the returned computation is
	 * itself evaluated.
	 *
	 * \tparam T must have an `operator!=`.
	 *
	 * \ingroup lazy
	 */
	template<typename T>
	lazy<bool> operator!=(lazy<T> l1, lazy<T> l2) {
		return lazy<bool>{[l1,l2]() {
			return *l1 != *l2;
		}};
	}

	/**
	 * Less than comparison
	 *
	 * Does not evaluate `lhs` or `rhs` until the returned computation is
	 * itself evaluated.
	 *
	 * \tparam T must have an `operator<`.
	 *
	 * \ingroup lazy
	 */
	template<typename T>
	lazy<bool> operator< (lazy<T> lhs, lazy<T> rhs) {
		return lazy<bool>{[lhs,rhs]() {
			return *lhs < *rhs;
		}};
	}

	/**
	 * Greater than comparison
	 *
	 * Does not evaluate `lhs` or `rhs` until the returned computation is
	 * itself evaluated.
	 *
	 * \tparam T must have an `operator>`.
	 *
	 * \ingroup lazy
	 */
	template<typename T>
	lazy<bool> operator> (lazy<T> lhs, lazy<T> rhs) {
		return lazy<bool>{[lhs,rhs]() {
			return *lhs > *rhs;
		}};
	}

	/**
	 * Monad instance for lazy values.
	 *
	 * Allows users to build "thunks" of computations, all left uncomputed until
	 * forced.
	 *
	 * \ingroup lazy
	 */
	template<typename T>
	struct monad<lazy<T>>
   	: deriving_join<in_terms_of_bind<lazy<T>>>
	, deriving_apply<in_terms_of_bind<lazy<T>>> {
		/**
		 * Create a computation that computes `t`
		 *
		 * Sounds a bit silly&mdash;we already know `t` after all&mdash;but
		 * there are situations when it can be useful (e.g. algorithms
		 * generalised over any monad).
		 */
		static lazy<T> pure(T t) {
			return lazy<T>{[t](){ return t; }};
		}
		// TODO: C++14: Add a pure that captures Ts by move

		/**
		 * Map a function to the deferred value.
		 *
		 * As expected, this does not actually compute the deferred value
		 * in `l`. Instead, we simply defer both the invocation of `f` and
		 * the computation of `l` until someone forces the _returned_ lazy
		 * copmutation (though technically, `l` could be forced by another,
		 * independant computation ahead of that).
		 */
		template<typename F, typename U = result_of<F(T)>>
		static lazy<U> map(F f, lazy<T> l) {
			return lazy<U>{function<U()>{[f,l]() { return f(*l); }}};
		}

		/**
		 * Sequences two lazy computations.
		 *
		 * As with functor<lazy<T>>::map, the whole bind is deferred until
		 * the returned computation is forced.
		 *
		 * Note that, again as with `map`, the lazy computation `l` could be
		 * forced ahead of time by unrelated code elsewhere. This is due to the
		 * shared nature of lazy values (copies always refer to the same
		 * internal object as the original).
		 */
		template<
				typename F,
				typename U = Value_type<result_of<F(T)>>
		>
		static lazy<U> bind(lazy<T> l, F f) {
			return lazy<U>{[f,l]() {
				return *(f(*l));
			}};
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monoid instance of lazy computations.
	 *
	 * \tparam T must be a monoid to begin with.
	 *
	 * This instance is exactly equivalent of `T`'s monoid instance, except
	 * that the computations of `monoid<T>::id()` and `monoid<T>::append()` are
	 * of course deferred until forced. 
	 *
	 * \ingroup lazy
	 */
	template<typename T>
	struct monoid<lazy<T>> {
		/**
		 * Lazily "computes" monoid<T>::id()
		 */
		static lazy<T> id() {
			return lazy<T>{monoid<T>::id};
		}

		/**
		 * Lazily computes monoid<T>::append(*l1, *l2).
		 *
		 * Note that neither `l1` nor `l2` are forced by invoking this function.
		 * They are, of course, forced when the result of this computation is.
		 */
		static lazy<T> append(lazy<T> l1, lazy<T> l2) {
			return lazy<T>([l1,l2](){ return monoid<T>::append(*l1, *l2); });
		}

		static constexpr bool instance = monoid<T>::instance;
	};
}

#endif

