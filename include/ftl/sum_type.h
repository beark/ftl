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
#ifndef FTL_SUM_TYPE_H
#define FTL_SUM_TYPE_H

#include <cassert>
#include "type_functions.h"
#include "concepts/basic.h"
#include "concepts/orderable.h"
#include "implementation/recursive_union.h"

namespace ftl {

	/**
	 * \defgroup sum_type Sum Type
	 *
	 * Sum types, related functions and concepts.
	 *
	 * \code
	 *   #include <ftl/sum_type.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - `<stdexcept>`
	 * - `<memory>`
	 * - `<string>`
	 * - \ref typelevel
	 * - \ref concepts_basic
	 * - \ref orderablepg
	 */

	/**
	 * A type that can be used as default match clause in a pattern match.
	 *
	 * \par Examples
	 *
	 * \code
	 *   sum_type<A,B,C> x = ...;
	 *   x.match(
	 *       [](A a){ return f(a); },
	 *       [](match_all){ return g(); }
	 *   );
	 * \endcode
	 *
	 * \note If present, this match clause should always be the final one. If
	 *       there are additional match clauses after `match_all`, they will
	 *       never be called.
	 *
	 * \ingroup sum_type
	 */
	struct match_all
	{
		template<class T>
		constexpr match_all(T&&) noexcept {}
	};

	namespace dtl_
	{
		template<class, class...>
		struct is_type_set;

		template<class T>
		struct is_type_set<T> : ::std::true_type {};

		template<class T, class...Ts>
		struct is_type_set<T,T,Ts...> : ::std::false_type {};

		template<class T, class U, class...Ts>
		struct is_type_set<T,U,Ts...>
		{
			static constexpr bool value =
				is_type_set<T,Ts...>::value
				&& is_type_set<U,Ts...>::value;
		};

		// We have a match if the decayed function argument type matches
		template<class T, class FnArg>
		struct is_match
			: ::std::is_same<T, ::std::decay_t<FnArg>> {};

		// Or if the actual types are
		template<class T>
		struct is_match<T,T> : ::std::true_type {};

		// Or if we're matching against match_all
		template<class T>
		struct is_match<T,::ftl::match_all> : ::std::true_type {};

		// Otherwise, there's no match
		template<class, class>
		struct is_match_exhaustive : ::std::false_type {};

		// A match is exhaustive if we've functions to spare
		template<typename...Fs>
		struct is_match_exhaustive<type_seq<Fs...>,type_seq<>>
			: ::std::true_type {};

		// A match is exhaustive if the final function accepts the final type
		template<typename F, typename T>
		struct is_match_exhaustive<type_seq<F>,type_seq<T>>
				: is_match<T,argument_type<F,0>> {};

		//A match is NOT exhaustive if there are left-over types
		template<typename T, typename...Ts>
		struct is_match_exhaustive<type_seq<>,type_seq<T,Ts...>>
			: ::std::false_type {};

		template<typename F, typename...Fs, typename T, typename...Ts>
		struct is_match_exhaustive<type_seq<F,Fs...>,type_seq<T,Ts...>>
		{
			static constexpr bool value =
				( is_match<T,argument_type<F,0>>::value
					&& is_match_exhaustive<type_seq<Fs...>,type_seq<Ts...>>::value)
				||
				( is_match_exhaustive<type_seq<F,Fs...>,type_seq<Ts...>>::value
					&& is_match_exhaustive<type_seq<Fs...>,type_seq<T>>::value);
		};

		template<class, class...>
		struct match_cases_are_in_type : ::std::false_type{};

		template<class...Ts>
		struct match_cases_are_in_type<type_seq<>,Ts...> : ::std::true_type{};

		template<class F, class...Fs, class...Ts>
		struct match_cases_are_in_type<type_seq<F,Fs...>,Ts...>
		{
			static constexpr bool value =
				type_is_in_v<::std::decay_t<argument_type<F,0>>,Ts...>
				&& match_cases_are_in_type<type_seq<Fs...>,Ts...>::value;
		};

		template<type_layout Layout, typename...Ts>
		struct sum_type_;

		template<type_layout, class...>
		struct match_selector;

