cmake_minimum_required(VERSION 2.8)

project(FTL)

add_subdirectory(should_fail)

include_directories("../include")

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")

if(CMAKE_COMPILER_IS_GNUCXX)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3 -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

	execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

	if(GCC_VERSION VERSION_LESS 5)
		message(WARNING "${PROJECT_NAME} only supports g++ 5 or later, but ${GCC_VERSION} was detected")
	endif()

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb3 -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DNDEBUG")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

	execute_process(
		COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE CLANG_VERSION_STRING)
	string(
		REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION ${CLANG_VERSION_STRING})

	if(CLANG_VERSION VERSION_LESS 3.8)
		message(WARNING "${PROJECT_NAME} only supports clang 3.8 or later, but ${CLANG_VERSION} was detected")
	endif()

elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

	message(WARNING "${PROJECT_NAME} does not support Visual Studio, expect errors")

else()

	message("Warning: unknown/unsupported compiler, things may go wrong")

endif()

set(SOURCES 
	static_tests.cpp
	sum_type_tests.cpp
	maybe_tests.cpp
	either_tests.cpp
	functional_tests.cpp
	concept_tests.cpp
	eithert_tests.cpp
	future_tests.cpp
	fwdlist_tests.cpp
	lazy_tests.cpp
	lazyt_tests.cpp
	list_tests.cpp
	map_tests.cpp
	maybet_tests.cpp
	memory_tests.cpp
	ord_tests.cpp
	prelude_tests.cpp
	set_tests.cpp
	string_tests.cpp
	tuple_tests.cpp
	unordered_map_tests.cpp
	vector_tests.cpp
	main.cpp
)

add_executable(ftl_tests ${SOURCES})

