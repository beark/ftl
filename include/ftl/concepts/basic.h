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
	 */

	/**
	 * Predicate to check if a type is \ref defcons.
	 *
	 * The expressions
	 * - `T t;`
	 * - `T t{};`
	 * - `T{}`
	 * - `T()`
	 * must all be valid and behave as expected.
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
	 */

	/**
	 * Predicate to check if a type is \ref movecons.
	 *
	 * The expressions
	 * - `T t = rv;`
	 * - `T(rv);`
	 * where `rv` is an rvalue reference of `T` must both be valid and behave
	 * as expected.
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
	 */

	/**
	 * Predicate to check if a type is \ref copycons.
	 *
	 * The expressions
	 * - `T t = v;`
	 * - `T(v);`
	 * where `v` is an instance of `T` must both be valid and result in objects
	 * that are equivalent of `v`, while leaving it completely unmodified.
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
	 * Any type that has defined a move assignment operator.
	 */

	/**
	 * Predicate to check if a type is \ref moveassignable.
	 *
	 * The expression
	 * - `a = rv;`
	 * where `rv` is an rvalue reference of `T` must be valid.
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
	 * Any type that has defined a copy assignment operator.
	 */

	/**
	 * Predicate to check if a type is \ref copyassignable.
	 *
	 * The expression
	 * - `a = v;`
	 * where `v` is an instance of `T` must be valid. After the operation, `a`
	 * must be equivalent of `v`, while leaving the latter completely
	 * unmodified.
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
	 * Any type that has a non-deleted, `noexcept` destructor.
	 */

	/**
	 * Predicate to check if a type is \ref destructible.
	 *
	 * The expression
	 * - `t.~T();`
	 * must be valid and result in all resources currently held exclusively by
	 * `t` being freed. No exception must be thrown. Accessing members of `t`
	 * after the destructor has been called may result in undefined or illegal
	 * behaviour.
	 *
	 * \ingroup concepts_basic
	 */
	template<typename T>
	constexpr bool Destructible() {
		return std::is_destructible<T>::value;
	}
}

#endif


