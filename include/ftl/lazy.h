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
#include "either.h"
#include "tuple.h"

namespace ftl {
	/**
	 * \defgroup lazy The lazy data type and its concept instances.
	 *
	 * \code
	 *   #include <ftl/lazy.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - \ref either
	 * - \ref tuple
	 */

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
	 * \note Lazy values are immutable. Bypassing this with some creative
	 *       casting may result in undefined behaviour.
	 *
	 * \par Concepts
	 * - \ref copycons
	 * - \ref movecons
	 * - \ref assignable (note that assigning to a `lazy` does not change or
	 *        force the underlying computation, it merely changes _what_
	 *        computation that particular `lazy` encapsulates.
	 * - \ref functor
	 * - \ref applicative
	 * - \ref monad
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
		explicit lazy(const function<T>& f)
		: val(new either<function<T>,T>(make_left<T>(f)))
		{}

		/**
		 * Get a reference to the value.
		 *
		 * This method forces evaluation.
		 */
		const T& operator*() const {
			if(*val)
				return **val;

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
				return &(*val);

			force();
			return &(*val);
		}

		lazy& operator= (const lazy&) = default;
		lazy& operator= (lazy&&) = default;

	private:
		void force() const {
			if(*val)
				return;

			*val = make_right<function<T>>((val->left())());
		}

		mutable std::shared_ptr<either<function<T>,T>> val;
	};

	/**
	 * Create a lazy computation from an arbitrary function.
	 *
	 * Currently, all parameters are _copied_ when `defer` is called. If you
	 * want to call by reference on some parameter, you should use `std::cref`
	 * (the use of `std::ref` is _not_ supported, because it would allow
	 * mutation of the parameter and all lazy computations are assumed to be
	 * _pure_, in the sense that they should have no side effects, nor contain
	 * any state).
	 */
	template<
			typename F,
			typename...Args,
			typename T = typename decayed_result<F(Args...)>::type
	>
	lazy<T> defer(F f, Args&&...args) {
		auto t = std::make_tuple(std::forward<Args>(args)...);
		return lazy<T>{[f,t]() {
				return apply(f, t);
		}};
	}

	template<typename T>
	struct monad<lazy<T>> {
		static lazy<T> pure(T t) {
			return lazy<T>{[t](){ return t; }};
		}

		template<
				typename F,
				typename U = typename decayed_result<F(T)>::type
		>
		static lazy<U> map(F f, lazy<T> l) {
			return lazy<U>{[f,l]() { return f(*l); }};
		}

		template<
				typename F,
				typename U =
					concept_parameter<typename decayed_result<F(T)>::type>
		>
		static lazy<U> bind(lazy<T> l, F f) {
			return lazy<U>{[f,l]() {
				return *(f(*l));
			}};
		}
	};

}

#endif

