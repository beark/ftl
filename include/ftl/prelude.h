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
#ifndef FTL_PRELUDE_H
#define FTL_PRELUDE_H

#include "function.h"

namespace ftl {
	/**
	 * \defgroup prelude Prelude
	 *
	 * A number of utility functions useful with higher order functions.
	 *
	 * \code
	 *   #include <ftl/prelude.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - <ftl/function.h>
	 */

	/**
	 * Identity function object.
	 *
	 * Returns whatever is given as parameter. This can be useful in combination
	 * with certain higher order functions. For example, `foldable::fold` can be
	 * trivially implemented as `foldable::foldMap(identity(), ...)`.
	 *
	 * \ingroup prelude
	 */
	struct identity {
		template<typename T>
		constexpr auto operator()(T&& t) const noexcept
		-> decltype(std::forward<T>(t)) {
			return std::forward<T>(t);
		}
	};

	/**
	 * Compile time instance of identity.
	 *
	 * Makes passing the identity function to higher order functions even more
	 * convenient.
	 *
	 * Example usage:
	 * \code
	 *   // Does nothing; v will be {1,2,3}
	 *   auto v = ftl::fmap(ftl::id, std::vector<int>{1,2,3});
	 * \endcode
	 *
	 * \ingroup prelude
	 */
	constexpr identity id{};

	/**
	 * Used to distinguish in-place constructors from others.
	 *
	 * Used by e.g. `ftl::maybe` and `ftl::either` to make perfect forwarding
	 * tot he contained type(s) possible.
	 *
	 * \ingroup prelude
	 */
	struct inplace_tag {};

	/**
	 * Curries an n-ary function pointer.
	 *
	 * Currying is the process of turning a function of e.g. `(a,b) -> c` into
	 * `(a) -> ((b) -> c)`. In other words, instead of taking two arguments and
	 * returning the answer, the curried function takes one argument and
	 * returns a function that takes another one and _then_ returns the
	 * answer.
	 *
	 * \note This operation is actually exactly equivalent of wrapping the
	 *       function in an ftl::function object, as those support curried
	 *       calling by default.
	 *
	 * \ingroup prelude
	 */
	template<typename R, typename P1, typename P2, typename...Ps>
	function<R,P1,P2,Ps...> curry(R (*f) (P1, P2, Ps...)) {
		return function<R,P1,P2,Ps...>(f);
	}

	/**
	 * \overload
	 *
	 * \ingroup prelude
	 */
	template<typename R, typename P1, typename P2, typename...Ps>
	function<R,P1,P2,Ps...> curry(const std::function<R(P1,P2,Ps...)>& f) {
		return function<R,P1,P2,Ps...>(f);
	}

	/**
	 * Uncurries a binary function.
	 *
	 * \ingroup prelude
	 */
	template<typename R, typename T1, typename T2>
	function<R,T1,T2> uncurry(function<function<R,T2>,T1> f) {
		return [f] (T1 t1, T2 t2) {
			return f(std::forward<T1>(t1))(std::forward<T2>(t2));
		};
	}

	/**
	 * Function composition first base case.
	 *
	 * Composes an arbitrary function object with a function pointer.
	 *
	 * \ingroup prelude
	 */
	template<
		typename F,
		typename A,
		typename B = typename std::result_of<F(A)>::type,
		typename...Ps>
	function<B,Ps...> compose(F f, A (*fn)(Ps...)) {
		return [f,fn](Ps...ps) {
			return f(fn(std::forward<Ps>(ps)...));
		};
	}

	/**
	 * Function composition second base case.
	 *
	 * Composes an arbitrary function object with an ftl::function.
	 *
	 * \ingroup prelude
	 */
	template<
		typename F,
		typename A,
		typename B = typename std::result_of<F(A)>::type,
		typename...Ps>
	function<B,Ps...> compose(F f, function<A,Ps...> fn) {
		return [f,fn](Ps...ps) {
			return f(fn(std::forward<Ps>(ps)...));
		};
	}

	/**
	 * Generalised, n-ary function composition.
	 *
	 * Composes an arbitrary number of functions, where each function's return
	 * value is piped to the next. The right-most function in the sequence is
	 * the first to be evaluated and its result is passed to the one step to the
	 * left. Return values must match parameter type of the next one in the
	 * chain.
	 *
	 * \ingroup prelude
	 */
	template<typename F, typename...Fs>
	auto compose(F f, Fs...fs)
	-> decltype(compose(f,compose(std::forward<Fs>(fs)...))) {
		return compose(f, compose(std::forward<Fs>(fs)...));
	}

