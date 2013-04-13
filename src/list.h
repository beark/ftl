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
#ifndef FTL_LIST_H
#define FTL_LIST_H

#include <list>
#include "type_functions.h"
#include "monoid.h"
#include "monad.h"

/**
 * \file list.h
 *
 * Various concept implementations for std::lists.
 */

namespace ftl {

	/**
	 * Thin wrapper around std::list, to comply better with ftl.
	 *
	 * The behvariour of ftl::list should be essentially indistinguishable fom
	 * std::list, except that it does some type level manipulation of the
	 * allocator, allowing one to use Functor and Monad instance of list
	 * without worrying about it.
	 */
	template<typename T, typename Alloc = std::allocator<T> >
	class list {
	public:
		using value_type = T;
		using allocator_type =
			typename std::allocator_traits<Alloc>::template rebind_alloc<T>;

		using size_type = typename std::list<T, allocator_type>::size_type;
		using difference_type =
			typename std::list<T, allocator_type>::difference_type;

		using reference = typename std::list<T, allocator_type>::reference;
		using const_reference =
			typename std::list<T, allocator_type>::const_reference;

		using pointer = typename std::list<T, allocator_type>::pointer;
		using const_pointer =
			typename std::list<T, allocator_type>::const_pointer;

		using iterator = typename std::list<T, allocator_type>::iterator;
		using const_iterator =
			typename std::list<T, allocator_type>::const_iterator;

		using reverse_iterator =
			typename std::list<T, allocator_type>::reverse_iterator;

		using const_reverse_iterator =
			typename std::list<T, allocator_type>::const_reverse_iterator;

		explicit list(const allocator_type& alloc = allocator_type())
			: _list(alloc) {}

		list(
				size_type count, const T& value,
				const allocator_type& alloc = allocator_type()) 
			: _list(count, value, alloc) {}

		explicit list(size_type count) : _list(count) {}

		template<typename InputIt>
		list(
				InputIt first, InputIt last,
				const allocator_type& alloc = allocator_type())
			: _list(first, last, alloc) {}

		list(const list& other) : _list(other._list) {}
		list(const list& other, const allocator_type& alloc)
			: _list(other._list, alloc) {}

		list(list&& other) : _list(std::move(other._list)) {}
		list(list&& other, const allocator_type& alloc)
			: _list(std::move(other._list), alloc) {}

		list(
				std::initializer_list<T> init,
				const allocator_type& alloc = allocator_type())
			: _list(init, alloc) {}

		explicit list(const std::list<T, allocator_type>& other)
			: _list(other) {}

		explicit list(std::list<T, allocator_type>&& other)
			: _list(std::move(other)) {}

		~list() = default;

		list& operator= (const list& other) {
			_list = other._list;
			return *this;
		}

		list& operator= (list&& other) {
			_list = std::move(other._list);
			return *this;
		}

		void assign(size_type count, const T& value) {
			_list.assign(count, value);
		}

		template<typename InputIt>
		void assign(InputIt first, InputIt last) {
			_list.assign(first, last);
		}

		void assign(std::initializer_list<T> ilist) {
			_list.assign(ilist);
		}

		allocator_type get_allocator() const {
			return _list.get_allocator();
		}

		reference front() {
			return _list.front();
		}

		const_reference front() const {
			return _list.front();
		}

		reference back() {
			return _list.back();
		}

		const_reference back() const {
			return _list.back();
		}

		iterator begin() noexcept {
			return _list.begin();
		}

		const_iterator begin() const noexcept {
			return _list.begin();
		}

		const_iterator cbegin() const noexcept {
			return _list.cbegin();
		}

		iterator end() noexcept {
			return _list.end();
		}

		const_iterator end() const noexcept {
			return _list.end();
		}

		const_iterator cend() const noexcept {
			return _list.cend();
		}

		reverse_iterator rbegin() noexcept {
			return _list.rbegin();
		}

		const_reverse_iterator rbegin() const noexcept {
			return _list.rbegin();
		}

		const_reverse_iterator crbegin() const noexcept {
			return _list.crbegin();
		}

		reverse_iterator rend() noexcept {
			return _list.rend();
		}

		const_reverse_iterator rend() const noexcept {
			return _list.rend();
		}

		const_reverse_iterator crend() const noexcept {
			return _list.crend();
		}

		bool empty() const noexcept {
			return _list.empty();
		}

		size_type size() const noexcept {
			return _list.size();
		}

		size_type max_size() const noexcept {
			return _list.max_size();
		}

