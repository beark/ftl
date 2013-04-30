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
#ifndef FTL_MAYBE_H
#define FTL_MAYBE_H

#include <stdexcept>
#include <type_traits>
#include "monoid.h"
#include "monad.h"

namespace ftl {

	/// Used to distinguish in-place constructors from others
	struct inplace_tag {};

	/**
	 * Abstracts the concept of optional arguments and similar.
	 *
	 * In essence, an instance of maybe is either a value, or nothing.
	 * 
	 * \par Concepts
	 * Maybe is an instance of the following concepts:
	 * \li DefaultConstructible
	 * \li CopyConstructible
	 * \li MoveConstructible
	 * \li Assignable
	 * \li Dereferencable
	 * \li EqComparable, iff A is EqComparable
	 * \li Orderable, iff A is Orderable
	 * \li Functor (in A)
	 * \li Monad (in A)
	 * \li Monoid, iff A is Monoid
	 */
	template<typename A>
	class maybe {
	public:
		/**
		 * Compatibility typedef.
		 *
		 * Allows compatibility with plethora of templated functions/structures
		 * that require an object have a value_type member.
		 */
		using value_type = A;

		/**
		 * Default c-tor, equivalent to \c nothing.
		 *
		 * Memory for the contained type is reserved on the stack, but no
		 * initialisation is done. In other words, A's constructor is \em not
		 * called.
		 */
		constexpr maybe() noexcept {}

		maybe(const maybe& m)
		noexcept(std::is_nothrow_copy_constructible<A>::value)
	    : isValid(m.isValid) {
			if(isValid) {
				new (&val) value_type(reinterpret_cast<const A&>(m.val));
			}
		}

		maybe(maybe&& m)
		noexcept(std::is_nothrow_move_constructible<A>::value)
		: isValid(m.isValid) {
			if(isValid) {
				new (&val) value_type(std::move(reinterpret_cast<A&>(m.val)));
				m.isValid = false;
			}
		}

		explicit maybe(const value_type& v)
		noexcept(std::is_nothrow_copy_constructible<A>::value)
		: isValid(true) {
			 new (&val) value_type(v);
		}

		explicit maybe(const value_type&& v)
		noexcept(std::is_nothrow_move_constructible<A>::value)
		: isValid(true) {
			new (&val) value_type(std::move(v));
		}

		/**
		 * In place construction constructor.
		 *
		 * Calls \c value_types's constructor with the given arguments.
		 */
		template<typename...Ts>
		maybe(inplace_tag, Ts...ts) noexcept(std::is_nothrow_constructible<A,Ts...>::value)
		: isValid(true) {
			new (&val) value_type(std::forward<Ts>(ts)...);
		}

		// TODO: Enable the noexcept specifier once is_nothrow_destructible is
		// available (gcc-4.8).
		~maybe() /*noexcept(std::is_nothrow_destructible<A>::value)*/ {
			self_destruct();
		}

		/**
		 * Check if the maybe is nothing.
		 */
		constexpr bool isNothing() const noexcept {
			return !isValid;
		}

		/**
		 * Check if the maybe is a value.
		 */
		constexpr bool isValue() const noexcept {
			return isValid;
		}

		const maybe& operator= (const maybe& m)
		/* TODO: Enable noexcept specifier once is_nothrow_destructible is
		 * available.
		noexcept(  std::is_nothrow_copy_constructible<A>::value
				&& std::is_nothrow_destructible<A>::value) */ {
			// Check for self-assignment
			if(this == &m)
				return *this;

			self_destruct();

			isValid = m.isValid;
			if(isValid) {
				new (&val) value_type(m.val);
			}

			return *this;
		}

		const maybe& operator= (maybe&& m)
		/* TODO: Enable noexcept specifier once is_nothrow_destructible is
		 * available.
		noexcept(  std::is_nothrow_copy_constructible<A>::value
				&& std::is_nothrow_destructible<A>::value) */ {
			// Check for self-assignment
			if(this == &m)
				return *this;

			self_destruct();

			isValid = m.isValid;
			if(isValid) {
				new (&val) value_type(std::move(m.val));
				m.isValid = false;
			}

			return *this;
		}

		/**
		 * Bool conversion operator.
		 *
		 * Provided for convenience, to allow syntax such as
		 * \code
		 *   maybe<T> m = ...;
		 *   if(m) {
		 *       doStuff(m);
		 *   }
		 * \endcode
		 */
		constexpr operator bool() const noexcept {
			return isValue();
		}

		/**
		 * Dereference operator.
		 * 
		 * \note Throws an \c std::logic_error if \c this is \c nothing.
		 */
		value_type& operator* () {
			if(!isValid)
				throw std::logic_error("Attempting to read the value of Nothing.");

			return reinterpret_cast<A&>(val);
		}

		/**
		 * \overload
		 */
		const value_type& operator* () const {
			if(!isValid)
				throw std::logic_error("Attempting to read the value of Nothing.");

			return reinterpret_cast<const A&>(val);
		}

