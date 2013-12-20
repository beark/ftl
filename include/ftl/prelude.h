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

#include <tuple>
#include "type_traits.h"
#include "function.h"

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
	 * - `<ftl/type_traits.h>`
	 * - `<ftl/function.h>`
	 */

#ifndef DOCUMENTATION_GENERATOR
	constexpr struct identity {
		template<typename T>
		constexpr auto operator()(T&& t) const noexcept
		-> decltype(std::forward<T>(t)) {
			return std::forward<T>(t);
		}
	} id{};
#else
	struct ImplementationDefined {
	}
	/**
	 * Identity function object.
	 *
	 * Has semantics equivalent to a function of type
	 * \code
	 * (T) -> T
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
	id;
#endif

	
	/**
	 * A short-hand alias of `std::enable_if`.
	 *
	 * \note Consider this a temporary solution until concepts lite or something
	 * similar becomes readily available. As soon as that comes to pass, this
	 * construct will be considered deprecated.
	 *
	 * Example:
	 * \code
	 *   template<typename M, typename = Requires<Monad<M>()>>
	 *   void foo(const M& m) {
	 *       // Safely perform monadic operations on m
	 *   }
	 * \endcode
	 *
	 * \ingroup prelude
	 */
	template<bool Pred>
	using Requires = typename std::enable_if<Pred>::type;

	/**
	 * Used to distinguish in-place constructors from others.
	 *
	 * Used by e.g. `ftl::maybe` and `ftl::either` to make perfect forwarding
	 * tot he contained type(s) possible.
	 *
	 * \ingroup prelude
	 */
	struct inplace_tag {};

	// A number of helpers for tuple_apply
	namespace _dtl {

		// Helpers for tuple_apply
		template<
			typename F,
			typename...Ts,
			size_t...S>
		auto tup_apply(seq<S...>, const F& f, const std::tuple<Ts...>& t)
		-> typename std::result_of<F(Ts...)>::type {
			return f(std::get<S>(t)...);
		}

		template<
			typename F,
			typename...Ts,
			size_t...S>
		auto tup_apply(seq<S...>, const F& f, std::tuple<Ts...>&& t)
		-> typename std::result_of<F(Ts...)>::type {
			return f(std::get<S>(t)...);
		}
	}

	/**
	 * Invoke a function using a tuple's fields as parameters.
	 *
	 * Example:
	 * \code
	 *   void foo(int, float);
	 *
	 *   // Invokes foo with 1 and 2.f as arguments
	 *   ftl::tuple_apply(foo, std::make_tuple(1, 2.f));
	 * \endcode
	 *
	 * \ingroup prelude
	 */
	template<typename F, typename...Ts>
	auto tuple_apply(F&& f, const std::tuple<Ts...>& t)
	-> typename std::result_of<F(Ts...)>::type {
		using indices_t = typename gen_seq<0,sizeof...(Ts)-1>::type;
		return _dtl::tup_apply(indices_t(), std::forward<F>(f), t);
	}

	/**
	 * \overload
	 *
	 * \ingroup prelude
	 */
	template<typename F, typename...Ts>
	auto tuple_apply(F&& f, std::tuple<Ts...>&& t)
	-> typename std::result_of<F(Ts...)>::type {
		using indices_t = typename gen_seq<0,sizeof...(Ts)-1>::type;
		return _dtl::tup_apply(indices_t(), std::forward<F>(f), std::move(t));
	}

	// Implementation details of currying
	namespace _dtl {
		template<typename F, typename...Args1>
		class curried_fn {
			F f;
			std::tuple<Args1...> args1;

		public:
			curried_fn(const F& f, const std::tuple<Args1...>& args)
			: f(f), args1(args) {}

			curried_fn(F&& f, const std::tuple<Args1...>& args)
			: f(std::move(f)), args1(args) {}

			curried_fn(const F& f, std::tuple<Args1...>&& args)
			: f(f), args1(std::move(args)) {}

			curried_fn(F&& f, std::tuple<Args1...>&& args)
			: f(std::move(f)), args1(std::move(args)) {}

			template<
					typename...Args2,
					typename = Requires<
						is_callable<F,Args1...,Args2...>::value
					>
			>
			auto operator() (Args2&&...args2) const
			-> typename std::result_of<F(Args1...,Args2...)>::type {
				return tuple_apply(
					f,
					std::tuple_cat(
						args1,
						std::forward_as_tuple(std::forward<Args2>(args2)...)
					)
				);
			}

			template<
					typename...Args2,
					typename = Requires<
						!is_callable<F,Args1...,Args2...>::value
					>
			>
			auto operator() (Args2&&...args2) const
			-> curried_fn<F,Args1...,Args2...> {
				return curried_fn<F,Args1...,Args2...>{
					f,
					std::tuple_cat(
						args1,
						std::make_tuple(std::forward<Args2>(args2)...)
					)
				};
			}

		};

		template<typename F>
		class curried_fn<F> {
			F f;

		public:
			explicit curried_fn(const F& f)
			noexcept(std::is_nothrow_copy_constructible<F>::value)
			: f(f) {}

			explicit curried_fn(F&& f)
			noexcept(std::is_nothrow_move_constructible<F>::value)
			: f(std::move(f)) {}

			template<
					typename...Args,
					typename = Requires<
						is_callable<F,Args...>::value
					>
			>
			auto operator() (Args&&...args) const
			-> decltype(f(std::forward<Args>(args)...)) {
				return f(std::forward<Args>(args)...);
			}

			template<
					typename...Args,
					typename = Requires<
						!is_callable<F,Args...>::value
					>
			>
			auto operator() (Args&&...args) const
			-> curried_fn<F,Args...> {
				return curried_fn<F,Args...>(
					f,
					std::make_tuple(std::forward<Args>(args)...)
				);
			}
		};

		template<typename F, typename Arg>
		class partial_application {
			F f;
			Arg arg;

		public:
			constexpr partial_application(F f, Arg arg) 
				: f(std::move(f)), arg(std::move(arg)) 
			{ }

			template<typename...Args>
			constexpr auto operator()(Args&&...args) 
			-> typename std::result_of<F(Arg,Args...)>::type {
				return f(arg, std::forward<Args>(args)...);
			}
		};

		template<typename F>
		constexpr F part( F&& f ) {
			return std::forward<F>(f);
		}

		template<
			typename F, typename Arg1, typename...Args,
			typename PartOne = partial_application<F,Arg1>
		>
		constexpr auto part(F f, Arg1 arg1, Args...args) 
		-> decltype( part(std::declval<PartOne>(), std::declval<Args>()...) ) {
			return part(
				PartOne(std::move(f), std::move(arg1)),
				std::move(args)...
			);
		}

		template<size_t N, typename F>
		class curried_fn_n {
			F f;

		public:
			constexpr curried_fn_n(F f) : f(f) { }
			
			template<
                typename...Args,
                typename = Requires<N==sizeof...(Args)>
            >
			constexpr result_of<F(Args...)>  operator()(Args&&...args) {
				return f(std::forward<Args>(args)...);
			}

			template<
				typename...Args,
				size_t n = sizeof...(Args),
				typename = Requires<(N>n)>,
				typename Applied = decltype(part(f,std::declval<Args>()...))
			>
			constexpr curried_fn_n<N-n,Applied> operator()(Args&&...args) {
				return part(f,std::forward<Args>(args)...);
			}
		};
	}

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
			typename = Requires<!is_monomorphic<plain_type<F>>::value>
	>