		template<type_layout Layout>
		struct match_selector<Layout>
		{
			template<
				class T, class U, class F, class...Fs,
				class = ::std::enable_if_t<is_match<T,argument_type<F,0>>::value>
			>
			static constexpr auto invoke(type_t<T>, U&& val, F&& f, Fs&&...)
			{
				return ::std::forward<F>(f)(::std::forward<U>(val));
			}

			template<
				class F, class T, class U, class...Fs,
				class = ::std::enable_if_t<!is_match<T,argument_type<F,0>>::value>
			>
			static constexpr auto invoke(type_t<T>, U&& val, F&&, Fs&&...fs)
			{
				return match_selector::invoke(
					type<T>,
					::std::forward<U>(val),
					::std::forward<Fs>(fs)...);
			}
		};

		template<type_layout Layout, size_t I, size_t J, size_t...Is, class T, class...Ts>
		struct match_selector<Layout,::std::index_sequence<I,J,Is...>,T,Ts...>
		{
			template<class...Fs> static constexpr auto
			invoke(const recursive_union_<Layout,T,Ts...>& s, size_t i, Fs&&...fs)
			{
				return i == I
					? match_selector<Layout>::invoke(type<T>, s.val, ::std::forward<Fs>(fs)...)
					: match_selector<Layout,::std::index_sequence<J,Is...>,Ts...>::invoke(
							s.rem, i, std::forward<Fs>(fs)...
						);
			}

			template<class...Fs> static constexpr auto
			invoke(recursive_union_<Layout,T,Ts...>& s, size_t i, Fs&&...fs)
			{
				return i == I
					? match_selector<Layout>::invoke(type<T>, s.val, ::std::forward<Fs>(fs)...)
					: match_selector<Layout,::std::index_sequence<J,Is...>,Ts...>::invoke(
							s.rem, i, std::forward<Fs>(fs)...
						);
			}

			template<class...Fs> static constexpr auto
			invoke(recursive_union_<Layout,T,Ts...>&& s, size_t i, Fs&&...fs)
			{
				return i == I
					? match_selector<Layout>::invoke(
							type<T>, ::std::move(s.val), ::std::forward<Fs>(fs)...
						)
					: match_selector<Layout,::std::index_sequence<J,Is...>,Ts...>::invoke(
							::std::move(s.rem), i, std::forward<Fs>(fs)...
						);
			}
		};

		template<type_layout Layout, size_t I, typename T, typename...Ts>
		struct match_selector<Layout, ::std::index_sequence<I>,T,Ts...>
		{
			template<typename...Fs> static constexpr auto
			invoke(const recursive_union_<Layout,T,Ts...>& s, size_t i, Fs&&...fs)
			{
				assert(i == I);

				return match_selector<Layout>::invoke(
					type<T>, s.val, ::std::forward<Fs>(fs)...
				);
			}

			template<typename...Fs> static constexpr auto
			invoke(recursive_union_<Layout,T,Ts...>& s, size_t i, Fs&&...fs)
			{
				assert(i == I);

				return match_selector<Layout>::invoke(
					type<T>, s.val, ::std::forward<Fs>(fs)...
				);
			}

			template<typename...Fs> static constexpr auto
			invoke(recursive_union_<Layout,T,Ts...>&& s, size_t i, Fs&&...fs)
			{
				assert(i == I);

				return match_selector<Layout>::invoke(
					type<T>, ::std::move(s.val), ::std::forward<Fs>(fs)...
				);
			}
		};

		template<typename...Ts>
		struct sum_type_<type_layout::trivially_copyable,Ts...>
		{
			static_assert(
				dtl_::is_type_set<Ts...>::value,
				"Each member type of a sum type must be unique"
			);

			static_assert(sizeof...(Ts) > 0, "Empty sum types are not allowed");

			sum_type_() = delete;
			sum_type_(const sum_type_&) = default;
			sum_type_(sum_type_&&) = default;

			template<class T, class...Args>
			constexpr sum_type_(type_t<T> s, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,Args...>::value)
				: data(s, std::forward<Args>(args)...), cons(index_of_v<T,Ts...>)
			{}

			template<class T, class U, class...Args>
			constexpr sum_type_(type_t<T> s, ::std::initializer_list<U> init_list, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,::std::initializer_list<U>,Args...>::value)
				: data(s, init_list, std::forward<Args>(args)...), cons(index_of_v<T,Ts...>)
			{}

			~sum_type_() = default;

