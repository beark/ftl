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
#ifndef FTL_PRELUDE_H
#define FTL_PRELUDE_H

#include <tuple>
#include "static.h"
#include "function.h"
#include "concepts/basic.h"
#include "concepts/orderable.h"

#include "implementation/currying.h"

namespace ftl {
	/**
	 * \defgroup prelude Prelude
	 *
	 * A collection of utilities and functions, typically useful in combination
	 * with the other, more specialised FTL modules.
	 *
	 * \code
	 *   #include <ftl/prelude.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - `<tuple>`
	 * - \ref static
	 * - \ref function
	 * - \ref concepts_basic
	 * - \ref orderable
	 */

	/**
	 * A short-hand alias of `std::enable_if`.
	 *
	 * Allows multiple predicates to be required in one go.
	 *
	 * \note Consider this a temporary solution until concepts lite or something
	 * similar becomes readily available. As soon as that comes to pass, this
	 * construct will be considered deprecated.
	 *
	 * \par Examples
	 * \code
	 *   template<typename M, typename = Requires<Monad<M>::value, Eq<M>::value>>
	 *   void foo(const M& m) {
	 *       // Safely perform monadic operations and equality comparisons on m
	 *   }
	 * \endcode
	 *
	 * \ingroup prelude
	 */
	template<bool...Preds>
	using Requires = std::enable_if_t<static_fold<bool,Preds...>::with(std::logical_and<bool>())>;

	struct identity_t
	{
		template<typename T> constexpr auto operator()(T&& t) const noexcept
		-> decltype(std::forward<T>(t))
		{
			return std::forward<T>(t);
		}
	};

	/**
	 * Identity function object.
	 *
	 * Has semantics equivalent to a function of type
	 * \code
	 *   (T) -> T
	 * \endcode
	 *
	 * In some ways, just an alias for `std::forward`. However, `id` is not
	 * only terser, it is also much easier to pass to higher-order functions.
	 *
	 * \par Examples
	 *
	 * Trivial usage:
	 * \code
	 *   // Does nothing; v will be {1,2,3}
	 *   auto v = ftl::fmap(ftl::id, std::vector<int>{1,2,3});
	 * \endcode
	 *
	 * \ingroup prelude
	 */
	constexpr identity_t id {};

	/**
	 * Identity type transformer.
	 *
	 * Just as the identity function can be useful with higher-order functions,
	 * the identity type transformer can come in handy with parametric type
	 * aliases and transformers.
	 *
	 * In particular, the fact that `Identity` often implements many of the
	 * concepts of FTL can be useful. For example, it is perhaps elegant
	 * to define a particular monad in terms of its corresponding monad
	 * transformer and `Identity`, as in:
	 *
	 * \code
	 *   template<typename T>
	 *   using some_monad = some_monadT<Identity<T>>;
	 * \endcode
	 *
	 * Since `Identity` is a very thin wrapper, the operations of which are
	 * usually one line constexpr functions, little if any performance should
	 * be lost by doing it that way.
	 *
	 * \par Concepts
	 *
	 * - \ref defaultcons, if `T` is
	 * - \ref copycons, if `T` is
	 * - \ref movecons, if `T` is
	 * - \ref deref to `T`
	 * - \ref eq, if `T` is
	 * - \ref orderable, if `T` is
	 * - \ref functorpg (instance defined in \ref monad)
	 * - \ref applicativepg (instance defined in \ref monad)
	 * - \ref monadpg (instance defined in \ref monad)
	 *
	 * In addition, `Identity<T>` is implicitly convertible to `T`, `T&`, and
	 * `T const&`. However, it is only _explicitly_ constructible from `T&&` and
	 * `T const&`.
	 *
	 * \ingroup prelude
	 */
	template<class T>
	struct Identity {
		using value_type = T;

		Identity() = default;
		Identity(const Identity&) = default;
		Identity(Identity&&) = default;

		explicit constexpr Identity(const T& t)
		noexcept(std::is_nothrow_copy_constructible<T>::value)
		: val(t) {}

