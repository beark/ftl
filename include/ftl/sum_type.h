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
	 *       [](otherwise){ return g(); }
	 *   );
	 * \endcode
	 *
	 * \note If present, this match clause should always be the final one. If
	 *       there are additional match clauses after `otherwise`, they will
	 *       never be called.
	 */
	struct otherwise
	{
		template<class T>
		constexpr otherwise(T&&) noexcept {}
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

		template<class T, class U>
		struct is_match
			: ::std::is_same<::ftl::plain_type<T>, ::ftl::plain_type<U>> {};

		template<class T>
		struct is_match<T,T> : ::std::true_type {};

		template<class T>
		struct is_match<T,::ftl::otherwise> : ::std::true_type {};

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
				type_is_in_v<plain_type<argument_type<F,0>>,Ts...>
				&& match_cases_are_in_type<type_seq<Fs...>,Ts...>::value;
		};

		template<bool AllTrivial, typename...Ts>
		struct sum_type_;

		template<bool, class...>
		struct match_selector;

		template<bool Trivial>
		struct match_selector<Trivial>
		{
			template<
				class T, class F, class...Fs,
				class = typename ::std::enable_if<
					is_match<::ftl::plain_type<T>,argument_type<F,0>>::value
				>::type
			>
			static constexpr auto invoke(T&& t, F&& f, Fs&&...)
			{
				return ::std::forward<F>(f)(::std::forward<T>(t));
			}

			template<
				class F, class T, class...Fs,
				class = typename ::std::enable_if<
					!is_match<::ftl::plain_type<T>,argument_type<F,0>>::value
				>::type
			>
			static constexpr auto invoke(T&& t, F&&, Fs&&...fs)
			{
				return match_selector::invoke(
					::std::forward<T>(t),
					::std::forward<Fs>(fs)...);
			}
		};

		template<bool Trivial, size_t I, size_t J, size_t...Is, class T, class...Ts>
		struct match_selector<Trivial,::std::index_sequence<I,J,Is...>,T,Ts...>
		{
			template<class...Fs> static constexpr auto
			invoke(const recursive_union_<Trivial,T,Ts...>& s, size_t i, Fs&&...fs)
			{
				return i == I
					? match_selector<Trivial>::invoke(s.val, ::std::forward<Fs>(fs)...)
					: match_selector<Trivial,::std::index_sequence<J,Is...>,Ts...>::invoke(
							s.rem, i, std::forward<Fs>(fs)...
						);
			}

			template<class...Fs> static constexpr auto
			invoke(recursive_union_<Trivial,T,Ts...>& s, size_t i, Fs&&...fs)
			{
				return i == I
					? match_selector<Trivial>::invoke(s.val, ::std::forward<Fs>(fs)...)
					: match_selector<Trivial,::std::index_sequence<J,Is...>,Ts...>::invoke(
							s.rem, i, std::forward<Fs>(fs)...
						);
			}

			template<class...Fs> static constexpr auto
			invoke(recursive_union_<Trivial,T,Ts...>&& s, size_t i, Fs&&...fs)
			{
				return i == I
					? match_selector<Trivial>::invoke(
							::std::move(s.val), ::std::forward<Fs>(fs)...
						)
					: match_selector<Trivial,::std::index_sequence<J,Is...>,Ts...>::invoke(
							::std::move(s.rem), i, std::forward<Fs>(fs)...
						);
			}
		};

		template<bool Trivial, size_t I, typename T, typename...Ts>
		struct match_selector<Trivial, ::std::index_sequence<I>,T,Ts...>
		{
			template<typename...Fs> static constexpr auto
			invoke(const recursive_union_<Trivial,T,Ts...>& s, size_t i, Fs&&...fs)
			{
				assert(i == I);

				return match_selector<Trivial>::invoke(
					s.val, ::std::forward<Fs>(fs)...
				);
			}

			template<typename...Fs> static constexpr auto
			invoke(recursive_union_<Trivial,T,Ts...>& s, size_t i, Fs&&...fs)
			{
				assert(i == I);

				return match_selector<Trivial>::invoke(
					s.val, ::std::forward<Fs>(fs)...
				);
			}

			template<typename...Fs> static constexpr auto
			invoke(recursive_union_<Trivial,T,Ts...>&& s, size_t i, Fs&&...fs)
			{
				assert(i == I);

				return match_selector<Trivial>::invoke(
					::std::move(s.val), ::std::forward<Fs>(fs)...
				);
			}
		};

		template<typename...Ts>
		struct sum_type_<true,Ts...>
		{
			static_assert(
				dtl_::is_type_set<Ts...>::value,
				"Each member type of a sum type must be unique"
			);

			static_assert(sizeof...(Ts) > 0, "Empty sum types are not allowed");

			sum_type_() = delete;
			sum_type_(const sum_type_&) = default;
			sum_type_(sum_type_&&) = default;

			template<typename T, typename...Args>
			explicit constexpr sum_type_(type_t<T> s, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,Args...>::value)
				: data(s, std::forward<Args>(args)...), cons(index_of_v<T,Ts...>)
			{}

			~sum_type_() = default;

			template<class T, class...Args>
			auto emplace(type_t<T> s, Args&&... args) noexcept
			-> ::std::enable_if_t<::std::is_nothrow_constructible<T,Args...>::value>
			{
				// Since we know we're dealing with trivial types, we can just
				// replace the storage, no need to destruct
				data = recursive_union<Ts...>(s, std::forward<Args>(args)...);
				cons = index_of_v<T,Ts...>;
			}

			template<class T>
			constexpr const T& unsafe_get() const &
			{
				return data.get(type<T>);
			}

			template<class T>
			constexpr T& unsafe_get() &
			{
				return data.get(type<T>);
			}

			template<class T>
			T&& unsafe_get() &&
			{
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
						type_seq<MatchArms...>,otherwise,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<true,::std::index_sequence_for<Ts...>,Ts...>::invoke(
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
						type_seq<MatchArms...>,otherwise,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<true,::std::index_sequence_for<Ts...>,Ts...>::invoke(
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
						type_seq<MatchArms...>,otherwise,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<true,::std::index_sequence_for<Ts...>,Ts...>::invoke(
						::std::move(data), cons, ::std::forward<MatchArms>(matchArms)...
					);
			}

			template<class T, class = ::std::enable_if_t<type_is_in_v<T,Ts...>>>
			const sum_type_& operator= (T&& t) noexcept
			{
				emplace(type<::std::decay_t<T>>, ::std::forward<T>(t));

				return *this;
			}

			sum_type_& operator= (const sum_type_& other) = default;
			sum_type_& operator= (sum_type_&& other) = default;

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
			recursive_union<Ts...> data;
			size_t cons;
		};

		template<class...Ts>
		struct sum_type_<false,Ts...>
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

			template<class T, class U = typename T::value_type>
			constexpr sum_type_(type_t<T> s, ::std::initializer_list<U> init)
			noexcept(::std::is_nothrow_constructible<T,::std::initializer_list<U>>::value)
				: data(s, init), cons(index_of_v<T,Ts...>)
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
				new (&data) recursive_union<Ts...> (s, ::std::forward<Args>(args)...);
				cons = index_of_v<T,Ts...>;
			}

			template<class T>
			constexpr const T& unsafe_get() const &
			{
				return data.get(type<T>);
			}

			template<class T>
			constexpr T& unsafe_get() &
			{
				return data.get(type<T>);
			}

			template<class T>
			T&& unsafe_get() &&
			{
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
						type_seq<MatchArms...>,otherwise,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<false,::std::index_sequence_for<Ts...>,Ts...>::invoke(
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
						type_seq<MatchArms...>,otherwise,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<false,::std::index_sequence_for<Ts...>,Ts...>::invoke(
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
						type_seq<MatchArms...>,otherwise,Ts...>::value,
					"Trying to match with a type that is not in the sum");

				return ::ftl::dtl_
					::match_selector<false,::std::index_sequence_for<Ts...>,Ts...>::invoke(
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
					new (&data) recursive_union<Ts...> (other.data, cons);
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
					new (&data) recursive_union<Ts...> (::std::move(other.data), cons);
				}

				return *this;
			}

			template<
				class T, class U = ::ftl::plain_type<T>,
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
					new (&data) recursive_union<Ts...> (type<U>, ::std::forward<T>(t));
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
			recursive_union<Ts...> data;
			size_t cons;
		};
	}

	template<class...Ts>
	using sum_type =
		::ftl::dtl_::sum_type_<All<::std::is_trivial,Ts...>::value, Ts...>;
}

#endif


