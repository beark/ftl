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
#ifndef FTL_ITERATOR_CONCEPTS_H
#define FTL_ITERATOR_CONCEPTS_H

#include <iterator>
#include "../type_functions.h"
#include "basic.h"
#include "orderable.h"

namespace ftl {
	/**
	 * \defgroup concepts_iterator Iterator Concepts
	 *
	 * Module containg definitions and checks for various iterator concepts.
	 *
	 * \code
	 *   #include <ftl/concepts/iterator.h>
	 * \endcode
	 *
	 * \par Dependencies
	 * - `<iterator>`
	 * - \ref typelevel
	 * - \ref concepts_basic
	 * - \ref orderable
	 */

	namespace _dtl {
		FTL_GEN_PREUNOP_TEST(*, deref);
		FTL_GEN_PREUNOP_TEST(++, pre_inc);
		FTL_GEN_POSTUNOP_TEST(++, post_inc);
		FTL_GEN_PREUNOP_TEST(--, pre_dec);
		FTL_GEN_POSTUNOP_TEST(--, post_dec);
		FTL_GEN_TYPEMEM_TEST(reference);
		FTL_GEN_TYPEMEM_TEST(value_type);
		FTL_GEN_TYPEMEM_TEST(pointer);

		template<typename T>
		auto test_pointer(decltype(std::declval<T>().operator->())*)
		-> decltype(std::declval<T>().operator->());

		template<typename>
		no test_pointer(...);
	}

	/**
	 * Compile time check if a type imlements the Iterator concept.
	 *
	 * The tests performed are designed to make sure `It` conforms to the
	 * requirements described e.g.
	 * [here](http://en.cppreference.com/w/cpp/concept/Iterator).
	 *
	 * \par Examples
	 *
	 * Examining a couple of types:
	 * \code
	 *   std::cout << std::boolalpha
	 *       << ftl::Iterator<std::vector<int>::iterator>{}
	 *       << ", "
	 *       << ftl::Iterator<int>{}
	 *       << ", "
	 *       << ftl::Iterator<int*>{}
	 *       << std::endl;
	 * \endcode
	 * Output:
	 * \code
	 *   true, false, true
	 * \endcode
	 *
	 * \ingroup concepts_iterator
	 */
	template<typename It>
	struct Iterator {
		using reference_type = decltype(
			_dtl::test_typemem_reference<std::iterator_traits<It>>(nullptr)
		);

		static constexpr bool value =
			CopyConstructible<It>()
			&& CopyAssignable<It>()
			&& Destructible<It>()
			&& !std::is_same<reference_type, _dtl::no>::value
			&& std::is_same<
				reference_type,
				decltype(_dtl::test_deref<It>(nullptr))
			>::value
			&& std::is_same<
				decltype(_dtl::test_pre_inc<It>(nullptr)),
				It&
			>::value;

		constexpr operator bool() const noexcept {
			return value;
		}
	};

	/**
	 * Compile time check if a type imlements the InputIterator concept.
	 *
	 * The tests performed are designed to make sure `It` conforms to the
	 * requirements described e.g.
	 * [here](http://en.cppreference.com/w/cpp/concept/InputIterator).
	 *
	 * \par Examples
	 *
	 * Examining a couple of types:
	 * \code
	 *   std::cout << std::boolalpha
	 *       << ftl::InputIterator<std::vector<int>::iterator>{}
	 *       << ", "
	 *       << ftl::InputIterator<int>{}
	 *       << ", "
	 *       << ftl::InputIterator<int*>{}
	 *       << std::endl;
	 * \endcode
	 * Output:
	 * \code
	 *   true, false, true
	 * \endcode
	 *
	 * \ingroup concepts_iterator
	 */
	template<typename It>
	struct InputIterator {
		using value_type = decltype(
			_dtl::test_typemem_value_type<std::iterator_traits<It>>(nullptr)
		);

		using pointer = decltype(
			_dtl::test_typemem_pointer<std::iterator_traits<It>>(nullptr)
		);

		using test_ptr = if_<std::is_pointer<It>::value,
			  pointer,
			  decltype(_dtl::test_pointer<It>(nullptr))
		>;

		static constexpr bool value =
			Iterator<It>()
			&& Eq<It>()
			&& !std::is_same<value_type, _dtl::no>::value
			&& std::is_convertible<
				decltype(_dtl::test_deref<It>(nullptr)),
				value_type
			>::value
			&& !std::is_same<pointer, _dtl::no>::value
			&& std::is_convertible<test_ptr,pointer>::value;

		constexpr operator bool() const noexcept {
			return value;
		}
	};

	/**
	 * Compile time check if a type imlements the ForwardIterator concept.
	 *
	 * The tests performed are designed to make sure `It` conforms to the
	 * requirements described e.g.
	 * [here](http://en.cppreference.com/w/cpp/concept/ForwardIterator).
	 *
	 * \par Examples
	 *
	 * Examining a couple of types:
	 * \code
	 *   std::cout << std::boolalpha
	 *       << ftl::ForwardIterator<int>{}
	 *       << ", "
	 *       << ftl::ForwardIterator<std::vector<int>::iterator>{}
	 *       << ", "
	 *       << ftl::ForwardIterator<int*>{}
	 *       << ", "
	 *       << ftl::ForwardIterator<ftl::maybe<int>::iterator>{}
	 *       << std::endl;
	 * \endcode
	 * Output:
	 * \code
	 *   false, true, true, true
	 * \endcode
	 *
	 * \ingroup concepts_iterator
	 */
	template<typename It>
	struct ForwardIterator {
		static constexpr bool value =
			InputIterator<It>()
			&& DefaultConstructible<It>()
			&& std::is_same<
				decltype(_dtl::test_post_inc<It>(nullptr)),
				It
			>::value;

		constexpr operator bool() const noexcept {
			return value;
		}
	};

	/**
	 * Compile time check if a type imlements the BidirectionalIterator concept.
	 *
	 * The tests performed are designed to make sure `It` conforms to the
	 * requirements described e.g.
	 * [here](http://en.cppreference.com/w/cpp/concept/BidirectionalIterator).
	 *
	 * \par Examples
	 *
	 * Examining a couple of types:
	 * \code
	 *   std::cout << std::boolalpha
	 *       << ftl::BidirectionalIterator<int>{}
	 *       << ", "
	 *       << ftl::BidirectionalIterator<std::vector<int>::iterator>{}
	 *       << ", "
	 *       << ftl::BidirectionalIterator<int*>{}
	 *       << ", "
	 *       << ftl::BidirectionalIterator<ftl::maybe<int>::iterator>{}
	 *       << std::endl;
	 * \endcode
	 * Output:
	 * \code
	 *   false, true, true, false
	 * \endcode
	 *
	 * \ingroup concepts_iterator
	 */
	template<typename It>
	struct BidirectionalIterator {
		static constexpr bool value =
			ForwardIterator<It>()
			&& std::is_same<
				decltype(_dtl::test_pre_dec<It>(nullptr)),
				It&
			>::value
			&& std::is_same<
				decltype(_dtl::test_post_dec<It>(nullptr)),
				It
			>::value;

		constexpr operator bool() const noexcept {
			return value;
		}
	};
}

#endif

