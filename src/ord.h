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
#ifndef FTL_ORD_H
#define FTL_ORD_H

#include "monoid.h"

namespace ftl {
	/**
	 * Encapsulation of the concept of an ordering.
	 *
	 * In essence, an ordering can be either 'less than' (Lt), 'equal' (Eq), or
	 * 'greater than' (Gt).
	 */
	class ord {
	public:
		enum ordering {
			Lt, Eq, Gt
		};

		constexpr ord() noexcept {}
		explicit constexpr ord(int n) noexcept : o(n < 0 ? Lt : (n > 0 ? Gt : Eq)) {}
		constexpr ord(const ord& order) noexcept : o(order.o) {}
		constexpr ord(ord&& order) noexcept : o(std::move(order.o)) {}
		constexpr ord(const ordering& order) noexcept : o(order) {}
		constexpr ord(ordering&& order) noexcept : o(std::move(order)) {}
		~ord() noexcept = default;

		/**
		 * Convenience operator for compatibility with stdlib's sort.
		 *
		 * \return true if the comparison resulting in this ord compared "less than".
		 */
		constexpr operator bool() noexcept {
			return o == Lt;
		}

		constexpr bool operator== (const ord& order) noexcept {
			return o == order.o;
		}

		constexpr bool operator== (ordering order) noexcept {
			return o == order;
		}

		constexpr bool operator!= (const ord& order) noexcept {
			return o != order.o;
		}

		constexpr bool operator!= (ordering order) noexcept {
			return o != order;
		}

	private:
		ordering o = Eq;
	};

	/**
	 * Monoid instance for ord.
	 *
	 * Quite neat in combination with the monoid instance for std::function.
	 *
	 * Semantics:
	 * \code
	 *   id() <=> Eq
	 *   append(a,b) <=> a, unless a == Eq, then b
	 * \endcode
	 */
	template<>
	struct monoid<ord> {
		static constexpr ord id() noexcept {
			return ord::Eq;
		}

		static constexpr ord append(ord o1, ord o2) noexcept {
			return (o1 == ord::Lt) ? o1 : (o1 == ord::Eq ? o2 : o1);
		}
	};

	constexpr ord operator^ (ord o1, ord o2) noexcept {
		return monoid<ord>::append(o1, o2);
	}
}

#endif