#ifndef DOCUMENTATION_GENERATOR
	_dtl::curried_fn<plain_type<F>>
#else
	ImplementationDefined
#endif
	curry(F&& f) {
		return _dtl::curried_fn<plain_type<F>>(std::forward<F>(f));
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
	_dtl::curried_fn_n<N,plain_type<F>>
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
			has_pre_inc<decltype(std::begin(std::declval<T>()))>::value &&
			has_post_inc<decltype(std::begin(std::declval<T>()))>::value &&
			std::is_same<
				Value_type<T>,
				plain_type<decltype(*std::begin(std::declval<T>()))>
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
				plain_type<decltype(*std::declval<T>().rbegin())>
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
	private:
        // Enable currying if supplied too few arguments to call F.
        // (Otherwise F's operator() is called.)
        template<typename...Args>
        using Enable = Requires< (N>sizeof...(Args)) >;

        using curried = decltype(curry<N>(std::declval<F>()));
        template<typename...Args>
        using applied = result_of<curried(Args...)>;

	public:
		template<typename...Args, typename = Enable<Args...>>
		applied<Args...> operator()(Args&&...args) const & {
            return curry<N>(*static_cast<const F*>(this))(
                std::forward<Args>(args)...
            );
		}

		template<typename...Args, typename = Enable<Args...>>
		applied<Args...> operator()(Args&&...args) && {
            return curry<N>(std::move(*static_cast<const F*>(this)))(
                std::forward<Args>(args)...
            );
		}
	};

	namespace _dtl {
		// This struct is used to generate curried calling convention for
		// arbitrary binary functions
		template<typename F>
		struct curried_binf {
		private:
			template<typename P>
			struct curried {
				curried(const F& f, const P& p) : f(f), p(p) {}
				curried(const F& f, P&& p) : f(f), p(std::move(p)) {}
				curried(F&& f, const P& p) : f(std::move(f)), p(p) {}
				curried(F&& f, P&& p) : f(std::move(f)), p(std::move(p)) {}

				F f;
				P p;

				template<typename T>
				auto operator() (T&& t) const &
				-> decltype(f(p, std::forward<T>(t))) {
					return f(p, std::forward<T>(t));
				}

				template<typename T>
				auto operator() (T&& t) &&
				-> decltype(std::move(f)(std::move(p), std::forward<T>(t))) {
					return std::move(f)(std::move(p), std::forward<T>(t));
				}
			};

		public:
			template<typename P>
			curried<plain_type<P>> operator() (P&& p) const & {
				return curried<plain_type<P>>(
						*static_cast<const F*>(this),
						std::forward<P>(p)
				);
			}

			template<typename P>
			curried<plain_type<P>> operator() (P&& p) && {
				return curried<plain_type<P>>(
						std::move(*static_cast<F*>(this)),
						std::forward<P>(p)
				);
			}
		};

		// This struct is used to generate curried calling convention for
		// arbitrary ternary functions
		template<typename F>
		struct curried_ternf {
		private:
			template<typename P1>
			struct curried1 {
			private:
				template<typename P2>
				struct curried {
					curried(const F& f, const P1& p1, const P2& p2)
					: f(f), p1(p1), p2(p2) {}

					curried(const F& f, const P1& p1, P2&& p2)
					: f(f), p1(p1), p2(std::move(p2)) {}

					curried(const F& f, P1&& p1, const P2& p2)
					: f(f), p1(std::move(p1)), p2(p2) {}

					curried(F&& f, const P1& p1, const P2& p2)
					: f(std::move(f)), p1(p1), p2(p2) {}

					curried(const F& f, P1&& p1, P2&& p2)
					: f(f), p1(std::move(p1)), p2(std::move(p2)) {}

					curried(F&& f, const P1& p1, P2&& p2)
					: f(std::move(f)), p1(p1), p2(std::move(p2)) {}

					curried(F&& f, P1&& p1, const P2& p2)
					: f(std::move(f)), p1(std::move(p1)), p2(p2) {}

					curried(F&& f, P1&& p1, P2&& p2)
					: f(std::move(f)), p1(std::move(p1)), p2(std::move(p2)) {}

					F f;
					P1 p1;
					P2 p2;

					template<typename P3>
					auto operator() (P3&& p3) const &
					-> decltype(f(p1, p2, std::forward<P3>(p3))) {
						return f(p1, p2, std::forward<P3>(p3));
					}

					template<typename P3>
					auto operator() (P3&& p3) &&
					-> decltype(std::move(f)(
								std::move(p1), std::move(p2), std::forward<P3>(p3)
					)) {
						return std::move(f)(
								std::move(p1), std::move(p2), std::forward<P3>(p3)
						);
					}
				};

			public:
				curried1(const F& f, const P1& p) : f(f), p(p) {}
				curried1(const F& f, P1&& p) : f(f), p(std::move(p)) {}
				curried1(F&& f, const P1& p) : f(std::move(f)), p(p) {}
				curried1(F&& f, P1&& p) : f(std::move(f)), p(std::move(p)) {}

				F f;
				P1 p;

				template<typename P2, typename P3>
				auto operator() (P2&& p2, P3&& p3) const &
				-> decltype(f(p, std::forward<P2>(p2), std::forward<P3>(p3))) {
					return f(p, std::forward<P2>(p2), std::forward<P3>(p3));
				}

				template<typename P2, typename P3>
				auto operator() (P2&& p2, P3&& p3) &&
				-> decltype(std::move(f)(
							std::move(p), std::forward<P2>(p2), std::forward<P3>(p3))
				) {
					return std::move(f)(
							std::move(p), std::forward<P2>(p2), std::forward<P3>(p3)
					);
				}

				template<typename P2>
				curried<plain_type<P2>> operator() (P2&& p2) const &  {
					return curried<plain_type<P2>>(f, p, std::forward<P2>(p2));
				}

				template<typename P2>
				curried<plain_type<P2>> operator() (P2&& p2) && {
					return curried<plain_type<P2>>(
							std::move(f), std::move(p), std::forward<P2>(p2)
					);
				}
			};

			template<typename P1, typename P2>
			struct curried2 {
				F f;
				P1 p1;
				P2 p2;

				curried2(const F& f, const P1& p1, const P2& p2)
				noexcept(std::is_nothrow_copy_constructible<F>::value
						&& std::is_nothrow_copy_constructible<P1>::value
						&& std::is_nothrow_copy_constructible<P2>::value)
				: f(f), p1(p1), p2(p2) {}

				curried2(const F& f, const P1& p1, P2&& p2)
				noexcept(std::is_nothrow_copy_constructible<F>::value
						&& std::is_nothrow_copy_constructible<P1>::value
						&& std::is_nothrow_move_constructible<P2>::value)
				: f(f), p1(p1), p2(std::move(p2)) {}

				curried2(const F& f, P1&& p1, const P2& p2)
				noexcept(std::is_nothrow_copy_constructible<F>::value
						&& std::is_nothrow_move_constructible<P1>::value
						&& std::is_nothrow_copy_constructible<P2>::value)
				: f(f), p1(std::move(p1)), p2(p2) {}

				curried2(F&& f, const P1& p1, const P2& p2)
				noexcept(std::is_nothrow_move_constructible<F>::value
						&& std::is_nothrow_copy_constructible<P1>::value
						&& std::is_nothrow_copy_constructible<P2>::value)
				: f(std::move(f)), p1(p1), p2(p2) {}

				curried2(const F& f, P1&& p1, P2&& p2)
				noexcept(std::is_nothrow_copy_constructible<F>::value
						&& std::is_nothrow_move_constructible<P1>::value
						&& std::is_nothrow_move_constructible<P2>::value)
				: f(f), p1(std::move(p1)), p2(std::move(p2)) {}

				curried2(F&& f, const P1& p1, P2&& p2)
				noexcept(std::is_nothrow_move_constructible<F>::value
						&& std::is_nothrow_copy_constructible<P1>::value
						&& std::is_nothrow_move_constructible<P2>::value)
				: f(std::move(f)), p1(p1), p2(std::move(p2)) {}

				curried2(F&& f, P1&& p1, const P2& p2)
				noexcept(std::is_nothrow_move_constructible<F>::value
						&& std::is_nothrow_move_constructible<P1>::value
						&& std::is_nothrow_copy_constructible<P2>::value)
				: f(std::move(f)), p1(std::move(p1)), p2(p2) {}

				curried2(F&& f, P1&& p1, P2&& p2)
				noexcept(std::is_nothrow_move_constructible<F>::value
						&& std::is_nothrow_move_constructible<P1>::value
						&& std::is_nothrow_move_constructible<P2>::value)
				: f(std::move(f)), p1(std::move(p1)), p2(std::move(p2)) {}

				template<typename P3>
				auto operator() (P3&& p3) const &
				-> decltype(f(p1, p2, std::forward<P3>(p3))) {
					return f(p1, p2, std::forward<P3>(p3));
				}

				template<typename P3>
				auto operator() (P3&& p3) &&
				-> decltype(std::move(f)(
							std::move(p1), std::move(p2), std::forward<P3>(p3)
				)) {
					return std::move(f)(
							std::move(p1), std::move(p2), std::forward<P3>(p3)
					);
				}
			};

		public:
			template<typename P>
			curried1<plain_type<P>> operator() (P&& p) const & {
				return curried1<plain_type<P>>(
						*static_cast<const F*>(this),
						std::forward<P>(p)
				);
			}

			template<typename P>
			curried1<plain_type<P>> operator() (P&& p) && {
				return curried1<plain_type<P>>(
						std::move(*static_cast<F*>(this)),
						std::forward<P>(p)
				);
			}

			template<typename P1, typename P2>
			curried2<plain_type<P1>,plain_type<P2>>
			operator() (P1&& p1, P2&& p2) const & {
				return curried2<plain_type<P1>,plain_type<P2>>(
					*static_cast<const F*>(this),
					std::forward<P1>(p1), std::forward<P2>(p2)
				);
			}

			template<typename P1, typename P2>
			curried2<plain_type<P1>,plain_type<P2>>
			operator() (P1&& p1, P2&& p2) && {
				return curried2<plain_type<P1>,plain_type<P2>>(
					std::move(*static_cast<F*>(this)),
					std::forward<P1>(p1), std::forward<P2>(p2)
				);
			}
		};
	}

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

