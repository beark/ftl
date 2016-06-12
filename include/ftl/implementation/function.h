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
#ifndef FTL_FUNCTION_IMPL_H
#define FTL_FUNCTION_IMPL_H

#include <memory>
#include <stdexcept>
#include <functional>
#include "../type_functions.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

namespace ftl {

	template<typename> class function;

	template<typename>
	struct force_function_heap_allocation : std::false_type {};

	// Namespace for internal details
	namespace dtl_ {
		struct manager_storage_type;

		struct functor_padding {
		protected:
			size_t padding_fst;
			size_t padding_snd;
		};

		struct empty_struct {};

		template<typename R, typename...Ps>
		R empty_call(const functor_padding&, Ps...) {
			throw std::bad_function_call();
		}

		template<typename T, typename Alloc>
		struct is_inplace_allocated {
			static constexpr bool value
				// so that it fits
				= sizeof(T) <= sizeof(functor_padding)

				// so that it will be aligned
				&& (std::alignment_of<functor_padding>::value
					% std::alignment_of<T>::value == 0)

				// so that we can offer noexcept move
				&& std::is_nothrow_move_constructible<T>::value

				// so that the user can override it
				&& !force_function_heap_allocation<T>::value;
		};

		template<typename T>
		T to_functor(T&& func) {
			return std::forward<T>(func);
		}
		template<typename R, typename C, typename...Ps>
		auto to_functor(R (C::*func)(Ps...)) -> decltype(std::mem_fn(func)) {
			return std::mem_fn(func);
		}
		template<typename R, typename C, typename...Ps>
		auto to_functor(R (C::*func)(Ps...) const) -> decltype(std::mem_fn(func)) {
			return std::mem_fn(func);
		}

		template<typename T>
		struct functor_type {
			using type = decltype(to_functor(std::declval<T>()));
		};

		template<typename T>
		bool is_null(const T&) {
			return false;
		}

		template<typename R, typename...Ps>
		bool is_null(R (* const & fp)(Ps...)) {
			return fp == nullptr;
		}

		template<typename R, typename C, typename...Ps>
		bool is_null(R (C::* const & fp)(Ps...)) {
			return fp == nullptr;
		}

		template<typename R, typename C, typename...Ps>
		bool is_null(R (C::* const & fp)(Ps...) const) {
			return fp == nullptr;
		}

		template<typename, typename>
		struct is_valid_function_argument {
			static constexpr bool value = false;
		};

		template<typename R, typename...Ps>
		struct is_valid_function_argument<function<R(Ps...)>, R (Ps...)> {
			static constexpr bool value = false;
		};

		template<typename T, typename R, typename...Ps>
		struct is_valid_function_argument<T, R (Ps...)> {

			template<typename U>
			static decltype(
				to_functor(std::declval<U>())(std::declval<Ps>()...)
			)
			check(U *);

			template<typename>
			static empty_struct check(...);

			static constexpr bool value
				= std::is_convertible<decltype(check<T>(nullptr)), R>::value;
		};

		enum function_manager_calls
		{
			call_move_and_destroy,
			call_copy,
			call_copy_functor_only,
			call_destroy,
		};

		template<typename T, typename Allocator>
		void* function_manager(
				void* first_arg,
				void* second_arg,
				function_manager_calls call_type);

		typedef void *(*manager_type)(void*, void*, function_manager_calls);

		struct manager_storage_type {

			template<typename Allocator>
			Allocator& get_allocator() noexcept {
				return reinterpret_cast<Allocator&>(manager);
			}

			template<typename Allocator>
			const Allocator& get_allocator() const noexcept {
				return reinterpret_cast<const Allocator&>(manager);
			}

			functor_padding functor;
			manager_type manager;
		};

		template<typename T, typename Allocator, typename Enable = void>
		struct function_manager_inplace_specialisation {

			template<typename R, typename...Ps>
			static R call(const functor_padding& storage, Ps... ps) {
				// do not call get_functor_ref because I want this function to
				// be fast in debug when nothing gets inlined
				return
					reinterpret_cast<T&>(
						const_cast<functor_padding&>(storage))(std::forward<Ps>(ps)...);
			}

			static void store_functor(manager_storage_type& storage, T to_store) {
				new (&get_functor_ref(storage)) T(std::forward<T>(to_store));
			}

			static void move_functor(
					manager_storage_type& lhs, manager_storage_type&& rhs
			)
			noexcept {
				new (&get_functor_ref(lhs)) T(std::move(get_functor_ref(rhs)));
			}

			static void destroy_functor(
					Allocator &, manager_storage_type & storage
			)
			noexcept {
				get_functor_ref(storage).~T();
			}

			static T& get_functor_ref(const manager_storage_type& storage) noexcept {
				return reinterpret_cast<T&>(
					const_cast<functor_padding&>(storage.functor)
				);
			}
		};

