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
#ifndef FTL_BASIC_CONCEPTS_H
#define FTL_BASIC_CONCEPTS_H

#include <type_traits>

namespace ftl {
	/**
	 * \defgroup concepts_basic Basic Concepts
	 *
	 * Module containg definitions and checks for various basic concepts.
	 *
	 * \code
	 *   #include <ftl/concepts/basic.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - <type_traits>
	 */

	/**
	 * \page defcons DefaultConstructible
	 *
	 * Any type that has a default constructor.
	 *
	 * This includes types that have an implicit default constructor (by either
	 * not declaring any of the standard constructors, or declaring it as
	 * `default`).
	 *
	 * More formally, the expressions
	 * - `T t;`
	 * - `T t{};`
	 * - `T{}`
	 * - `T()`
	 * must all be valid and behave as expected. Which is to say, they should
	 * construct an instance of `T` with whatever default semantics are
	 * appropriate.
	 */

	/**
	 * Predicate to check if a type is \ref defcons.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   void foo() {
	 *       static_assert(
	 *           DefaultConstructible<T>(),
	 *           "foo: T is not an instance of DefaultConstructible"
	 *       );
	 *
	 *       // Construct Ts using its default c-tor
	 *   }
	 * \endcode
	 *
	 * \ingroup concepts_basic
	 */
	template<typename T>
	constexpr bool DefaultConstructible() {
		return std::is_default_constructible<T>::value;
	}

	/**
	 * \page movecons MoveConstructible
	 *
	 * Any type that has a move constructor.
	 *
	 * This includes types that have an implicit move constructor (by either not
	 * declaring any of the standard constructors, or declaring it as
	 * `default`).
	 *
	 * More formally, the expressions
	 * - `T t = rv;`
	 * - `T(rv);`
	 * where `rv` is an rvalue reference of `T` must both be valid and behave
	 * as expected.
	 */

	/**
	 * Predicate to check if a type is \ref movecons.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   void foo() {
	 *       static_assert(
	 *           MoveConstructible<T>(),
	 *           "foo: T is not an instance of MoveConstructible"
	 *       );
	 *
	 *       // Construct Ts using its move c-tor
	 *   }
	 * \endcode
	 *
	 * \ingroup concepts_basic
	 */
	template<typename T>
	constexpr bool MoveConstructible() {
		return std::is_move_constructible<T>::value;
	}

	/**
	 * \page copycons CopyConstructible
	 *
	 * Any type that has a copy constructor.
	 *
	 * This includes types that have an implicit copy constructor (by either not
	 * declaring any of the standard constructors, or declaring it as
	 * `default`).
	 *
	 * More formally, the expressions
	 * - `T t = v;`
	 * - `T(v);`
	 * where `v` is an instance of `T` must both be valid and result in objects
	 * that are equivalent of `v`, while leaving it completely unmodified.
	 */

	/**
	 * Predicate to check if a type is \ref copycons.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   void foo() {
	 *       static_assert(
	 *           CopyConstructible<T>(),
	 *           "foo: T is not an instance of CopyConstructible"
	 *       );
	 *
	 *       // Construct Ts using its copy c-tor
	 *   }
	 * \endcode
	 *
	 * \ingroup concepts_basic
	 */
	template<typename T>
	constexpr bool CopyConstructible() {
		return std::is_copy_constructible<T>::value;
	}

	/**
	 * \page moveassignable MoveAssignable
	 *
	 * Types that can be move assigned to.
	 *
	 * Requires that the expression
	 * - `a = rv;`
	 * where `rv` is an rvalue reference of `T` is valid. After the operation,
	 * `a` must be equivalent of whatever `rv` was _before_ it. `rv` may be
	 * changed by the operation.
	 */

	/**
	 * Predicate to check if a type is \ref moveassignable.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   void foo() {
	 *       static_assert(
	 *           MoveAssignable<T>(),
	 *           "foo: T is not an instance of MoveAssignable"
	 *       );
	 *
	 *       // Assign rvalues to Ts
	 *   }
	 * \endcode
	 *
	 * \ingroup concepts_basic
	 */
	template<typename T>
	constexpr bool MoveAssignable() {
		return std::is_move_assignable<T>::value;
	}

	/**
	 * \page copyassignable CopyAssignable
	 *
	 * Types that can be copy assigned to.
	 *
	 * Requires that the expression
	 * - `a = v;`
	 * where `v` is an instance of `T` is valid. After the operation, `a`
	 * must be equivalent of `v`, while leaving the latter completely
	 * unmodified.
	 */

	/**
	 * Predicate to check if a type is \ref copyassignable.
	 *
	 * Example:
	 * \code
	 *   template<typename T>
	 *   void foo() {
	 *       static_assert(
	 *           CopyAssignable<T>(),
	 *           "foo: T is not an instance of CopyAssignable"
	 *       );
	 *
	 *       // Assign lvalues to Ts
	 *   }
	 * \endcode
	 *
	 * \ingroup concepts_basic
	 */
	template<typename T>
	constexpr bool CopyAssignable() {
		return std::is_copy_assignable<T>::value;
	}

	/**
	 * \page destructible Destructible
	 *
	 * Types that can be destructed.
	 *
	 * The expression
	 * - `t.~T();`
	 * must be valid and result in all resources currently held exclusively by
	 * `t` being freed. No exception must be thrown. Accessing members of `t`
	 * after the destructor has been called may result in undefined or illegal
	 * behaviour.
	 */

	/**
	 * Predicate to check if a type is \ref destructible.
	 *
	 * \ingroup concepts_basic
	 */
	template<typename T>
	constexpr bool Destructible() {
		return std::is_destructible<T>::value;
	}
}

#endif


