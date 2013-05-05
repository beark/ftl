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
#include "monoid.h"
#include "monad.h"

namespace ftl {

	/**
	 * Workaround to make vectors compatible with the functor series of
	 * concepts.
	 *
	 * If you require a different allocator than \c std::allocator, then make
	 * a similar type synonym and give it a monad instance of its own.
	 */
	template<typename T>
	using vector = std::vector<T, std::allocator<T>>;

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy Function<Container<B>(A)>
	 */
	template<
		typename F,
		typename A,
		typename B = typename decayed_result<F(A)>::type::value_type>
	vector<B> concatMap(F f, const vector<A>& v) {

		std::vector<B> result;
		result.reserve(v.size() * 2);	// Reasonable assumption? TODO: test!
		auto nested = f % v;

		for(auto& el : nested) {
			for(auto& e : el) {
				result.push_back(e);
			}
		}

		return result;
	}

	/**
	 * Monoid implementation for vectors.
	 */
	template<typename T>
	struct monoid<std::vector<T>> {
		static std::vector<T> id() {
			return std::vector<T>();
		}

		static std::vector<T> append(
				const std::vector<T>& v1,
				const std::vector<T>& v2) {
			auto rv(v1);
			rv.reserve(v2.size());
			rv.insert(rv.end(), v2.begin(), v2.end());
			return rv;
		}

		static std::vector<T> append(
				std::vector<T>&& v1,
				const std::vector<T>& v2) {
			v1.reserve(v2.size());
			v1.insert(v1.end(), v2.begin(), v2.end());
			return v1;
		}

		static std::vector<T> append(
				const std::vector<T>& v1,
				std::vector<T>&& v2) {
			v2.reserve(v1.size());
			v2.insert(v2.begin(), v1.begin(), v1.end());
			return v1;
		}

		static constexpr bool instance = true;
	};

	/**
	 * Monad implementation of vectors
	 */
	template<>
	struct monad<vector> {
		template<typename T>
		static vector<T> pure(const T& t) {
			return vector{t};
		}

		template<typename T>
		static vector<T> pure(T&& t) {
			return vector{std::move(t)};
		}

		/// Applies f to each element
		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type>
		static vector<B> map(F&& f, const vector<A>& v) {
			vector<B> ret;
			ret.reserve(v.size());
			for(const auto& e : v) {
				ret.push_back(f(e));
			}

			return ret;
		}

		/// Equivalent of flip(concatMap)
		template<
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type::value_type>
		static vector<B> bind(const vector<A>& v, F&& f) {
			return concatMap(std::forward<F>(f), v);
		}

		static constexpr bool instance = true;
	};

}

#endif