		/**
		 * Member access operator.
		 * 
		 * \note Throws an \c std::logic_error if \c this is \c nothing.
		 */
		value_type* operator-> () {
			if(!isValid)
				throw std::logic_error("Attempting to read the value of Nothing.");

			return reinterpret_cast<const A*>(&val);
		}

		/// \overload
		const value_type* operator-> () const {
			if(!isValid)
				throw std::logic_error("Attempting to read the value of Nothing.");

			return reinterpret_cast<const A*>(&val);
		}

		/**
		 * Static constructor of Nothing:s.
		 */
		static constexpr maybe<A> nothing() noexcept {
			return maybe();
		}

	private:
		void self_destruct() {
			if(isValid) {
				reinterpret_cast<A&>(val).~A();
				isValid = false;
			}
		}

		typename std::aligned_storage<
			sizeof(A),
			std::alignment_of<A>::value>::type val;

		bool isValid = false;
	};

	/**
	 * Convenience function to create maybe:s.
	 */
	template<typename A>
	constexpr maybe<A> value(const A& a)
	noexcept(std::is_nothrow_copy_constructible<A>::value) {
		return maybe<A>(a);
	}

	/**
	 * \overload
	 */
	template<typename A>
	constexpr maybe<A> value(A&& a)
	noexcept(std::is_nothrow_move_constructible<A>::value) {
		return maybe<A>(std::move(a));
	}

	/**
	 * EqComparable::operator== implementation for maybe.
	 *
	 * Will result in compilation error if A is not EqComparable.
	 */
	template<typename A>
	bool operator== (const maybe<A>& m1, const maybe<A>& m2) {
		if(m1 && m2) {
			return *m1 == *m2;
		}
		else if(!m1 && !m2) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * EqComparable::operator!= implementation for maybe.
	 *
	 * Will result in compilation error if A is not EqComparable.
	 */
	template<typename A>
	bool operator!= (const maybe<A>& m1, const maybe<A>& m2) {
		return !(m1 == m2);
	}

	/**
	 * Orderable::operator< implementation for maybe.
	 *
	 * Will result in compilation error if A is not also Orderable.
	 */
	template<typename A>
	bool operator< (const maybe<A>& m1, const maybe<A>& m2) {
		if(m1) {
			if(m2) {
				return *m1 < *m2;
			}
			else {
				return false;
			}
		}
		else if(m2) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * Orderable::operator> implementation for maybe.
	 */
	template<typename A>
	bool operator> (const maybe<A>& m1, const maybe<A>& m2) {
		if(m1) {
			if(m2) {
				return *m1 > *m2;
			}
			else {
				return true;
			}
		}
		else if(m2) {
			return false;
		}
		else {
			return false;
		}
	}

	/**
	 * Monoid implementation for maybe.
	 *
	 * Semantics are:
	 * \code
	 *   id() <=> maybe::nothing() <=> maybe()
	 *   append(value(x), value(y)) <=> value(append(x, y))
	 *   append(value(x), maybe::nothing()) <=> value(x)
	 *   append(maybe::nothing, value(y)) <=> value(y)
	 *   append(maybe::nothing, maybe::nothing) <=> maybe::nothing
	 * \endcode
	 *
	 * In other words, the append operation is simply lifted into the
	 * \c value_type of the maybe and all nothings are ignored (unless
	 * everything is nothing).
	 */
	template<typename A>
	struct monoid<maybe<A>> {
		static constexpr maybe<A> id() {
			return maybe<A>();
		}

		static maybe<A> append(const maybe<A>& m1, const maybe<A>& m2) {
			if(m1) {
				if(m2) {
					return maybe<A>(monoid<A>::append(*m1, *m2));
				}

				else {
					return m1;
				}
			}
			else if(m2) {
				return m2;
			}

			else {
				return maybe<A>();
			}
		}

		static constexpr bool instance = monoid<A>::instance;
	};

	/**
	 * Implementation of monad for maybe.
	 *
	 * \note This automatically gives maybe default applicative and functor
	 *       instances.
	 */
	template<>
	struct monad<maybe> {
		template<typename A>
		static constexpr maybe<A> pure(const A& a)
		noexcept(std::is_nothrow_copy_constructible<A>::value) {
			return value(a);
		}

		template<typename A>
		static constexpr maybe<A> pure(A&& a)
		noexcept(std::is_nothrow_move_constructible<A>::value) {
			return value(std::move(a));
		}

		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type>
		static maybe<B> map(F f, maybe<A> m) {
			return m ? value(f(*m)) : maybe<B>();
		}
		

		/**
		 * Applies a function to unwrapped maybe value.
		 *
		 * \tparam F must satisfy Function<maybe<B>(A)>
		 */
		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type::value_type>
		static maybe<B> bind(const maybe<A>& m, F f) {
			return m ? f(*m) : maybe<B>();
		}

		static constexpr bool instance = true;
	};
}

#endif

