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

/*
 * Big thanks to Malte Skarupke for the original implementation this
 * version is based on! See
 * http://probablydance.com/2013/01/13/a-faster-implementation-of-stdfunction/
 * for details.
 */
#ifndef FTL_FUNCTION_H
#define FTL_FUNCTION_H

#include "implementation/function.h"

namespace ftl {

	/**
	 * \defgroup function Function
	 *
	 * Generalised function type similar to `std::function`.
	 *
	 * \code
	 *   #include <ftl/function.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - `<memory>`
	 * - `<stdexcept>`
	 * - `<functional>`
	 * - \ref typelevel
	 */

	/**
	 * Generic, type erasing function object encapsulator.
	 *
	 * \tparam R Return value of the wrapped function or function object.
	 * \tparam Ps Parameter pack of the wrapped function's `operator()`.
	 *
	 * The point of including this data type is that, unlike `std::function`,
	 * it provides built-in support for curried calling.
	 *
	 * \par Concepts
	 * - \ref fullycons
	 * - \ref assignable
	 * - \ref fn`<R(Ps...)>`
	 * - \ref fn`<`\ref fn`<R(P2, ..., PN)>(P1)>`, assuming `sizeof...(Ps) > 2`
	 *   and `P1`, `P2`, ..., `PN` are the elements of `Ps`
	 * - \ref functorpg (instance defined in \ref functional)
	 * - \ref applicativepg (instance defined in \ref functional)
	 * - \ref monadpg (instance defined in \ref functional)
	 * - \ref monoidpg, if and only if `R` is a monoid (instance defined in
	 *   \ref functional)
	 *
	 * If the second \ref fn instance above is not completely obvious in what it
	 * means, it signifies that ftl::function supports the notion of curried
	 * calling. For example:
	 * \code
	 *   ftl::function<int,int,int> f = ... // Binary function of integers
	 *   f(1,2);        // Call directly
	 *   auto g = f(1); // Curried call, providing one parameter only
	 *   g(2);          // Results in the same as f(1,2);
	 * \endcode
	 *
	 * \note While often convenient, it may give rise to ambiguous calls to
	 * `operator()` if the function object encapsulated by the `ftl::function`
	 * has several overloaded call operators that differ only in the number of
	 * parameters they take.
	 *
	 * \warning Curried calling _will_ result in copies of the parameter being
	 *          made. Every time you invoke `operator()` without filling the
	 *          complete parameter list, you are creating copies.
	 *
	 * \ingroup function
	 */
	template<typename>
	class function {};

	template<typename R, typename...Ps>
	class function<R(Ps...)> : private ::ftl::dtl_::curried<R,Ps...> {
	public:
		/**
		 * Type sequence representation of the function's parameter list.
		 *
		 * Use in combination with e.g. get_nth to get the type of any
		 * parameter.
		 */
		using parameter_types = type_seq<Ps...>;

		/// Type returned when calling the function object.
		using result_type = R;

		/// Equivalent of function(std::nullptr_t)
		function() noexcept {
			initialise_empty();
		}

		/// Initialise a nullary function wrapper
		function(std::nullptr_t) noexcept {
			initialise_empty();
		}

		function(const function& f) : call(f.call) {
			f.manager_storage.manager(
					&manager_storage,
					const_cast<::ftl::dtl_::manager_storage_type*>(&f.manager_storage),
					::ftl::dtl_::call_copy);
		}

		function(function&& f) noexcept {
			initialise_empty();
			swap(f);
		}

		/**
		 * Construct from arbitrary function object.
		 *
		 * \tparam F must have a function call operator matching the type the
		 *           `ftl::function` is declared as.
		 */
		template<typename F>
		function(
				F f,
				typename std::enable_if<
					::ftl::dtl_::is_valid_function_argument<F, R (Ps...)>::value,
					::ftl::dtl_::empty_struct>::type = ::ftl::dtl_::empty_struct()
		)
		noexcept(::ftl::dtl_::is_inplace_allocated<
				F,
				std::allocator<typename ::ftl::dtl_::functor_type<F>::type>>::value
		)
		{
			if(::ftl::dtl_::is_null(f))
				initialise_empty();

			else {
				using functor_type = typename ::ftl::dtl_::functor_type<F>::type;
				initialise(
					::ftl::dtl_::to_functor(
						std::forward<F>(f)
					),
					std::allocator<functor_type>()
				);
			}
		}

		/// Default construct using a custom allocator
		template<typename Allocator>
		function(std::allocator_arg_t, const Allocator&) {
			// ignore the allocator because I don't allocate
			initialise_empty();
		}

		/// Null construct using a custom allocator
		template<typename Allocator>
		function(std::allocator_arg_t, const Allocator&, std::nullptr_t) {
			// ignore the allocator because I don't allocate
			initialise_empty();
		}

		/**
		 * Construct from arbitrary function object using a custom allocator.
		 */
		template<typename Allocator, typename F>
		function(
				std::allocator_arg_t,
				const Allocator& allocator,
				F functor,
				typename std::enable_if<
					::ftl::dtl_::is_valid_function_argument<F, R (Ps...)>::value,
					::ftl::dtl_::empty_struct>::type = ::ftl::dtl_::empty_struct())
		noexcept(::ftl::dtl_::is_inplace_allocated<F, Allocator>::value) {

			if(::ftl::dtl_::is_null(functor))
				initialise_empty();

			else {
				initialise(::ftl::dtl_::to_functor(
					std::forward<F>(functor)), Allocator(allocator)
				);
			}
		}