		explicit constexpr Identity(T&& t)
		noexcept(std::is_nothrow_move_constructible<T>::value)
		: val(std::move(t)) {}

		~Identity() = default;

		constexpr operator T() const noexcept {
			return val;
		}

		constexpr operator const T& () const noexcept {
			return val;
		}

		operator T& () noexcept {
			return val;
		}

		T& operator*() noexcept {
			return val;
		}

		constexpr const T& operator*() const noexcept {
			return val;
		}

		T* operator->() noexcept {
			return std::addressof(val);
		}

		constexpr const T* operator->() const noexcept {
			return std::addressof(val);
		}

		Identity& operator= (const Identity&) = default;
		Identity& operator= (Identity&&) = default;

		T val;
	};

	// ## Operators for Identity type transformer

	template<typename T, typename = Requires<Eq<T>::value>>
	constexpr auto operator== (const Identity<T>& a, const Identity<T>& b)
	noexcept
	{
		return a.val == b.val;
	}

	template<typename T, typename = Requires<Eq<T>::value>>
	constexpr auto operator!= (const Identity<T>& a, const Identity<T>& b)
	noexcept
	{
		return a.val != b.val;
	}

	template<typename T, typename = Requires<Orderable<T>::value>>
	constexpr auto operator< (const Identity<T>& a, const Identity<T>& b)
	noexcept -> decltype(std::declval<T>() < std::declval<T>()) {
		return a.val < b.val;
	}

	template<typename T, typename = Requires<Orderable<T>::value>>
	constexpr auto operator<= (const Identity<T>& a, const Identity<T>& b)
	noexcept -> decltype(std::declval<T>() <= std::declval<T>()) {
		return a.val <= b.val;
	}

	template<typename T, typename = Requires<Orderable<T>{}>>
	constexpr auto operator>= (const Identity<T>& a, const Identity<T>& b)
	noexcept -> decltype(std::declval<T>() >= std::declval<T>()) {
		return a.val >= b.val;
	}

	template<typename T, typename = Requires<Orderable<T>{}>>
	constexpr auto operator> (const Identity<T>& a, const Identity<T>& b)
	noexcept -> decltype(std::declval<T>() > std::declval<T>()) {
		return a.val > b.val;
	}

