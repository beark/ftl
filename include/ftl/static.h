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
#ifndef FTL_STATIC_H
#define FTL_STATIC_H

namespace ftl
{
	/**
	 * \defgroup static Static
	 *
	 * A collection of utilities and functions for doing static computations.
	 *
	 * \code
	 *   #include <ftl/static.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - N/A
	 */

	/**
	 * Fold a variadic sequence at compile time.
	 *
	 * Use the static member function `with` to provide the binary folding
	 * function. The folding function has to conform to the following type
	 * signature: `(T,T) -> T` and be `constexpr`.
	 *
	 * \par Examples
	 * \code
	 *   static_assert(
	 *       ftl::static_fold<bool, true, true, true>::with(ftl::and_fn),
	 *       "true && true && true cannot be false");
	 * \endcode
	 *
	 * \ingroup static
	 */
	template<class T, T x, T...xs>
	struct static_fold {};

	template<class T, T x, T y, T...xs>
	struct static_fold<T,x,y,xs...>
	{
		template<class F>
		static constexpr T with(F f) noexcept(noexcept(f(x,y)))
		{
			return f(x, static_fold<T,y,xs...>::with(f));
		}
	};

	template<class T, T x>
	struct static_fold<T,x>
	{
		template<class F>
		static constexpr T with(F) noexcept
		{
			return x;
		}
	};
}

#endif