		void clear() noexcept {
			_list.clear();
		}

		iterator insert(iterator pos, const T& value) {
			return _list.insert(pos, value);
		}

		iterator insert(const_iterator pos, const T& value) {
			return _list.insert(pos, value);
		}

		iterator insert(const_iterator pos, T&& value) {
			return _list.insert(pos, std::move(value));
		}

		void insert(iterator pos, size_type count, const T& value) {
			_list.insert(pos, count, value);
		}

		void insert(const_iterator pos, size_type count, const T& value) {
			_list.insert(pos, count, value);
		}

		template<typename InputIt>
		iterator insert(iterator pos, InputIt first, InputIt last) {
			return _list.insert(pos, first, last);
		}

		template<typename InputIt>
		iterator insert(const_iterator pos, InputIt first, InputIt last) {
			return _list.insert(pos, first, last);
		}

		iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
			return _list.insert(pos, ilist);
		}

		template<typename...Args>
		iterator emplace(const_iterator pos, Args&&... args) {
			return _list.emplace(pos, std::forward(args)...);
		}

		iterator erase(iterator pos) {
			return _list.erase(pos);
		}

		iterator erase(const_iterator pos) {
			return _list.erase(pos);
		}

		iterator erase(iterator first, iterator last) {
			return _list.erase(first, last);
		}

		iterator erase(const_iterator first, const_iterator last) {
			return _list.erase(first, last);
		}

		void push_back(const T& value) {
			_list.push_back(value);
		}

		void push_back(T&& value) {
			_list.push_back(std::move(value));
		}

		template<typename...Args>
		void emplace_back(Args&&... args) {
			_list.emplace_back(std::forward(args)...);
		}

		void pop_back() {
			_list.pop_back();
		}

		template<typename...Args>
		void emplace_front(Args&&...args) {
			_list.emplace_front(std::forward(args)...);
		}

		void pop_front() {
			_list.pop_front();
		}

		void resize(size_type count) {
			_list.resize(count);
		}

		void resize(size_type count, const value_type& value) {
			_list.resize(count, value);
		}

		void swap(list& other) noexcept {
			_list.swap(other._list);
		}

		void merge(list& other) {
			_list.merge(other._list);
		}

		void merge(list&& other) {
			_list.merge(std::move(other._list));
		}

		template<typename Cmp>
		void merge(list& other, Cmp cmp) {
			_list.merge(other._list, cmp);
		}

		template<typename Cmp>
		void merge(list&& other, Cmp cmp) {
			_list.merge(std::move(other._list), cmp);
		}

		void splice(const_iterator pos, list& other) {
			_list.splice(pos, other._list);
		}

		void splice(const_iterator pos, list&& other) {
			_list.splice(pos, std::move(other._list));
		}

		void splice(const_iterator pos, list& other, const_iterator it) {
			_list.splice(pos, other._list, it);
		}

		void splice(const_iterator pos, list&& other, const_iterator it) {
			_list.splice(pos, std::move(other._list), it);
		}

		void splice(
				const_iterator pos, list& other,
				const_iterator first, const_iterator last) {
			_list.splice(pos, other._list, first, last);
		}

		void splice(
				const_iterator pos, list&& other,
				const_iterator first, const_iterator last) {
			_list.splice(pos, std::move(other._list), first, last);
		}

		void remove(const T& value) {
			_list.remove(value);
		}

		template<typename UnaryPred>
		void remove_if(UnaryPred p) {
			_list.remove_if(p);
		}

		void reverse() {
			_list.reverse();
		}

		void unique() {
			_list.unique();
		}

		template<typename BinaryPred>
		void unique(BinaryPred p) {
			_list.unique(p);
		}

		void sort() {
			_list.sort();
		}

		template<class Cmp>
		void sort(Cmp comp) {
			_list.sort(comp);
		}

		/// Get the underlying std::list object
		std::list<T, allocator_type>& underlyingList() {
			return _list;
		}

		/// \overload
		const std::list<T, allocator_type>& underlyingList() const {
			return _list;
		}

