/*
* Copyright (c) 2016 Björn Aili
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
#ifndef FTL_RECURSIVE_UNION_H
#define FTL_RECURSIVE_UNION_H

#include <cassert>
#include <stdexcept>

namespace ftl
{
	/**
	* Type tag type used to select which constructor to use in a `sum_type`.
	*
	* \par Examples
	*
	* Copy-constructing a sub-type of a `sum_type`
	* \code
	*   sum_type<int,string> x{type<int>, 12};
	* \endcode
	*
	* Calling arbitrary constructor of a sub-type
	* \code
	*   // B must have a constructor taking an int and a c-string, or something
	*   // they implicitly convert to
	*   sum_type<A,B> x{type<B>, 12, "foo"};
	* \endcode
	*
	* \see sum_type
	*
	* \ingroup sum_type
	*/
	template<typename>
	struct type_t {};

	/**
	* Convenience instance of the type_t type tag.
	*
	* \see sum_type
	*
	* \ingroup sum_type
	*/
	template<typename T>
	constexpr type_t<T> type{};

	/**
	 * Exception type thrown when accessing a sum type erroneously.
	 *
	 * \see sum_type
	 *
	 * \ingroup sum_type
	 */
	class invalid_sum_type_access : public std::logic_error
	{
	public:
		invalid_sum_type_access()
		: std::logic_error("Access to sum type at invalid type index") {}

		using std::logic_error::logic_error;
	};

	namespace dtl_
	{
		// Hierarchy of type layouts. Too much maintenance to take into account the
		// entire matrix of destructor and constructor properties.
		// Each layout level must satisfy the constraints of the one below, too.
		enum class type_layout
		{
			trivially_copyable,	// Trivially copy/move constructible/assignable
			trivial_destructor,	// Trivially destructible
			complex							// No constraints
		};

		template<class...Ts>
		constexpr type_layout get_layout()
		{
			if (!All<::std::is_trivially_destructible, Ts...>::value)
			{
				return type_layout::complex;
			}
			else if (!All<::std::is_trivially_copyable, Ts...>::value)
			{
				return type_layout::trivial_destructor;
			}
			else
			{
				return type_layout::trivially_copyable;
			}
		}

		template<type_layout, class...Ts>
		struct recursive_union_;

		template<>
		struct recursive_union_<type_layout::trivially_copyable>
		{
			constexpr bool compare(size_t, const recursive_union_&) const noexcept
			{
				return false;
			}
		};

		template<>
		struct recursive_union_<type_layout::trivial_destructor>
		{
			constexpr recursive_union_(const recursive_union_&, size_t) noexcept {}
			constexpr recursive_union_(recursive_union_&&, size_t) noexcept {}

			void copy(const recursive_union_&, size_t)
			{
				throw invalid_sum_type_access();
			}

			void copy(recursive_union_&&, size_t)
			{
				throw invalid_sum_type_access();
			}

			void move(recursive_union_&&, size_t)
			{
				throw invalid_sum_type_access();
			}

			template<class T, class U>
			void assign(type_t<T>, U&&)
			{
				throw invalid_sum_type_access();
			}

			constexpr bool compare(size_t, const recursive_union_&) const noexcept
			{
				return false;
			}
		};

		template<>
		struct recursive_union_<type_layout::complex>
		{
			constexpr recursive_union_(const recursive_union_&, size_t) noexcept {}
			constexpr recursive_union_(recursive_union_&&, size_t) noexcept {}

			void destroy(size_t)
			{
				throw invalid_sum_type_access();
			}

			void copy(const recursive_union_&, size_t)
			{
				throw invalid_sum_type_access();
			}

			void copy(recursive_union_&&, size_t)
			{
				throw invalid_sum_type_access();
			}

			void move(recursive_union_&&, size_t)
			{
				throw invalid_sum_type_access();
			}

			template<class T, class U>
			void assign(type_t<T>, U&&)
			{
				throw invalid_sum_type_access();
			}

			constexpr bool compare(size_t, const recursive_union_&) const noexcept
			{
				return false;
			}

			template<class U>
			constexpr U get(type_t<U>)
			{
				throw invalid_sum_type_access();
			}
		};

		template<typename T, typename...Ts>
		struct recursive_union_<type_layout::trivially_copyable, T, Ts...>
		{
			recursive_union_() = delete;
			recursive_union_(const recursive_union_&) = default;
			recursive_union_(recursive_union_&&) = default;

			template<typename...Args>
			constexpr recursive_union_(type_t<T>, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,Args...>::value)
			: val(std::forward<Args>(args)...)
			{}

			template<typename U, typename...Args>
			constexpr recursive_union_(type_t<U> s, Args&&...args)
			noexcept
			(
				::std::is_nothrow_constructible<
					recursive_union_<
						type_layout::trivially_copyable,Ts...>,Args...>::value
			)
			: rem(s, std::forward<Args>(args)...)
			{}

			~recursive_union_() = default;

			recursive_union_& operator= (const recursive_union_&) = default;
			recursive_union_& operator= (recursive_union_&&) = default;

			template<class U>
			constexpr const U& get(type_t<U> u) const &
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				return rem.get(u);
			}

			constexpr const T& get(type_t<T>) const &
			{
				return val;
			}

			template<class U>
			constexpr U& get(type_t<U> u) &
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				return rem.get(u);
			}

			constexpr T& get(type_t<T>) &
			{
				return val;
			}

			template<class U>
			constexpr U&& get(type_t<U> u) &&
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				return ::std::move(rem).get(u);
			}

			constexpr T&& get(type_t<T>) &&
			{
				return ::std::move(val);
			}

			constexpr bool compare(size_t I, const recursive_union_& rhs) const noexcept
			{
				assert(I <= sizeof...(Ts) && "Type index out of range; illegal sum_type state");
				return I == 0 ? val == rhs.val : rem.compare(I - 1, rhs.rem);
			}

			union
			{
				T val;
				recursive_union_<type_layout::trivially_copyable,Ts...> rem;
			};
		};

		template<typename T, typename...Ts>
		struct recursive_union_<type_layout::trivial_destructor, T, Ts...>
		{
			recursive_union_() = delete;
			recursive_union_(const recursive_union_&) = delete;
			recursive_union_(recursive_union_&&) = delete;

			template<typename...Args>
			constexpr recursive_union_(type_t<T>, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,Args...>::value)
			: val(std::forward<Args>(args)...)
			{}

			template<typename U, typename...Args>
			constexpr recursive_union_(type_t<U> s, Args&&...args)
			noexcept
			(
				::std::is_nothrow_constructible<
					recursive_union_<
						type_layout::trivially_copyable,Ts...>,Args...>::value
			)
			: rem(s, std::forward<Args>(args)...)
			{}

			recursive_union_(const recursive_union_& other, size_t i)
			{
				if (i == 0)
				{
					new (&val) T(other.val);
				}
				else
				{
					assert(sizeof...(Ts) > 0 && "Invalid sum type access");
					new (&rem) recursive_union_<type_layout::trivial_destructor,Ts...>{other.rem, i - 1};
				}
			}

			recursive_union_(recursive_union_&& other, size_t i)
			{
				if (i == 0)
				{
					new (&val) T(::std::move(other.val));
				}
				else
				{
					assert(sizeof...(Ts) > 0 && "Invalid sum type access");
					new (&rem) recursive_union_<type_layout::trivial_destructor,Ts...>(::std::move(other.rem), i - 1);
				}
			}

			~recursive_union_() = default;

			void copy(const recursive_union_& u, size_t i)
			{
				if (i == 0)
				{
					val = u.val;
				}
				else
				{
					assert(sizeof...(Ts) > 0 && "Invalid sum type access");
					rem.copy(u.rem, i - 1);
				}
			}

			void move(recursive_union_&& u, size_t i)
			{
				if (i == 0)
				{
					val = ::std::move(u.val);
				}
				else
				{
					assert(sizeof...(Ts) > 0 && "Invalid sum type access");
					rem.move(::std::move(u.rem), i - 1);
				}
			}

			template<class U, class V>
			void assign(type_t<U>, V&& v)
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				rem.assign(type<U>, ::std::forward<V>(v));
			}

			void assign(type_t<T>, const T& v)
			{
				val = v;
			}

			void assign(type_t<T>, T&& v)
			{
				val = ::std::move(v);
			}

			template<class U>
			constexpr const U& get(type_t<U> u) const &
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				return rem.get(u);
			}

			constexpr const T& get(type_t<T>) const &
			{
				return val;
			}

			template<class U>
			constexpr U& get(type_t<U> u) &
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				return rem.get(u);
			}

			constexpr T& get(type_t<T>) &
			{
				return val;
			}

			template<class U>
			constexpr U&& get(type_t<U> u) &&
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				return ::std::move(rem).get(u);
			}

			constexpr T&& get(type_t<T>) &&
			{
				return ::std::move(val);
			}

			constexpr bool compare(size_t I, const recursive_union_& rhs) const noexcept
			{
				assert(I <= sizeof...(Ts) && "Type index out of range; illegal sum_type state");
				return I == 0 ? val == rhs.val : rem.compare(I - 1, rhs.rem);
			}

			union
			{
				T val;
				recursive_union_<type_layout::trivial_destructor,Ts...> rem;
			};
		};

		template<typename T, typename...Ts>
		struct recursive_union_<type_layout::complex, T, Ts...>
		{
			constexpr recursive_union_() {}
			recursive_union_(const recursive_union_&) = delete;
			recursive_union_(recursive_union_&&) = delete;

			template<typename...Args>
			constexpr recursive_union_(type_t<T>, Args&&...args)
			noexcept(::std::is_nothrow_constructible<T,Args...>::value)
			: val(std::forward<Args>(args)...)
			{}

			template<typename U, typename...Args>
			constexpr recursive_union_(type_t<U> s, Args&&...args)
			noexcept
			(
				::std::is_nothrow_constructible<
					recursive_union_<type_layout::complex,Ts...>,Args...>::value
			)
			: rem(s, std::forward<Args>(args)...)
			{}

			recursive_union_(const recursive_union_& other, size_t i)
			{
				if (i == 0)
				{
					new (&val) T(other.val);
				}
				else
				{
					assert(sizeof...(Ts) > 0 && "Invalid sum type access");
					new (&rem) recursive_union_<type_layout::complex,Ts...>(other.rem, i - 1);
				}
			}

			recursive_union_(recursive_union_&& other, size_t i)
			{
				if (i == 0)
				{
					new (&val) T(::std::move(other.val));
				}
				else
				{
					assert(sizeof...(Ts) > 0 && "Invalid sum type access");
					new (&rem) recursive_union_<type_layout::complex,Ts...>(::std::move(other.rem), i - 1);
				}
			}

			~recursive_union_() {}

			void copy(const recursive_union_& u, size_t i)
			{
				if (i == 0)
				{
					val = u.val;
				}
				else
				{
					assert(sizeof...(Ts) > 0 && "Invalid sum type access");
					rem.copy(u.rem, i - 1);
				}
			}

			void move(recursive_union_&& u, size_t i)
			{
				if (i == 0)
				{
					val = ::std::move(u.val);
				}
				else
				{
					assert(sizeof...(Ts) > 0 && "Invalid sum type access");
					rem.move(::std::move(u.rem), i - 1);
				}
			}

			template<class U, class V>
			void assign(type_t<U>, V&& v)
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				rem.assign(type<U>, ::std::forward<V>(v));
			}

			void assign(type_t<T>, const T& v)
			{
				val = v;
			}

			void assign(type_t<T>, T&& v)
			{
				val = ::std::move(v);
			}

			void destroy(size_t i)
			{
				if (i == 0)
				{
					val.~T();
				}
				else
				{
					assert(sizeof...(Ts) > 0 && "Invalid sum type access");
					rem.destroy(i - 1);
				}
			}

			template<class U>
			constexpr const U& get(type_t<U> u) const &
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				return rem.get(u);
			}

			constexpr const T& get(type_t<T>) const &
			{
				return val;
			}

			template<class U>
			constexpr U& get(type_t<U> u) &
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				return rem.get(u);
			}

			constexpr T& get(type_t<T>) &
			{
				return val;
			}

			template<class U>
			constexpr U&& get(type_t<U> u) &&
			{
				assert(sizeof...(Ts) > 0 && "Invalid sum type access");
				return ::std::move(rem).get(u);
			}

			constexpr T&& get(type_t<T>) &&
			{
				return ::std::move(val);
			}

			constexpr bool compare(size_t I, const recursive_union_& rhs) const noexcept
			{
				assert(I <= sizeof...(Ts) && "Type index out of range; illegal sum_type state");
				return I == 0 ? val == rhs.val : rem.compare(I - 1, rhs.rem);
			}

			union
			{
				T val;
				recursive_union_<type_layout::complex,Ts...> rem;
			};
		};
	}
}

#endif
