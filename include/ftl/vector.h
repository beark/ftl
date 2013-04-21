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
#ifndef FTL_VECTOR_H
#define FTL_VECTOR_H

#include <vector>
#include "type_functions.h"
#include "monoid.h"

namespace ftl {
	/**
	 * Implementation of mappable::map for vectors.
	 */
	template<
		typename F,
		typename A,
		template<typename, typename...> class Alloc,
		typename B = typename decayed_result<F(A)>::type,
		typename...AllocArgs>
	auto map(const F& f, const std::vector<A, Alloc<A, AllocArgs...>>& v)
	-> std::vector<B, Alloc<B, AllocArgs...>> {
		std::vector<B, Alloc<B, AllocArgs...>> result;
		result.reserve(v.size());

		for(const auto& e : v) {
			result.push_back(f(e));
		}

		return result;
	}

	/**
	 * Implementation of mappable::map for vectors.
	 */
	template<
		typename F,
		typename A,
		typename Alloc,
		typename B = typename decayed_result<F(A)>::type>
	std::vector<B, Alloc> map(const F& f, const std::vector<A, Alloc>& v) {
		std::vector<B, Alloc> result;
		result.reserve(v.size());

		for(const auto& e : v) {
			result.push_back(f(e));
		}

		return result;
	}

	/**
	 * Implementation of mappable::concatMap for vectors.
	 */
	template<
		typename F,
		template <typename, typename...> class Alloc,
		typename A,
		typename B = typename decayed_result<F(A)>::type::value_type,
		typename...AllocArgs>
	auto concatMap(const F& f, const std::vector<A, Alloc<A, AllocArgs...>>& v)
	-> std::vector<B, Alloc<B, AllocArgs...>> {

		std::vector<B, Alloc<B, AllocArgs...>> result;
		result.reserve(v.size() * 2);	// Reasonable assumption? TODO: test!

		auto nested = map(f, l);

		for(auto& el : nested) {
			for(auto& e : el) {
				result.push_back(e);
			}
		}

		return result;
	}

	/// \overload
	template<
		typename F,
		typename Alloc,
		typename A,
		typename B = typename decayed_result<F(A)>::type::value_type>
	auto concatMap(const F& f, const std::vector<A, Alloc>& v)
	-> std::vector<B, Alloc> {

		std::vector<B, Alloc> result;
		result.reserve(v.size() * 2);	// Reasonable assumption? TODO: test!

		auto nested = map(f, l);

		for(auto& el : nested) {
			for(auto& e : el) {
				result.push_back(e);
			}
		}

		return result;
	}

	/**
	 * Implementation of functor for vectors
	 */
	template<
		typename F,
		template <typename, typename...> class Alloc,
		typename A,
		typename B = decayed_result<F(A)>::type,
		typename...AllocArgs>
	auto fmap(const F& f, const std::vector<A, Alloc<A,AllocArgs...>>& v)
	-> std::vector<B, Alloc<B,AllocArgs...>>
		return map(f, l);
	}

	// \overload
	template<
		typename F,
		typename Alloc,
		typename A,
		typename B = decayed_result<F(A)>::type>
	std::vector<B, Alloc> fmap(const F& f, const std::vector<A, Alloc>& v) {
		return map(f, l);
	}

	/**
	 * Monoid implementation for vectors.
	 */
	template<typename...Ps>
	struct monoid<std::vector<Ps...>> {
		static std::vector<Ps...> id() {
			return std::vector<Ps...>();
		}

		static std::vector<Ps...> append(
				const std::vector<Ps...>& v1,
				const std::vector<Ps...>& v2) {
			auto rv(v1);
			rv.reserve(v2.size());
			rv.insert(rv.end(), v2.begin(), v2.end());
			return rv;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad::bind implementation for vectors.
	 */
	auto bind(const std::vector<A, Alloc<A, AllocArgs...>>& v, const F& f)
	-> std::vector<B, Alloc<B, AllocArgs...>> {
		return concatMap(f, v);
	}

}

#endif

