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
#include "prelude.h"
#include "concepts/monoid.h"
#include "concepts/monad.h"
#include "concepts/foldable.h"

namespace ftl {

	/**
	 * \defgroup maybe Maybe
	 *
	 * The maybe data type and associated operations.
	 *
	 * \code
	 *   #include <ftl/maybe.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - <stdexcept>
	 * - <type_traits>
	 * - \ref prelude
	 * - \ref monoid
	 * - \ref monad
	 * - \ref foldable
	 */

	/**
	 * Type that can be used to compare any maybe to `nothing`.
	 *
	 * \ingroup maybe
	 */
	struct nothing_t {
		constexpr nothing_t() noexcept {}
		constexpr nothing_t(const nothing_t&) noexcept {}
		constexpr nothing_t(nothing_t&&) noexcept {}
	};

	/**
	 * Convenience instance value of `nothing_t`.
	 *
	 * Useful when one wants to be more explicit than just using the bool cast
	 * of `maybe`.
	 *
	 * Example:
	 * \code
	 *   ftl::maybe<T> m = ...;
	 *   if(m != ftl::nothing)
	 *       do_something();
	 * \endcode
	 *
	 * \ingroup maybe
	 */
	constexpr nothing_t nothing{};

	/**
	 * Abstracts the concept of optional arguments and similar.
	 *
	 * \tparam A Must be a completely "plain" type, as in, it must not be
	 *           a reference, nor have `const`, `volatile`, or similar
	 *           qualifiers. It _may_ be of pointer type (incl. function
	 *           and method pointer).
	 *
	 * In essence, an instance of maybe is either a value, or nothing.
	 * 
	 * \par Concepts
	 * Maybe is an instance of the following concepts:
	 * \li \ref fullycons
	 * \li \ref assignable
	 * \li \ref deref
	 * \li \ref empty (the empty state is, of course, when the maybe is
	 *                `nothing`)
	 * \li \ref eq, if, and only if, `A` is EqComparable
	 * \li \ref orderable, if, and only if, `T` is Orderable
	 * \li \ref functor (in `T`)
	 * \li \ref applicative (in `T`)
	 * \li \ref monad (in `T`)
	 * \li \ref monoid, if, and only if, `T` is a Monoid
	 * \li \ref foldable
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	class maybe {
	public:
		/**
		 * Compatibility typedef.
		 *
		 * Allows compatibility with plethora of templated functions/structures
		 * that require an object have a value_type member.
		 */
		using value_type = T;

		/**
		 * Default c-tor, equivalent to \c nothing.
		 *
		 * Memory for the contained type is reserved on the stack, but no
		 * initialisation is done. In other words, A's constructor is \em not
		 * called.
		 */
		constexpr maybe() noexcept {}

		/// Copy c-tor
		maybe(const maybe& m)
		noexcept(std::is_nothrow_copy_constructible<T>::value)
	    : isValid(m.isValid) {
			if(isValid) {
				new (&val) value_type(reinterpret_cast<const T&>(m.val));
			}
		}

		/// Move c-tor
		maybe(maybe&& m)
		noexcept(std::is_nothrow_move_constructible<T>::value)
		: isValid(m.isValid) {
			if(isValid) {
				new (&val) value_type(std::move(reinterpret_cast<T&>(m.val)));
				m.isValid = false;
			}
		}

		/**
		 * Construct a value by copy.
		 */
		explicit maybe(const value_type& v)
		noexcept(std::is_nothrow_copy_constructible<T>::value)
		: isValid(true) {
			 new (&val) value_type(v);
		}

		/// Construct a value by move.
		explicit maybe(value_type&& v)
		noexcept(std::is_nothrow_move_constructible<T>::value)
		: isValid(true) {
			new (&val) value_type(std::move(v));
		}

		/**
		 * In-place value construction constructor.
		 *
		 * Constructs a value in the internal storage, forwarding the parameters
		 * to `T`'s constructor.
		 */
		template<typename...Ts>
		maybe(inplace_tag, Ts&&...ts) noexcept(std::is_nothrow_constructible<T,Ts...>::value)
		: isValid(true) {
			new (&val) value_type(std::forward<Ts>(ts)...);
		}

