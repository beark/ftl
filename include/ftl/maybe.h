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
	 * Exception type signifying an attempt to access an empty maybe object.
	 *
	 * \ingroup maybe
	 */
	class invalid_maybe_access : public std::logic_error {
	public:
		explicit invalid_maybe_access(const std::string& what)
		: logic_error(what) {}

		explicit invalid_maybe_access(std::string&& what)
		: logic_error(std::move(what)) {}

		explicit invalid_maybe_access(const char* what)
		: logic_error(what) {}

	};

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

	template<typename T>
	class maybe;

	namespace _dtl {

		template<typename T>
		class base_maybe_it
		: public std::iterator<std::forward_iterator_tag,T> {

		protected:
			maybe<T>* ref = nullptr;

		public:
			base_maybe_it() = default;
			base_maybe_it(const base_maybe_it&) = default;
			base_maybe_it(base_maybe_it&&) = default;
			~base_maybe_it() = default;

			explicit constexpr base_maybe_it(maybe<T>* m) noexcept
			: ref(m && *m ? m : nullptr) {}

			base_maybe_it& operator++ () noexcept {
				ref = nullptr;
				return *this;
			}

			base_maybe_it operator++ (int) noexcept {
				auto it = *this;
				ref = nullptr;
				return it;
			}

			constexpr bool operator== (const base_maybe_it& it) const noexcept {
				return ref == it.ref;
			}

			constexpr bool operator!= (const base_maybe_it& it) const noexcept {
				return ref != it.ref;
			}

			base_maybe_it& operator= (const base_maybe_it&) = default;
			base_maybe_it& operator= (base_maybe_it&&) = default;
		};

		template<typename T>
		class maybe_iterator : public base_maybe_it<T> {
		public:
			maybe_iterator() = default;
			maybe_iterator(const maybe_iterator&) = default;
			maybe_iterator(maybe_iterator&&) = default;
			~maybe_iterator() = default;

			explicit constexpr maybe_iterator(maybe<T>* m) noexcept
			: base_maybe_it<T>{m} {}

			constexpr T& operator* () const {
				return this->ref->operator*();
			}

			constexpr T* operator-> () const {
				return this->ref->operator->();
			}

			maybe_iterator& operator= (const maybe_iterator&) = default;
			maybe_iterator& operator= (maybe_iterator&&) = default;
		};

		template<typename T>
		class const_maybe_iterator : public base_maybe_it<T> {
		public:
			const_maybe_iterator() = default;
			const_maybe_iterator(const const_maybe_iterator&) = default;
			const_maybe_iterator(const_maybe_iterator&&) = default;
			~const_maybe_iterator() = default;

			explicit constexpr const_maybe_iterator(maybe<T>* m) noexcept
			: base_maybe_it<T>{m} {}

			constexpr const T& operator* () const noexcept {
				return this->ref->operator*();
			}

			constexpr const T* operator-> () const noexcept {
				return this->ref->operator->();
			}

			const_maybe_iterator& operator= (const const_maybe_iterator&)
			   	= default;
			const_maybe_iterator& operator= (const_maybe_iterator&&) = default;
		};
	}

	/**
	 * Abstracts the concept of optional arguments and similar.
	 *
	 * \tparam T Must be a completely "plain" type, as in, it must not be
	 *           a reference, nor have `const`, `volatile`, or similar
	 *           qualifiers. It _may_ be of pointer type (incl. function
	 *           and method pointer).
	 *
	 * In essence, an instance of maybe is either a value, or nothing.
	 * 
	 * \par Concepts
	 * Maybe is an instance of the following concepts:
	 * - \ref fullycons
	 * - \ref assignable, if `T` is
	 * - \ref deref
	 * - \ref empty (the empty state is, of course, when the maybe is
	 *                `nothing`)
	 * - \ref eq, if, and only if, `A` is EqComparable
	 * - \ref orderable, if, and only if, `T` is Orderable
	 * - \ref functor (in `T`)
	 * - \ref applicative (in `T`)
	 * - \ref monad (in `T`)
	 * - \ref monoid, if, and only if, `T` is a Monoid
	 * - \ref foldable, as if it were a container of zero or one element
	 * - \ref fwditerable, as above
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	class maybe {
	public:
		/**
		 * Necessary to fulfill the ForwardIterable concept.
		 */
		using value_type = T;

		using iterator = _dtl::maybe_iterator<T>;
		using const_iterator = _dtl::const_maybe_iterator<T>;

		/**
		 * Default c-tor, equivalent to `nothing`.
		 *
		 * Memory for the contained type is reserved on the stack, but no
		 * initialisation is done. In other words, `T`'s constructor is _not_
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

		/// Nothings should cast implicitly to maybes
		constexpr maybe(nothing_t) noexcept {}

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

		// TODO: Add std::is_nothrow_constructible<T>::value check (gcc-4.8)
		~maybe() noexcept {
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
		noexcept(std::is_nothrow_copy_assignable<T>::value) {
			// Check for self-assignment
			if(this == &m)
				return *this;

			isValid = m.isValid;
			if(isValid) {
				reinterpret_cast<T&>(val) = reinterpret_cast<const T&>(m.val);
			}
			else
				self_destruct();

			return *this;
		}

		/// Move assignment
		maybe& operator= (maybe&& m)
		noexcept(std::is_nothrow_move_assignable<T>::value) {
			// Check for self-assignment
			if(this == &m)
				return *this;

			isValid = m.isValid;
			if(isValid) {
				reinterpret_cast<T&>(val)
					= std::move(reinterpret_cast<T&>(m.val));
			}
			else
				self_destruct();

			return *this;
		}

		/**
		 * Convenience copy assignment operator.
		 */
		maybe& operator= (const T& v)
		noexcept(std::is_nothrow_copy_assignable<T>::value) {
			if(isValid)
				reinterpret_cast<T&>(val) = v;

			else
				new (&val) T{v};

			return *this;
		}

		/**
		 * Convenience move assignment operator.
		 */
		maybe& operator= (T&& v)
		noexcept(std::is_nothrow_move_assignable<T>::value) {
			if(isValid)
				reinterpret_cast<T&>(val) = std::move(v);

			else
				new (&val) T{std::move(v)};

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
		 * \throws `invalid_maybe_access` if `this` is not a value.
		 */
		value_type& operator* () {
			if(!isValid)
				throw invalid_maybe_access{
					"Attempting to read the value of Nothing."
				};

			return reinterpret_cast<T&>(val);
		}

		/// \overload
		const value_type& operator* () const {
			if(!isValid)
				throw invalid_maybe_access{
					"Attempting to read the value of Nothing."
				};

			return reinterpret_cast<const T&>(val);
		}

		/**
		 * Member access operator.
		 * 
		 * \throws std::logic_error if `this` is `nothing`.
		 */
		value_type* operator-> () {
			if(!isValid)
				throw invalid_maybe_access{
					"Attempting to read the value of Nothing."
				};

			return reinterpret_cast<T*>(&val);
		}

		/// \overload
		const value_type* operator-> () const {
			if(!isValid)
				throw invalid_maybe_access{
					"Attempting to read the value of Nothing."
				};

			return reinterpret_cast<const T*>(&val);
		}

		iterator begin() noexcept {
			return iterator(this);
		}

		const_iterator begin() const noexcept {
			return const_iterator(const_cast<maybe*>(this));
		}

		constexpr const_iterator cbegin() const noexcept {
			return const_iterator(const_cast<maybe*>(this));
		}

		iterator end() noexcept {
			return iterator();
		}

		const_iterator end() const noexcept {
			return const_iterator();
		}

		constexpr const_iterator cend() const noexcept {
			return const_iterator();
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
	 * The `maybe` monad's main win is that it allows a user to postpone the
	 * need to check for value or nothing. This can result in much more compact
	 * code, that is nevertheless just as robust&mdash;if not more so.
	 *
	 * \ingroup maybe
	 */
	template<typename T>
	struct monad<maybe<T>>
	: deriving_pure<maybe<T>>
	, deriving_apply<in_terms_of_bind<maybe<T>>>
	, deriving_join<in_terms_of_bind<maybe<T>>> {

#ifdef DOCUMENTATION_GENERATOR
		/**
		 * Embed a pure value in a `maybe`.
		 *
		 * The same as constructing with `value(t)`.
		 */
		static constexpr maybe<T> pure(const T& t)
		noexcept(std::is_nothrow_copy_constructible<T>::value);

		/// \overload
		static constexpr maybe<T> pure(T&& t)
		noexcept(std::is_nothrow_move_constructible<T>::value);
#endif

		/**
		 * Apply `f` if `m` is a value.
		 *
		 * Example:
		 * \code
		 *   int foo(int x) {
		 *       return 2*x;
		 *   }
		 *
		 *   maybe<int> bar(maybe<int> x) {
		 *       return ftl::fmap(foo, x);
		 *   }
		 *
		 *   bar(ftl::value(5)); // Results in value(10)
		 *   bar(ftl::nothing);  // Results in nothing
		 * \endcode
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
		 * This allows users to sequence an arbitrary number of computations
		 * that might result in `nothing`, without having to explicitly check
		 * for `nothing` until the very end&mdash;if then, the result may well
		 * be further composed.
		 *
		 * Example:
		 * \code
		 *   maybe<float> foo(int);
		 *   maybe<string> bar(float);
		 *   maybe<int> baz(string);
		 *
		 *   maybe<int> run(maybe<int> m) {
		 *       return ((m >>= foo) >>= bar) >>= baz;
		 *   }
		 *
		 *   // Once again, without using bind
		 *   maybe<int> run2(maybe<int> m) {
		 *       if(m) {
		 *           auto m2 = foo(*m);
		 *           if(m2) {
		 *               auto m3 = bar(*m2);
		 *               if(m3) {
		 *                   return baz(*m3);
		 *               }
		 *           }
		 *       }
		 *
		 *       return nothing;
		 *   }
		 * \endcode
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
	: deriving_foldl<maybe<T>>
	, deriving_foldMap<maybe<T>>, deriving_fold<maybe<T>> {

		template<
				typename Fn,
				typename U,
				typename = Requires<
					std::is_convertible<
						typename std::result_of<Fn(T,U)>::type,U
					>::value
				>
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