		template<typename T, typename Allocator>
		struct function_manager_inplace_specialisation<
					T,
					Allocator,
					typename std::enable_if<
						!is_inplace_allocated<T, Allocator>::value>::type> {

			using alloc_traits = std::allocator_traits<Allocator>;
			using ptr_t = typename alloc_traits::pointer;

			template<typename R, typename...Ps>
			static R call(const functor_padding& storage, Ps... ps) {
				// do not call get_functor_ptr_ref because I want this function
				// to be fast in debug when nothing gets inlined
				return
					(*reinterpret_cast<ptr_t&>(const_cast<functor_padding&>(storage)))(
						std::forward<Ps>(ps)...
					);
			}

			static void store_functor(manager_storage_type& self, T to_store) {

				Allocator& allocator = self.get_allocator<Allocator>();;
				static_assert(
						sizeof(ptr_t) <= sizeof(self.functor),
						"The allocator's pointer type is too big"
				);

				ptr_t* ptr = new (&get_functor_ptr_ref(self)) ptr_t(
						alloc_traits::allocate(allocator, 1)
				);

				alloc_traits::construct(allocator, *ptr, std::forward<T>(to_store));
			}

			static void move_functor(
					manager_storage_type& lhs, manager_storage_type&& rhs
			)
			noexcept {

				static_assert(
					std::is_nothrow_move_constructible<ptr_t>::value,
					"Cannot offer noexcept swap if the pointer type is "
					"not nothrow move constructible"
				);

				new (&get_functor_ptr_ref(lhs)) ptr_t(
						std::move(get_functor_ptr_ref(rhs))
				);

				// this next assignment makes the destroy function easier
				get_functor_ptr_ref(rhs) = nullptr;
			}

			static void destroy_functor(
					Allocator& allocator, manager_storage_type& storage
			)
			noexcept {

				ptr_t& pointer = get_functor_ptr_ref(storage);
				if (!pointer)
					return;

				alloc_traits::destroy(allocator, pointer);
				alloc_traits::deallocate(allocator, pointer, 1);
			}

			static T & get_functor_ref(const manager_storage_type& storage) noexcept {
				return *get_functor_ptr_ref(storage);
			}

			static ptr_t& get_functor_ptr_ref(const manager_storage_type& storage)
			noexcept {
				return reinterpret_cast<ptr_t&>(
						const_cast<functor_padding&>(storage.functor)
				);
			}
		};

		template<typename T, typename Allocator>
		static void create_manager(manager_storage_type& storage, Allocator&& allocator)
		{
			new (&storage.get_allocator<Allocator>()) Allocator(std::move(allocator));
			storage.manager = &function_manager<T, Allocator>;
		}

		// this function acts as a vtable. it is an optimization to prevent
		// code-bloat from rtti. see the documentation of boost::function
		template<typename T, typename Allocator>
		void* function_manager(
				void* first_arg, void* second_arg,
				function_manager_calls call_type
		)
		{
			using specialisation
				= function_manager_inplace_specialisation<T,Allocator>;

			static_assert(
				std::is_empty<Allocator>::value,
				"Allocator must be an empty class, else it cannot fit in storage"
			);

			switch(call_type) {

			case call_move_and_destroy: {

				manager_storage_type& lhs =
					*static_cast<manager_storage_type*>(first_arg);

				manager_storage_type& rhs =
					*static_cast<manager_storage_type*>(second_arg);

				specialisation::move_functor(lhs, std::move(rhs));
				specialisation::destroy_functor(rhs.get_allocator<Allocator>(), rhs);

				create_manager<T,Allocator>(
						lhs, std::move(rhs.get_allocator<Allocator>())
				);

				rhs.get_allocator<Allocator>().~Allocator();

				return nullptr;
			}

			case call_copy: {

				manager_storage_type& lhs =
					*static_cast<manager_storage_type*>(first_arg);

				const manager_storage_type& rhs =
					*static_cast<const manager_storage_type*>(second_arg);

				create_manager<T, Allocator>(
						lhs, Allocator(rhs.get_allocator<Allocator>())
				);

				specialisation::store_functor(
						lhs, const_cast<const T&>(specialisation::get_functor_ref(rhs))
				);

				return nullptr;
			}

			case call_destroy: {

				manager_storage_type& self =
					*static_cast<manager_storage_type *>(first_arg);

				specialisation::destroy_functor(self.get_allocator<Allocator>(), self);

				self.get_allocator<Allocator>().~Allocator();

				return nullptr;
			}

			case call_copy_functor_only:

				specialisation::store_functor(
						*static_cast<manager_storage_type *>(first_arg),
						const_cast<const T&>(
							specialisation::get_functor_ref(
								*static_cast<const manager_storage_type*>(second_arg)
							)
						)
				);

				return nullptr;

			default:
				return nullptr;
			}
		}

		template<typename...>
		struct curried {};

		template<typename R>
		struct curried<R> {
			R operator()() const {
				throw(std::logic_error("Curried calling of parameterless function"));
			}
		};

		template<typename R, typename P>
		struct curried<R,P> {
			function<R()> operator()(P) const {
				throw(std::logic_error("Curried calling of parameterless function"));
			}
		};

		template<typename R, typename P1, typename P2, typename...Ps>
		struct curried<R,P1,P2,Ps...> {
		private:
			using applied_type = function<R(P2,Ps...)>;

			// Apply one argument.
			applied_type apply_one(P1 p1) const {
				auto self = *reinterpret_cast<const function<R(P1,P2,Ps...)>*>(this);
				return [self,p1] (P2 p2, Ps...ps) {
					return self.operator()(
							p1, std::forward<P2>(p2), std::forward<Ps>(ps)...
					);
				};
			}
		public:
			function<R(P2,Ps...)> operator() (P1 p1) const {
				return apply_one(std::move(p1));
			}

			// Apply each argument, return a new function.
			// If the number of arguments equals the function's arity,
			// ftl::function::operator() will be called instead.
			template<typename...Ps2>
			auto operator()(P1 p1, P2 p2, Ps2&&...ps2) const
			-> typename std::result_of<applied_type(P2,Ps2...)>::type
			{
				applied_type g = apply_one(std::move(p1));
				return g(std::move(p2), std::forward<Ps2>(ps2)...);
			}
		};

	}
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#endif