		// TODO: Enable the noexcept specifier once is_nothrow_destructible is
		// available (gcc-4.8).
		~maybe() /*noexcept(std::is_nothrow_destructible<T>::value)*/ {
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

		/// Copy assignment
		maybe& operator= (const maybe& m)
		/* TODO: Enable noexcept specifier once is_nothrow_destructible is
		 * available.
		noexcept(  std::is_nothrow_copy_constructible<T>::value
				&& std::is_nothrow_destructible<T>::value) */ {
			// Check for self-assignment
			if(this == &m)
				return *this;

			self_destruct();

			isValid = m.isValid;
			if(isValid) {
				new (&val) value_type(reinterpret_cast<const T&>(m.val));
			}

			return *this;
		}

		/// Move assignment
		maybe& operator= (maybe&& m)
		/* TODO: Enable noexcept specifier once is_nothrow_destructible is
		 * available.
		noexcept(  std::is_nothrow_copy_constructible<T>::value
				&& std::is_nothrow_destructible<T>::value) */ {
			// Check for self-assignment
			if(this == &m)
				return *this;

			self_destruct();

			isValid = m.isValid;
			if(isValid) {
				new (&val) value_type(
						std::move(reinterpret_cast<T&>(m.val)));
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
		explicit constexpr operator bool() const noexcept {
			return isValue();
		}

		/**
		 * Dereference operator.
		 * 
		 * \throws std::logic_error if `this` is `nothing`.
		 */
		value_type& operator* () {
			if(!isValid)
				throw std::logic_error("Attempting to read the value of Nothing.");

			return reinterpret_cast<T&>(val);
		}

		/// \overload
		const value_type& operator* () const {
			if(!isValid)
				throw std::logic_error("Attempting to read the value of Nothing.");

			return reinterpret_cast<const T&>(val);
		}

		/**
		 * Member access operator.
		 * 
		 * \throws std::logic_error if `this` is `nothing`.
		 */
		value_type* operator-> () {
			if(!isValid)
				throw std::logic_error("Attempting to read the value of Nothing.");

			return reinterpret_cast<T*>(&val);
		}

		/// \overload
		const value_type* operator-> () const {
			if(!isValid)
				throw std::logic_error("Attempting to read the value of Nothing.");

			return reinterpret_cast<const T*>(&val);
		}

		/**
		 * Static constructor of Nothing:s.
		 *
		 * The only purpose of this static method is to be more explicit than
		 * relying on maybe's default constructor. The end result is exactly
		 * equivalent.
		 */
		static constexpr maybe<T> nothing() noexcept {
			return maybe();
		}

	private:
		void self_destruct() {
			if(isValid) {
				reinterpret_cast<T&>(val).~T();
				isValid = false;
			}
		}

		typename std::aligned_storage<
			sizeof(T),
			std::alignment_of<T>::value>::type val;

		bool isValid = false;
	};

	/**
	 * Convenience function to create maybe:s.
	 *
	 * Creates a maybe, the exact type of which is automatically inferred from
	 * the parameter type.
	 *
	 * \ingroup maybe
	 */
	template<typename A>
	constexpr maybe<plain_type<A>> value(A&& a)
	noexcept(std::is_nothrow_constructible<plain_type<A>,A>::value) {
		return maybe<plain_type<A>>(std::forward<A>(a));
	}

	/**
	 * Equality comparison for maybe.
	 *
	 * \note Instantiating this operator for `A`s that have no equality operator
	 *       of their own will result in compilation error.
	 *
	 * \ingroup maybe
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
	 * \overload
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	bool operator== (const maybe<T>& m, nothing_t) noexcept {
		return !static_cast<bool>(m);
	}

	/**
	 * \overload
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	bool operator== (nothing_t, const maybe<T>& m) noexcept {
		return !static_cast<bool>(m);
	}

	/**
	 * Not equal to operator for maybe.
	 *
	 * \note Instantiating this operator for `A`s that have no `operator!=`
	 *       of their own will result in compilation error.
	 *
	 * \ingroup maybe
	 */
	template<typename A>
	bool operator!= (const maybe<A>& m1, const maybe<A>& m2) {
		return !(m1 == m2);
	}

	/**
	 * \overload
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	bool operator!= (const maybe<T>& m, nothing_t) noexcept {
		return static_cast<bool>(m);
	}

	/**
	 * \overload
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	bool operator!= (nothing_t, const maybe<T>& m) noexcept {
		return static_cast<bool>(m);
	}

	/**
	 * Less than operator.
	 *
	 * \note Will result in compilation error if `A` is not Orderable.
	 *
	 * \ingroup maybe
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
	 * Greater than operator.
	 *
	 * \note Will result in compilation error if `A` is not Orderable.
	 *
	 * \ingroup maybe
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
	 * `value_type` of the maybe and all nothings are ignored (unless
	 * everything is nothing).
	 *
	 * \tparam A must be a \ref monoid
	 *
	 * \ingroup maybe
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
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	struct monad<maybe<T>>
	: deriving_apply<maybe<T>>, deriving_join<maybe<T>> {

		static constexpr maybe<T> pure(T&& t)
		noexcept(std::is_nothrow_copy_constructible<T>::value) {
			return value(std::forward<T>(t));
		}

		/**
		 * Apply `f` if `m` is a value.
		 */
		template<typename F, typename U = result_of<F(T)>>
		static maybe<U> map(F&& f, const maybe<T>& m) {
			return m ? value(std::forward<F>(f)(*m)) : maybe<U>();
		}
		
		/**
		 * \overload
		 */
		template<typename F, typename U = result_of<F(T)>>
		static maybe<U> map(F&& f, maybe<T>&& m) {
			return m ? value(std::forward<F>(f)(std::move(*m))) : maybe<U>();
		}

		/**
		 * Applies a function to unwrapped maybe value.
		 *
		 * \tparam F must satisfy \ref fn`<maybe<U>(T)>`
		 */
		template<
			typename F,
			typename U = typename result_of<F(T)>::value_type>
		static maybe<U> bind(const maybe<T>& m, F&& f) {
			return m ? std::forward<F>(f)(*m) : maybe<U>();
		}

		/// \overload
		template<
			typename F,
			typename U = typename result_of<F(T)>::value_type>
		static maybe<U> bind(maybe<T>&& m, F&& f) {
			return m ? std::forward<F>(f)(std::move(*m)) : maybe<U>();
		}

		static constexpr bool instance = true;
	};

	/**
	 * Implementation of ftl::monoidA concept.
	 *
	 * Semantics are simple:
	 * \code
	 *   value(x)            | maybeValue = value(x)
	 *   maybe<T>::nothing() | maybeValue = maybeValue
	 * \endcode
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	struct monoidA<maybe<T>> {
		static constexpr maybe<T> fail() noexcept {
			return maybe<T>{};
		}

		static maybe<T> orDo(const maybe<T>& m1, const maybe<T>& m2) {
			return m1 ? m1 : m2;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Foldable instance for maybe
	 *
	 * Folds as if maybe was a container like any other. Naturally, a particular
	 * maybe instance will only ever contain zero or one elements ("nothing" or
	 * "value").
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	struct foldable<maybe<T>>
	: deriving_foldMap<maybe<T>>, deriving_fold<maybe<T>> {

		template<
				typename Fn,
				typename U,
				typename = typename std::enable_if<
					std::is_same<U, result_of<Fn(U,T)>>::value
				>::type
		>
		static U foldl(Fn&& fn, U&& z, const maybe<T>& m) {
			if(m) {
				return fn(std::forward<U>(z), *m);
			}

			return z;
		}

		template<
				typename Fn,
				typename U,
				typename = typename std::enable_if<
					std::is_same<U, result_of<Fn(T,U)>>::value
				>::type
		>
		static U foldr(Fn&& fn, U&& z, const maybe<T>& m) {
			if(m) {
				return fn(std::forward<U>(z), *m);
			}

			return z;
		}

		static constexpr bool instance = true;
	};
}

#endif