	/**
	 * Used to distinguish in-place constructors from others.
	 *
	 * Used by e.g. `ftl::maybe` and `ftl::either` to make perfect forwarding
	 * tot he contained type(s) possible.
	 *
	 * \ingroup prelude
	 */
	struct inplace_tag {};

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _tuple_apply : ::ftl::_dtl::curried_binf<_tuple_apply> {

		template<typename F, typename Tuple>
		constexpr auto operator() (F&& f, Tuple&& tuple) const
		-> decltype(
			::ftl::_dtl::tup_apply(std::forward<F>(f), std::forward<Tuple>(tuple))
		)
		{
			return ::ftl::_dtl::tup_apply(
				std::forward<F>(f), std::forward<Tuple>(tuple)
			);
		}

		using _dtl::curried_binf<_tuple_apply>::operator();

	} tuple_apply{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Invoke a function using a tuple's fields as parameters.
	 *
	 * Behaves like a curried function of type:
	 * \code
	 *   ((Ts...) -> R, std::tuple<Ts...>) -> R
	 * \endcode
	 *
	 * \par Examples
	 *
	 * Simple invocation:
	 * \code
	 *   void foo(int, float);
	 *
	 *   // Invokes foo with 1 and 2.f as arguments
	 *   ftl::tuple_apply(foo, std::make_tuple(1, 2.f));
	 * \endcode
	 *
	 * \ingroup prelude
	 */
	tuple_apply;
#endif

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
	function<R(P1,P2,Ps...)> curry(R (*f) (P1, P2, Ps...)) {
		return function<R(P1,P2,Ps...)>(f);
	}

	/**
	 * \overload
	 *
	 * \ingroup prelude
	 */
	template<typename R, typename P1, typename P2, typename...Ps>
	function<R(P1,P2,Ps...)> curry(const std::function<R(P1,P2,Ps...)>& f) {
		return function<R(P1,P2,Ps...)>(f);
	}

	/**
	 * Curries arbitrary function objects.
	 *
	 * Example:
	 * \code
	 *   auto f = [](int x, int y, int z){ return x+y-z; };
	 *
	 *   auto g = ftl::curry(f);
	 *
	 *   // g(1, 2, 3) == g(1, 2)(3) == g(1)(2, 3) == g(1)(2)(3)
	 * \endcode
	 *
	 * \note Because this version of `curry` works on arbitrary function objects
	 *       with unknown and possibly multiple, overloaded `operator()`s,
	 *       there is no way to force the result of `curry` to accept only
	 *       matching types. If you give a curried function object parameters
	 *       that does not match any of its `operator()`s, it will simply
	 *       never be invoked, it will just continue to accumulate parameters.
	 *
	 * \ingroup prelude
	 */
	template<
			typename F,
			typename = Requires<!is_monomorphic<::std::decay_t<F>>::value>
	>
#ifndef DOCUMENTATION_GENERATOR
	_dtl::curried_fn<::std::decay_t<F>>
#else
	ImplementationDefined
#endif
	curry(F&& f)
	{
		return _dtl::curried_fn<::std::decay_t<F>>(std::forward<F>(f));
	}

	/**
	 * Curries an N-ary function objects.
	 *
	 * Example:
	 * \code
	 *   auto f = [](int x, int y, int z){ return x+y-z; };
	 *
	 *   auto g = ftl::curry<3>(f);
	 *
	 *   // g(1, 2, 3) == g(1, 2)(3) == g(1)(2, 3) == g(1)(2)(3)
	 * \endcode
	 *
	 * \note Because this version of `curry` works on arbitrary function objects
	 *       with unknown and possibly multiple, overloaded `operator()`s,
	 *       there is no way to force the result of `curry` to accept only
	 *       matching types. 
	 *
	 * \ingroup prelude
	 */
	template<size_t N, typename F>
#ifndef DOCUMENTATION_GENERATOR
	_dtl::curried_fn_n<N,::std::decay_t<F>>
#else
	ImplementationDefined
#endif
	curry( F&& f ) {
		return std::forward<F>(f);
	}

	/**
	 * Uncurries a binary function.
	 *
	 * \ingroup prelude
	 */
	template<typename R, typename T1, typename T2>
	function<R(T1,T2)> uncurry(function<function<R(T2)>(T1)> f) {
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
	function<B(Ps...)> compose(F f, A (*fn)(Ps...)) {
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
	function<B(Ps...)> compose(F f, function<A(Ps...)> fn) {
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
	auto compose(F&& f, Fs&&...fs)
	-> decltype(compose(std::forward<F>(f), compose(std::forward<Fs>(fs)...))) {
		return compose(std::forward<F>(f), compose(std::forward<Fs>(fs)...));
	}

	/**
	 * Flip the parameter order of a binary function.
	 *
	 * \ingroup prelude
	 */
	template<typename A, typename B, typename R>
	function<R(B,A)> flip(function<R(A,B)> f) {
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
	function<R(B,A)> flip(R (&f) (A,B)) {
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
	function<function<R(A)>(B)> flip(function<function<R(B)>(A)> f) {
		return [f](B b) {
			return [f,b](A a) {
				return f(std::forward<A>(a))(b);
			};
		};
	}

	/**
	 * Compile time check for \ref fwditerable instances.
	 *
	 * Example:
	 * \code
	 *   template<
	 *       typename Container,
	 *       typename = Requires<
	 *           ForwardIterable<Container>()
	 *       >
	 *   >
	 *   void foo(const Container& c) {
	 *       // Safe to iterate with e.g. for(auto& e : c)
	 *   }
	 * \endcode
	 *
	 * \ingroup prelude
	 */
	template<typename T>
	constexpr bool ForwardIterable() {
		return has_begin<T>::value &&
			has_end<T>::value &&
			has_pre_inc<decltype(begin(std::declval<T>()))>::value &&
			has_post_inc<decltype(begin(std::declval<T>()))>::value &&
			std::is_same<
				Value_type<T>,
				::std::decay_t<decltype(*begin(std::declval<T>()))>
			>::value;
	}

	template<typename T>
	constexpr bool ReverseIterable() {
		return has_rbegin<T>::value &&
			has_rend<T>::value &&
			// TODO: C++14 - std::rbegin(std::declval<T>())
			has_pre_inc<decltype(std::declval<T>().rbegin())>::value &&
			has_post_inc<decltype(std::declval<T>().rbegin())>::value &&
			std::is_same<
				Value_type<T>,
				::std::decay_t<decltype(*std::declval<T>().rbegin())>
			>::value;
	}

	template<typename T>
	constexpr bool BackInsertable() {
		return has_push_back<T,Value_type<T>>::value;
	}

	/**
	 * Generate curried calling convention for N-ary functions.
	 *
	 * \par Examples
	 *
	 * \code
	 * 	 struct _add3 : public make_curried_n<3,_add3> {
	 *   	int operator()(int x,int y,int z) {
	 *   		return x+y+z;
	 *   	}
	 *
	 * 		// Import curried overloads.
	 *   	using ftl::make_curried_n<3,_add3>::operator();
	 *	 } add3;
	 *
	 *   // ...
	 *
	 *   auto x = add3(1)(2)(3);
	 *   auto y = add3(1,2)(3);
	 *   auto z = add3(1)(2,3);
	 * \endcode
	 * 
	 * \ingroup prelude
	 */
	template<size_t N, typename F>
	struct make_curried_n {
		template<typename...Args>
		using partial_move = decltype( 
			_dtl::part(std::declval<F&&>(), std::declval<Args>()...)
		);

		template<typename...Args>
		using curry_move = decltype(
			curry<N-sizeof...(Args)>(std::declval<partial_move<Args...>>())
		);

		template<typename...Args>
		using partial_const = decltype( 
			_dtl::part(std::declval<const F&>(), std::declval<Args>()...)
		);

		template<typename...Args>
		using curry_const = decltype(
			curry<N-sizeof...(Args)>(std::declval<partial_const<Args...>>())
		);

		template<
			typename...Args, 
			size_t M = sizeof...(Args),
			typename = Requires< (N > M) >
		>
		curry_const<Args...> operator()(Args&&...args) const & {
            return curry<N-M>(
				_dtl::part(
					*static_cast<const F*>(this),
					std::forward<Args>(args)...
				)
			);
		}

		template<
			typename...Args, 
			size_t M = sizeof...(Args),
			typename = Requires< (N > M) >
		>
		curry_move<Args...> operator()(Args&&...args) && {
            return curry<N-M>(
				_dtl::part(
					std::move(*static_cast<F*>(this)),
					std::forward<Args>(args)...
				)
			);
		}
	};

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct _const : public _dtl::curried_binf<_const> {
		template<typename T, typename U>
		constexpr auto operator() (T&& t, U&&) const noexcept
		-> decltype(std::forward<T>(t)) {
			return std::forward<T>(t);
		}

		using _dtl::curried_binf<_const>::operator();
	} const_{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Function object sometimes useful with higher-order functions.
	 *
	 * Has the behaviour of a curried function of type
	 * \code
	 *   (T, U) -> T
	 * \endcode
	 *
	 * Basically, `const_` ignores the second parameter and behaves like
	 * `std::forward` for the first. This can be surprisingly useful in certain
	 * cases.
	 *
	 * \par Examples
	 *
	 * A simple, but useless example:
	 * \code
	 *   ftl::fmap(const_(42), std::list<int>{1,2,3}); // {42, 42, 42}
	 * \endcode
	 *
	 * Implementing monad's `operator>>` in terms of `const_` and `operator>>=`:
	 * \code
	 *   template<...>
	 *   M2 operator>> (M1 m1, M2 m2) {
	 *       return m1 >>= const_(m2);
	 *   }
	 * \endcode
	 *
	 * \ingroup prelude
	 */
	const_;
#endif

}
#endif