		/// Copy construct using a custom allocator
		template<typename Allocator>
		function(std::allocator_arg_t,
				const Allocator& allocator,
				const function& other
		)
		: call(other.call) {

			using alloc_traits = std::allocator_traits<Allocator>;
			using MyAllocator =
				typename alloc_traits::template rebind_alloc<function>::other;

			// first try to see if the allocator matches the target type
			::ftl::dtl_::manager_type manager_for_allocator =
				&::ftl::dtl_::function_manager<
					typename alloc_traits::value_type, Allocator
				>;

			if(other.manager_storage.manager == manager_for_allocator) {

				::ftl::dtl_::create_manager<
					typename alloc_traits::value_type, Allocator
				> (
					manager_storage, Allocator(allocator)
				);

				manager_for_allocator(
					&manager_storage,
					const_cast<::ftl::dtl_::manager_storage_type*>(
						&other.manager_storage
					),
					::ftl::dtl_::call_copy_functor_only
				);
			}

			// if it does not, try to see if the target contains my type. this
			// breaks the recursion of the last case. otherwise repeated copies
			// would allocate more and more memory
			else if(other.manager_storage.manager
					== &::ftl::dtl_::function_manager<function, MyAllocator>
			) {

				::ftl::dtl_::create_manager<function, MyAllocator>(
					manager_storage,
					MyAllocator(allocator)
				);

				::ftl::dtl_::function_manager<function, MyAllocator>(
					&manager_storage,
					const_cast<::ftl::dtl_::manager_storage_type*>(
						&other.manager_storage
					),
					::ftl::dtl_::call_copy_functor_only
				);
			}

			else
			{
				// else store the other function as my target
				initialise(other, MyAllocator(allocator));
			}
		}

		/// Move construct using a custom allocator
		template<typename Allocator>
		function(std::allocator_arg_t, const Allocator&, function&& other) noexcept {
			// ignore the allocator because there's no allocation
			initialise_empty();
			swap(other);
		}

		~function() noexcept {
			manager_storage.manager(
					&manager_storage,
					nullptr,
					::ftl::dtl_::call_destroy);
		}

		function& operator= (function other) noexcept {
			swap(other);
			return *this;
		}

		// Inherit the curried function call operator(s)
		using ::ftl::dtl_::curried<R,Ps...>::operator();

		/// Call the wrapped function object
		R operator()(Ps...ps) const {
			return call(manager_storage.functor, std::forward<Ps>(ps)...);
		}

		template<typename F, typename Allocator>
		void assign(F&& f, const Allocator& alloc)
		noexcept(::ftl::dtl_::is_inplace_allocated<F, Allocator>::value) {
			function(std::allocator_arg, alloc, f).swap(*this);
		}

		void swap(function& other) noexcept {
			::ftl::dtl_::manager_storage_type temp_storage;

			other.manager_storage.manager(
					&temp_storage,
					&other.manager_storage,
					::ftl::dtl_::call_move_and_destroy);

			manager_storage.manager(
					&other.manager_storage,
					&manager_storage,
					::ftl::dtl_::call_move_and_destroy);

			temp_storage.manager(
					&manager_storage,
					&temp_storage,
					::ftl::dtl_::call_move_and_destroy);

			std::swap(call, other.call);
		}

		/// Check if function is nullary
		constexpr operator bool() const noexcept {
			return call != &::ftl::dtl_::empty_call<R, Ps...>;
		}

	private:
		::ftl::dtl_::manager_storage_type manager_storage;
		R (*call)(const ::ftl::dtl_::functor_padding&, Ps...);

		template<typename F, typename Allocator>
		void initialise(F f, Allocator&& alloc) {

			call = &::ftl::dtl_::function_manager_inplace_specialisation<F,Allocator>
				::template call<R, Ps...>;

			::ftl::dtl_::create_manager<F,Allocator>(
					manager_storage,
					std::forward<Allocator>(alloc));

			::ftl::dtl_::function_manager_inplace_specialisation<F, Allocator>
				::store_functor(manager_storage, std::forward<F>(f));
		}

		using empty_fn_type = R(*)(Ps...);

		void initialise_empty() noexcept {
			using Allocator = std::allocator<empty_fn_type>;

			static_assert(
				::ftl::dtl_::is_inplace_allocated<empty_fn_type, Allocator>::value,
				"The empty function should benefit from small functor optimization");

			::ftl::dtl_::create_manager<empty_fn_type,Allocator>(
					manager_storage,
					Allocator()
			);

			::ftl::dtl_
				::function_manager_inplace_specialisation<empty_fn_type,Allocator>
					::store_functor(manager_storage, nullptr);

			call = &::ftl::dtl_::empty_call<R, Ps...>;
		}
	};

	template<typename R, typename...Ps>
	struct parametric_type_traits<function<R(Ps...)>> {
		using value_type = R;

		template<typename S>
		using rebind = function<S(Ps...)>;
	};

	template<class R, class...Args>
	struct fn_traits<function<R(Args...)>> : fn_traits<R(Args...)> {};

}

#endif