			template<class T, class...Args>
			auto emplace(type_t<T> s, Args&&... args) noexcept
			-> ::std::enable_if_t<::std::is_nothrow_constructible<T,Args...>::value>
			{
				// Since we know we're dealing with trivial types, we can just
				// replace the storage, no need to destruct
				new (&data) recursive_union_<type_layout::trivially_copyable, Ts...>(s, std::forward<Args>(args)...);
				cons = index_of_v<T,Ts...>;
			}

			template<class T>
			constexpr auto unsafe_get() const &
			-> ::std::enable_if_t<type_is_in_v<T,Ts...>, const T&>
			{
				if (cons != index_of_v<T,Ts...>)
					throw invalid_sum_type_access();

				return data.get(type<T>);
			}

			template<class T>
			constexpr auto unsafe_get() &
			-> ::std::enable_if_t<type_is_in_v<T,Ts...>,T&>
			{
				if (cons != index_of_v<T,Ts...>)
					throw invalid_sum_type_access();

				return data.get(type<T>);
			}

			template<class T>
			constexpr auto unsafe_get() &&
			-> ::std::enable_if_t<type_is_in_v<T,Ts...>,T&&>
			{
				if (cons != index_of_v<T,Ts...>)
					throw invalid_sum_type_access();

				return ::std::move(data).get(type<T>);
			}

