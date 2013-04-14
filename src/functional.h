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
#ifndef FTL_FUNCTIONAL_H
#define FTL_FUNCTIONAL_H

#include <functional>
#include "monoid.h"

namespace ftl {

	template<typename A, typename B>
	std::function<B(A)> compose(std::function<B(A)> f) {
		return f;
	}

	template<typename A, typename B>
	std::function<B(A)> compose(B(*f)(A)) {
		return [=](A a){ return f(a); };
	}

	template<typename A, typename B, typename C>
	auto compose(std::function<C(B)> f1, std::function<B(A)> f2)
	-> std::function<C(A)> {
		return [=] (A a) { return f1(f2(a)); };
	}

	template<typename A, typename B, typename C>
	std::function<C(A)> compose(C(*f1)(B), B(*f2)(A)) {
		return [=] (A a) { return f1(f2(a)); };
	}

	template<typename A, typename B, typename C, typename...Fns>
	auto compose(std::function<C(B)> f1, std::function<B(A)> f2, Fns...fns)
	-> std::function<C (typename decltype(compose(fns...))::argument_type)> {

		using top_arg_type = typename decltype(compose(fns...))::argument_type;

		auto fr = compose(fns...);
		return [=] (top_arg_type arg) { return f1(f2(fr(arg))); };
	}

	template<typename A, typename B, typename C, typename...Fns>
	auto compose(C(*f1)(B), B(*f2)(A), Fns...fns)
	-> std::function<C (typename decltype(compose(fns...))::argument_type)> {

		using top_arg_type = typename decltype(compose(fns...))::argument_type;

		auto fr = compose(fns...);
		return [=] (top_arg_type arg) { return f1(f2(fr(arg))); };
	}

	/**
	 * Monoid instance for functions returning monoids.
	 *
	 * The reason this works might not be immediately obvious, but basically,
	 * any function (regardless of arity) that returns a value that is an
	 * instance of monoid, is in fact also a monoid. It works as follows:
	 * \code
	 *   id() <=> A function, returning monoid<result_type>::id(),
	 *            regardless of parameters.
	 *   append(a,b) <=> A function that forwards its arguments to both a and b,
	 *                   and then calls monoid<result_type>::append on the two
	 *                   results.
	 * \endcode
	 */
	template<typename M, typename...Ps>
	struct monoid<std::function<M(Ps...)>> {
		static std::function<M(Ps...)> id() {
			return [](Ps...ps) { return monoid<M>::id(); };
		}

		static std::function<M(Ps...)> append(
				const std::function<M(Ps...)>& f1,
				const std::function<M(Ps...)>& f2) {
			return [=] (Ps...ps) {
				return monoid<M>::append(f1(ps...), f2(ps...));
			};
		}
	};

	template<typename M, typename...Ps>
	std::function<M(Ps...)> operator^ (
			const std::function<M(Ps...)>& f1,
			const std::function<M(Ps...)>& f2) {
		return monoid<std::function<M(Ps...)>>::append(f1, f2);
	}

}

#endif