	private:
		std::list<T, allocator_type> _list;
	};

	template<typename T, typename A>
	bool operator== (const list<T, A>& lhs, const list<T, A>& rhs) {
		return lhs.underlyingList() == rhs.underlyingList();
	}

	template<typename T, typename A>
	bool operator!= (const list<T, A>& lhs, const list<T, A>& rhs) {
		return lhs.underlyingList() != rhs.underlyingList();
	}

	template<typename T, typename A>
	bool operator< (const list<T, A>& lhs, const list<T, A>& rhs) {
		return lhs.underlyingList() < rhs.underlyingList();
	}

	template<typename T, typename A>
	bool operator<= (const list<T, A>& lhs, const list<T, A>& rhs) {
		return lhs.underlyingList() <= rhs.underlyingList();
	}

	template<typename T, typename A>
	bool operator> (const list<T, A>& lhs, const list<T, A>& rhs) {
		return lhs.underlyingList() < rhs.underlyingList();
	}

	template<typename T, typename A>
	bool operator>= (const list<T, A>& lhs, const list<T, A>& rhs) {
		return lhs.underlyingList() <= rhs.underlyingList();
	}

	/**
	 * Functor instance of list.
	 */
	template<
		typename F,
		typename Alloc,
		typename A,
		typename B = typename decayed_result<F(A)>::type>
	list<B,Alloc> fmap(F f, const list<A,Alloc>& l) {
		list<B,Alloc> ret;
		for(const auto& e : l) {
			ret.push_back(f(e));
		}

		return ret;
	}

	/**
	 * Mappable::mutate implementation for list.
	 */
	template<typename T, typename A, typename F>
	list<T,A>& mutate(list<T,A>& l, F f) {
		for(auto& e : l) {
			e = f(e);
		}

		return l;
	}

	/**
	 * Mappable::each implementation for list.
	 */
	template<typename T, typename A, typename F>
	list<T,A>& each(list<T,A>& l, F f) {
		for(auto& e : l) {
			f(e);
		}

		return l;
	}

	/// \overload
	template<typename T, typename A, typename F>
	list<T,A>& each(const list<T,A>& l, F f) {
		for(const auto& e : l) {
			f(e);
		}

		return l;
	}

	/**
	 * Maps and concatenates in one step.
	 *
	 * \tparam F must satisfy Function<list<B>(A)>
	 */
	template<
		typename F,
		typename Alloc,
		typename A,
		typename B = typename decayed_result<F(A)>::type::value_type>
	list<B,Alloc> concatMap(F f, const list<A,Alloc>& l) {

		list<B,Alloc> result;
		auto nested = fmap(f, l);

		for(auto& el : nested) {
			for(auto& e : el) {
				result.push_back(e);
			}
		}

		return result;
	}

	/**
	 * Monoid implementation for list.
	 *
	 * The identity element is (naturally) the empty list, and the append
	 * operation is (again, naturally) to append the second list to the first.
	 */
	template<typename...Ps>
	struct monoid<list<Ps...>> {
		static list<Ps...> id() {
			return list<Ps...>();
		}

		static list<Ps...> append(
				const list<Ps...>& l1,
				const list<Ps...>& l2) {
			auto l3 = l1;
			l3.insert(l3.end(), l2.begin(), l2.end());
			return l3;
		}
	};

	template<typename...Ps>
	list<Ps...> operator^(const list<Ps...>& l1, const list<Ps...>& l2) {
		return monoid<list<Ps...>>::append(l1, l2);
	}

	/**
	 * Monad implementation for list.
	 */
	template<>
	struct monad<list> {

		template<typename A, typename Alloc>
		static list<A,Alloc> pure(const A& a) {
			return list<A,Alloc>(1, a);
		}

		template<typename A, typename Alloc>
		static list<A,Alloc> pure(A&& a) {
			list<A,Alloc> l;
			l.push_front(std::move(a));
			return l;
		}

		template<
			typename Alloc,
			typename F,
			typename A,
			typename B = typename decayed_result<F(A)>::type>
		static list<B,Alloc> bind(const list<A,Alloc>& l, F f) {
			return concatMap(f, l);
		}
	};

	/**
	 * Implementation of monoid for std::list.
	 *
	 * Equivalent of the list monoid. Provided so not only ftl::lists can have
	 * all the fun.
	 */
	template<typename...Ps>
	struct monoid<std::list<Ps...>> {
		static std::list<Ps...> id() {
			return std::list<Ps...>();
		}

		static std::list<Ps...> append(
				const std::list<Ps...>& l1,
				const std::list<Ps...>& l2) {
			auto l3 = l1;
			l3.insert(l3.end(), l2.begin(), l2.end());
			return l3;
		}
	};

	template<typename...Ps>
	std::list<Ps...> operator^(const std::list<Ps...>& l1, const std::list<Ps...>& l2) {
		return monoid<std::list<Ps...>>::append(l1, l2);
	}

}

#endif