	/**
	 * Flip the parameter order of a binary function.
	 *
	 * \ingroup prelude
	 */
	template<typename A, typename B, typename R>
	function<R,B,A> flip(function<R,A,B> f) {
		return [f](B b, A a) {
			return f(std::forward<A>(a), std::forward<B>(b));
		};
	}

	/**
	 * \overload
	 *
	 * \ingroup prelude
	 */
	template<typename A, typename B, typename R>
	function<R,B,A> flip(R (&f) (A,B)) {
		return [&f](B b, A a) {
			return f(std::forward<A>(a), std::forward<B>(b));
		};
	}

	/**
	 * Flip parameter order of a curried binary function.
	 *
	 * \ingroup prelude
	 */
	template<typename R, typename A, typename B>
	function<function<R,A>,B> flip(function<function<R,B>,A> f) {
		return [f](B b) {
			return [f,b](A a) {
				return f(std::forward<A>(a))(b);
			};
		};
	}

	namespace _dtl {
		// This struct is used to generate curried calling convention for
		// arbitrary binary functions
		template<typename F>
		struct curried_binf {
		private:
			template<typename P>
			struct curried {
				explicit curried(const P& p) : p(p) {}
				explicit curried(P&& p) : p(std::move(p)) {}

				P p;

				template<typename T>
				auto operator() (T&& t) const
				-> decltype(std::declval<F>()(p, std::forward<T>(t))) {
					return F()(p, std::forward<T>(t));
				}
			};

		public:
			template<typename P>
			curried<plain_type<P>> operator() (P&& p) const {
				return curried<plain_type<P>>(std::forward<P>(p));
			}
		};

		// This struct is used to generate curried calling convention for
		// arbitrary ternary functions
		template<typename F>
		struct curried_ternf {
		private:
			template<typename P1>
			struct curried2 {
			private:
				template<typename P2>
				struct curried {
					constexpr curried(const P1& p1, const P2& p2)
					noexcept(
						std::is_nothrow_copy_constructible<P1>::value
						&& std::is_nothrow_copy_constructible<P2>::value
					)
					: p1(p1), p2(p2) {}

					constexpr curried(const P1& p1, P2&& p2)
					noexcept(
						std::is_nothrow_copy_constructible<P1>::value
						&& std::is_nothrow_move_constructible<P2>::value
					)
					: p1(p1), p2(std::move(p2)) {}

					constexpr curried(P1&& p1, const P2& p2)
					noexcept(
						std::is_nothrow_move_constructible<P1>::value
						&& std::is_nothrow_copy_constructible<P2>::value
					)
					: p1(std::move(p1)), p2(p2) {}

					constexpr curried(P1&& p1, P2&& p2)
					noexcept(
						std::is_nothrow_move_constructible<P1>::value
						&& std::is_nothrow_move_constructible<P2>::value
					)
					: p1(std::move(p1)), p2(std::move(p2)) {}

					P1 p1;
					P2 p2;

					template<typename P3>
					auto operator() (P3&& p3) const
					-> decltype(std::declval<F>()(p1, p2, std::forward<P3>(p3))) {
						return F()(p1, p2, std::forward<P3>(p3));
					}
				};

			public:
				explicit constexpr curried2(const P1& p)
				noexcept(std::is_nothrow_copy_constructible<P1>::value)
				: p(p) {}

				explicit constexpr curried2(P1&& p)
				noexcept(std::is_nothrow_move_assignable<P1>::value)
				: p(std::move(p)) {}

				P1 p;

				template<typename P2, typename P3>
				auto operator() (P2&& p2, P3&& p3)
				-> decltype(std::declval<F>()(p, std::forward<P2>(p2), std::forward<P3>(p3))) {
					return F()(p, std::forward<P2>(p2), std::forward<P3>(p3));
				}

				template<typename P2>
				curried<plain_type<P2>> operator() (P2&& p2) const /* & */  {
					return curried<plain_type<P2>>(p, std::forward<P2>(p2));
				}

				/** TODO: Enable r-value overload when gcc-4.8 becomes more standard
				template<typename P2>
				curried<plain_type<P2>> operator() (P2&& p2) && {
					return curried<P1,plain_type<P2>>(std::move(p), std::forward<P2>(p2));
				}
				*/
			};

		public:
			template<typename P>
			curried2<plain_type<P>> operator() (P&& p) const {
				return curried2<plain_type<P>>(std::forward<P>(p));
			}
		};
	}
}

#endif

