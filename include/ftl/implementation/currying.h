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
#ifndef FTL_IMPL_CURRYING_H
#define FTL_IMPL_CURRYING_H

#include <type_traits>
#include "../type_functions.h"
#include "../type_traits.h"
#include "tuple_apply.h"

namespace ftl {
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
						std::move(p), std::forward<P2>(p2), std::forward<P3>(p3)
				)) {
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
					typename = typename std::enable_if<
						is_callable<F,Args1...,Args2...>::value
					>::type
			>
			auto operator() (Args2&&...args2) const
			-> typename std::result_of<F(Args1...,Args2...)>::type {
				return tup_apply(
					f,
					std::tuple_cat(
						args1,
						std::forward_as_tuple(std::forward<Args2>(args2)...)
					)
				);
			}

			template<
					typename...Args2,
					typename = typename std::enable_if<
						!is_callable<F,Args1...,Args2...>::value
					>::type
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
					typename = typename std::enable_if<
						is_callable<F,Args...>::value
					>::type
			>
			auto operator() (Args&&...args) const
			-> decltype(f(std::forward<Args>(args)...)) {
				return f(std::forward<Args>(args)...);
			}

			template<
					typename...Args,
					typename = typename std::enable_if<
						!is_callable<F,Args...>::value
					>::type
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
			constexpr auto operator()(Args&&...args) const &
			-> result_of<F(Arg,Args...)> {
				return f(arg, std::forward<Args>(args)...);
			}

			template<typename...Args>
			auto operator()(Args&&...args) &&
			-> result_of<F(Arg,Args...)> {
				return std::move(f)(arg, std::forward<Args>(args)...);
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

			// The arity of F after applying so many arguments.
			template<typename...Args>
			struct left_over : 
				std::integral_constant<size_t, N-sizeof...(Args)>
			{
			};

			// TODO: Why does this definition not work with gcc?
			//template<typename...Args>
			//static constexpr size_t left_over() {
			//	return N-sizeof...(Args);
			//}
			
			template<typename...Args>
			using EnableCall =
				typename std::enable_if<left_over<Args...>::value==0>::type;
			
			template<typename...Args>
			using EnableCurry =
				typename std::enable_if<(left_over<Args...>::value>0)>::type;
			
			// The type f after applying Args.
			template<typename...Args>
			using applied_type = curried_fn_n<
				left_over<Args...>::value,
				decltype(part(f,std::declval<Args>()...))
			>;
		public:
			constexpr curried_fn_n(F f) : f(f) { }
			
			// Call f.
			template<typename...Args, typename = EnableCall<Args...>>
			constexpr result_of<F(Args...)>  operator()(Args&&...args) const & {
				return f(std::forward<Args>(args)...);
			}

			template<typename...Args, typename = EnableCall<Args...>>
			result_of<F(Args...)>  operator()(Args&&...args) && {
				return std::move(f)(std::forward<Args>(args)...);
			}

            // Curry f.
			template<typename...Args, typename = EnableCurry<Args...>>
			constexpr applied_type<Args...> operator()(Args&&...args) const & {
				return part(f,std::forward<Args>(args)...);
			}

			template<typename...Args, typename = EnableCurry<Args...>>
			applied_type<Args...> operator()(Args&&...args) && {
				return part(f,std::forward<Args>(args)...);
			}
		};
	}
}
#endif