			template<class...MatchArms>
			constexpr auto match(MatchArms&&...matchArms) const &
			{
				static_assert(
					::ftl::dtl_::is_match_exhaustive<
						type_seq<MatchArms...>,type_seq<Ts...>>::value,
					"All matches must be exhaustive");

				static_assert(
					::ftl::dtl_::match_cases_are_in_type<
						type_seq<MatchArms...>,match_all,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<type_layout::trivially_copyable,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						data, cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			template<typename...MatchArms>
			constexpr auto match(MatchArms&&...matchArms) &
			{
				static_assert(
					::ftl::dtl_::is_match_exhaustive<
					type_seq<MatchArms...>,type_seq<Ts...>>::value,
					"All matches must be exhaustive");

				static_assert(
					::ftl::dtl_::match_cases_are_in_type<
						type_seq<MatchArms...>,match_all,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<type_layout::trivially_copyable,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						data, cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			template<typename...MatchArms>
			constexpr auto match(MatchArms&&...matchArms) &&
			{
				static_assert(
					::ftl::dtl_::is_match_exhaustive<
					type_seq<MatchArms...>,type_seq<Ts...>>::value,
					"All matches must be exhaustive");

				static_assert(
					::ftl::dtl_::match_cases_are_in_type<
						type_seq<MatchArms...>,match_all,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<type_layout::trivially_copyable,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						::std::move(data), cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			template<class T, class = ::std::enable_if_t<type_is_in_v<::std::decay_t<T>,Ts...>>>
			sum_type_& operator= (T&& t) noexcept
			{
				emplace(type<::std::decay_t<T>>, ::std::forward<T>(t));

				return *this;
			}

			sum_type_& operator= (const sum_type_& other) = default;
			sum_type_& operator= (sum_type_&& other) = default;

			constexpr void swap(sum_type_& other) noexcept
			{
				auto tempCons = other.cons;
				other.cons = cons;
				cons = tempCons;

				auto temp = ::std::move(other.data);
				other.data = ::std::move(data);
				data = ::std::move(temp);
			}

			template<class T>
			constexpr bool is() const noexcept
			{
				return index_of_v<T,Ts...> == cons;
			}

			constexpr bool operator== (const sum_type_& rhs) const noexcept
			{
				return cons == rhs.cons && data.compare(cons, rhs.data);
			}

			constexpr bool operator!= (const sum_type_& rhs) const noexcept
			{
				return !operator== (rhs);
			}

		private:
			recursive_union_<type_layout::trivially_copyable, Ts...> data;
			size_t cons;
		};

		template<class...Ts>
		constexpr void swap(sum_type_<type_layout::trivially_copyable,Ts...>& a, sum_type_<type_layout::trivially_copyable,Ts...>& b) noexcept
		{
			a.swap(b);
		}

		template<class...Ts>
		struct sum_type_<type_layout::trivial_destructor,Ts...>
		{
			static_assert(
				dtl_::is_type_set<Ts...>::value,
				"Each element type of a sum type must be unique"
			);

			static_assert(sizeof...(Ts) > 0, "Empty sum types are not allowed");

			sum_type_() = delete;

			constexpr sum_type_(const sum_type_& other)
			noexcept(All<std::is_nothrow_copy_constructible, Ts...>::value)
			: data(other.data, other.cons), cons(other.cons)
			{}

			sum_type_(sum_type_&& other)
			noexcept(All<std::is_nothrow_move_constructible, Ts...>::value)
			: data(other.data, other.cons), cons(other.cons)
			{}

			template<class T, class...Args>
			explicit constexpr sum_type_(type_t<T> s, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,Args...>::value)
				: data(s, std::forward<Args>(args)...), cons(index_of_v<T,Ts...>)
			{}

			template<class T, class U, class...Args>
			constexpr sum_type_(type_t<T> s, ::std::initializer_list<U> init_list, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,::std::initializer_list<U>,Args...>::value)
				: data(s, init_list, ::std::forward<Args>(args)...), cons(index_of_v<T,Ts...>)
			{}

			~sum_type_() = default;

			template<typename T, typename...Args>
			auto emplace(type_t<T> s, Args&&... args) noexcept
			-> ::std::enable_if_t<::std::is_nothrow_constructible<T,Args...>::value>
			{
				// All element types are trivially destructible, so we can just write
				// over the data.
				new (&data) recursive_union_<type_layout::trivial_destructor, Ts...> (s, ::std::forward<Args>(args)...);
				cons = index_of_v<T,Ts...>;
			}

			template<class T>
			constexpr auto unsafe_get() const &
			-> ::std::enable_if_t<type_is_in_v<T,Ts...>, const T&>
			{
				if (cons != index_of_v<T,Ts...>)
					throw invalid_sum_type_access();

				return data.get(type<T>);
			}

			template<class T>
			constexpr auto unsafe_get() &
			-> ::std::enable_if_t<type_is_in_v<T,Ts...>,T&>
			{
				if (cons != index_of_v<T,Ts...>)
					throw invalid_sum_type_access();

				return data.get(type<T>);
			}

			template<class T>
			constexpr auto unsafe_get() &&
			-> ::std::enable_if_t<type_is_in_v<T,Ts...>,T&&>
			{
				if (cons != index_of_v<T,Ts...>)
					throw invalid_sum_type_access();

				return ::std::move(data).get(type<T>);
			}

			template<typename...MatchArms>
			constexpr auto match(MatchArms&&...matchArms) const &
			{
				static_assert(
					::ftl::dtl_::is_match_exhaustive<
						type_seq<MatchArms...>,type_seq<Ts...>>::value,
					"All matches must be exhaustive");

				static_assert(
					::ftl::dtl_::match_cases_are_in_type<
						type_seq<MatchArms...>,match_all,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<type_layout::trivial_destructor,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						data, cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			template<typename...MatchArms>
			constexpr auto match(MatchArms&&...matchArms) &
			{
				static_assert(
					::ftl::dtl_::is_match_exhaustive<
					type_seq<MatchArms...>,type_seq<Ts...>>::value,
					"All matches must be exhaustive");

				static_assert(
					::ftl::dtl_::match_cases_are_in_type<
						type_seq<MatchArms...>,match_all,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<type_layout::trivial_destructor,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						data, cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			template<typename...MatchArms>
			constexpr auto match(MatchArms&&...matchArms) &&
			{
				static_assert(
					::ftl::dtl_::is_match_exhaustive<
					type_seq<MatchArms...>,type_seq<Ts...>>::value,
					"All matches must be exhaustive");

				static_assert(
					::ftl::dtl_::match_cases_are_in_type<
						type_seq<MatchArms...>,match_all,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<type_layout::trivial_destructor,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						::std::move(data), cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			sum_type_& operator= (const sum_type_& other) noexcept
			{
				static_assert(
					All<std::is_nothrow_copy_constructible,Ts...>::value
					&& All<std::is_nothrow_copy_assignable,Ts...>::value,
					"Cannot copy assign a sum_type unless all element types are nothrow "
					"copyable (constructible & assignable)."
				);

				if (cons == other.cons)
				{
					data.copy(other.data, cons);
				}
				else
				{
					cons = other.cons;
					new (&data) recursive_union_<type_layout::trivial_destructor, Ts...> (other.data, cons);
				}

				return *this;
			}

			sum_type_& operator= (sum_type_&& other) noexcept
			{
				static_assert(
					All<::std::is_nothrow_move_constructible, Ts...>::value
					&& All<::std::is_nothrow_move_assignable, Ts...>::value,
					"Cannot move assign sum_type unless all element types are nothrow "
					"movable (constructible & assignable)."
				);

				if (cons == other.cons)
				{
					data.move(::std::move(other.data), cons);
				}
				else
				{
					cons = other.cons;
					new (&data) recursive_union_<type_layout::trivial_destructor, Ts...> (::std::move(other.data), cons);
				}

				return *this;
			}

			template<
				class T, class U = ::std::decay_t<T>,
				class = std::enable_if_t<type_is_in_v<U,Ts...>>>
			const sum_type_& operator= (T&& t) noexcept
			{
				static_assert(
					::std::is_nothrow_copy_assignable<U>::value
					&& ::std::is_nothrow_copy_constructible<U>::value,
					"Cannot assign to a sum type unless the target type is nothrow copy "
					"constructible and assignable."
				);

				if (cons == index_of_v<U,Ts...>)
				{
					data.assign(type<U>, ::std::forward<T>(t));
				}
				else
				{
					cons = index_of_v<U,Ts...>;
					new (&data) recursive_union_<type_layout::trivial_destructor, Ts...> (type<U>, ::std::forward<T>(t));
				}

				return *this;
			}

			template<class T>
			constexpr bool is() const noexcept
			{
				return index_of_v<T,Ts...> == cons;
			}

			constexpr bool operator== (const sum_type_& rhs) const noexcept
			{
				return cons == rhs.cons && data.compare(cons, rhs.data);
			}

			constexpr bool operator!= (const sum_type_& rhs) const noexcept
			{
				return !operator== (rhs);
			}

		private:
			recursive_union_<type_layout::trivial_destructor, Ts...> data;
			size_t cons;
		};

		template<class...Ts>
		struct sum_type_<type_layout::complex,Ts...>
		{
			static_assert(
				dtl_::is_type_set<Ts...>::value,
				"Each element type of a sum type must be unique"
			);

			static_assert(sizeof...(Ts) > 0, "Empty sum types are not allowed");

			sum_type_() = delete;

			constexpr sum_type_(const sum_type_& other)
			noexcept(All<std::is_nothrow_copy_constructible, Ts...>::value)
			: data(other.data, other.cons), cons(other.cons)
			{}

			sum_type_(sum_type_&& other)
			noexcept(All<std::is_nothrow_move_constructible, Ts...>::value)
			: data(other.data, other.cons), cons(other.cons)
			{}

			template<class T, class...Args>
			explicit constexpr sum_type_(type_t<T> s, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,Args...>::value)
				: data(s, std::forward<Args>(args)...), cons(index_of_v<T,Ts...>)
			{}

			template<class T, class U, class...Args>
			constexpr sum_type_(type_t<T> s, ::std::initializer_list<U> init_list, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,::std::initializer_list<U>,Args...>::value)
				: data(s, init_list, ::std::forward<Args>(args)...), cons(index_of_v<T,Ts...>)
			{}

			~sum_type_()
			{
				data.destroy(cons);
			}

			template<typename T, typename...Args>
			auto emplace(type_t<T> s, Args&&... args) noexcept
			-> ::std::enable_if_t<
				All<::std::is_nothrow_destructible,Ts...>::value
				&& ::std::is_nothrow_constructible<T,Args...>::value>
			{
				data.destroy(cons);
				new (&data) recursive_union_<type_layout::complex, Ts...> (s, ::std::forward<Args>(args)...);
				cons = index_of_v<T,Ts...>;
			}

			template<class T>
			constexpr auto unsafe_get() const &
			-> ::std::enable_if_t<type_is_in_v<T,Ts...>, const T&>
			{
				if (cons != index_of_v<T,Ts...>)
					throw invalid_sum_type_access();

				return data.get(type<T>);
			}

			template<class T>
			constexpr auto unsafe_get() &
			-> ::std::enable_if_t<type_is_in_v<T,Ts...>, T&>
			{
				if (cons != index_of_v<T,Ts...>)
					throw invalid_sum_type_access();

				return data.get(type<T>);
			}

			template<class T>
			constexpr auto unsafe_get() &&
			-> ::std::enable_if_t<type_is_in_v<T,Ts...>, T&&>
			{
				if (cons != index_of_v<T,Ts...>)
					throw invalid_sum_type_access();

				return ::std::move(data).get(type<T>);
			}

			template<typename...MatchArms>
			constexpr auto match(MatchArms&&...matchArms) const &
			{
				static_assert(
					::ftl::dtl_::is_match_exhaustive<
						type_seq<MatchArms...>,type_seq<Ts...>>::value,
					"All matches must be exhaustive");

				static_assert(
					::ftl::dtl_::match_cases_are_in_type<
						type_seq<MatchArms...>,match_all,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<type_layout::complex,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						data, cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			template<typename...MatchArms>
			constexpr auto match(MatchArms&&...matchArms) &
			{
				static_assert(
					::ftl::dtl_::is_match_exhaustive<
					type_seq<MatchArms...>,type_seq<Ts...>>::value,
					"All matches must be exhaustive");

				static_assert(
					::ftl::dtl_::match_cases_are_in_type<
						type_seq<MatchArms...>,match_all,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<type_layout::complex,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						data, cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			template<typename...MatchArms>
			constexpr auto match(MatchArms&&...matchArms) &&
			{
				static_assert(
					::ftl::dtl_::is_match_exhaustive<
					type_seq<MatchArms...>,type_seq<Ts...>>::value,
					"All matches must be exhaustive");

				static_assert(
					::ftl::dtl_::match_cases_are_in_type<
						type_seq<MatchArms...>,match_all,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<type_layout::complex,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						::std::move(data), cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			sum_type_& operator= (const sum_type_& other) noexcept
			{
				static_assert(
					All<std::is_nothrow_destructible,Ts...>::value,
					"Cannot copy assign a sum_type unless all element types are nothrow "
					"destructible."
				);

				static_assert(
					All<std::is_nothrow_copy_constructible,Ts...>::value
					&& All<std::is_nothrow_copy_assignable,Ts...>::value,
					"Cannot copy assign a sum_type unless all element types are nothrow "
					"copyable (constructible & assignable)."
				);

				if (cons == other.cons)
				{
					data.copy(other.data, cons);
				}
				else
				{
					data.destroy(cons);
					cons = other.cons;
					new (&data) recursive_union_<type_layout::complex, Ts...> (other.data, cons);
				}

				return *this;
			}

			sum_type_& operator= (sum_type_&& other) noexcept
			{
				static_assert(
					All<::std::is_nothrow_destructible, Ts...>::value,
					"Cannot move assign sum types unless all element types are nothrow "
					"destructible."
				);

				static_assert(
					All<::std::is_nothrow_move_constructible, Ts...>::value
					&& All<::std::is_nothrow_move_assignable, Ts...>::value,
					"Cannot move assign sum_type unless all element types are nothrow "
					"movable (constructible & assignable)."
				);

				if (cons == other.cons)
				{
					data.move(::std::move(other.data), cons);
				}
				else
				{
					data.destroy(cons);
					cons = other.cons;
					new (&data) recursive_union_<type_layout::complex, Ts...> (::std::move(other.data), cons);
				}

				return *this;
			}

			template<
				class T, class U = ::std::decay_t<T>,
				class = std::enable_if_t<type_is_in_v<U,Ts...>>>
			const sum_type_& operator= (T&& t) noexcept
			{
				static_assert(
					All<::std::is_nothrow_destructible,Ts...>::value,
					"Cannot assign to a sum type unless all element types are nothrow "
					"destructible."
				);

				static_assert(
					::std::is_nothrow_copy_assignable<U>::value
					&& ::std::is_nothrow_copy_constructible<U>::value,
					"Cannot assign to a sum type unless the target type is nothrow copy "
					"constructible and assignable."
				);

				if (cons == index_of_v<U,Ts...>)
				{
					data.assign(type<U>, ::std::forward<T>(t));
				}
				else
				{
					data.destroy(cons);
					cons = index_of_v<U,Ts...>;
					new (&data) recursive_union_<type_layout::complex, Ts...> (type<U>, ::std::forward<T>(t));
				}

				return *this;
			}

			template<class T>
			constexpr bool is() const noexcept
			{
				return index_of_v<T,Ts...> == cons;
			}

			constexpr bool operator== (const sum_type_& rhs) const noexcept
			{
				return cons == rhs.cons && data.compare(cons, rhs.data);
			}

			constexpr bool operator!= (const sum_type_& rhs) const noexcept
			{
				return !operator== (rhs);
			}

		private:
			recursive_union_<type_layout::complex, Ts...> data;
			size_t cons;
		};
	}

	/**
	 * A basic sum type with machinery for pattern matching.
	 *
	 * A sum type is essentially a tagged union. It encompasses the idea of a
	 * value that is either of type `T1`, or the type `T2`, or, ..., `TN` (for the
	 * sum type `<T1,T2,...,TN>`), but it may never have a value of more than one
	 * of its sub types at any given time.
	 *
	 * In FTL, this is enforced as much as is possible at compile time. Since the
	 * actual type of a value cannot be known until runtime, not everything can
	 * be checked statically. However, as long as no method with `unsafe` in its
	 * name is used, it should not be possible to violate any invariants (for
	 * example, to access a value as if it were a type it is not).
	 *
	 * Further, sum types in FTL are guaranteed not to have an invalid/partially
	 * constructed state. The \em only states it can take on, are as valid values
	 * of its component types. Note that this may include, eg, "moved from"
	 * states.
	 *
	 * \par "Pattern Matching"
	 *
	 * Sum types in FTL support a very basic version of pattern matching, in the
	 * form of essentially a simple visitor pattern. This looks a little like
	 * this:
	 *
	 * \code
	 *   auto x = sum_type<A,B>{type<A>};
	 *   x.match(
	 *     [](A a) { do_stuff_with_a(a); },
	 *     [](B b) { do_stuff_with_b(b); },
	 *   );
	 * \endcode
	 *
	 * These match calls have some additional static checks to enforce the
	 * invariants guaranteed by sum types. For example, the matches must be
	 * exhaustive: every component type must have a matching "visitor" function.
	 * This can be either by value, reference, or const reference.
	 *
	 * One exception is that `match` also allows a visitor with an argument of
	 * type `match_all`. This function will act as a catch-all for any types that
	 * have not been matched earlier in the list.
	 *
	 * `match` is a `constexpr` operation if all the given matching functions are.
	 *
	 * \par Type Traits & Concepts
	 *
	 * - A `sum_type<T1..TN>` is a trivial type, iff all of `T1..TN` are
	 *   `TriviallyCopyable`
	 * - A `sum_type<T1..TN>` is literal, iff all of `T1..TN` are
	 * - A `sum_type<T1..TN>` is standard layout, iff all of `T1..TN` are
	 * - A `sum_type<T1..TN>` is \ref copycons, iff all of `T1..TN` are
	 *   - Additionally, it is `noexcept` iff all of `T1..TN` are
	 * - A `sum_type<T1..TN>` is \ref movecons, iff all of `T1..TN` are
	 *   - Additionally, it is `noexcept` iff all of `T1..TN` are
	 * - A `sum_type<T1..TN>` is \ref copyassignable, iff all of `T1..TN` are
	 *   \em noexcept copy assignable. Assignment, if available, is always
	 *   noexcept.
	 * - A `sum_type<T1..TN>` is \ref moveassignable, iff all of `T1..TN` are
	 *   \em noexcept move assignable. Assignment, if available, is always
	 *   noexcept.
	 * - For any `T` in `T1..TN` that is noexcept \ref copyassignable,
	 *   `sum_type<T1..TN>` is noexcept copy assignable to values of that type.
	 * - For any `T` in `T1..TN` that is noexcept \ref moveassignable,
	 *   `sum_type<T1..TN>` is noexcept move assignable to values of that type.
	 * - A `sum_type<T1..TN>` is \ref eq, iff all of `T1..TN` are
	 *
	 * Note that a `sum_type` is never default constructible, since this would
	 * by necessity entail some kind of empty or invalid state.
	 *
	 * \see either, maybe
	 *
	 * \ingroup sum_type
	 */
	template<class...Ts>
	using sum_type =
		::ftl::dtl_::sum_type_<::ftl::dtl_::get_layout<Ts...>(), Ts...>;
}

#endif
