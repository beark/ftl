cmake_minimum_required(VERSION 2.8)

project(FTL)

include_directories("../include")

if(CMAKE_COMPILER_IS_GNUCXX)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

	execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	if(GCC_VERSION VERSION_GREATER 7.1)
		message("-- g++ > 7.1 detected, enabling C++17 features")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -DFTL_CPP14")
	else()
		message(FATAL_ERROR "${PROJECT_NAME} requires g++ 7.1 or later")
	endif()

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -pedantic -Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -lc++")

	execute_process(
		COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE CLANG_VERSION_STRING)
	string(
		REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION ${CLANG_VERSION_STRING})

	if(CLANG_VERSION VERSION_GREATER 5.0)
		message("-- Clang > 5.0 detected, enabling C++17 features")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -DFTL_CPP14")
	else()
		message(FATAL_ERROR "${PROJECT_NAME} requires clang 5.0 or later")
	endif()

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)

    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.16")
        message(FATAL_ERROR "You'll need Visual Studio 2017 Version 15.9.4+ to build FTL")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
    endif()

else()

	message("Warning: unknown/unsupported compiler, things may go wrong")

endif()

set(SOURCES 
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

