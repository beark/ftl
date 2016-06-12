include(CTest)

add_executable(throwing_copy_ctor1 throwing_copy_ctor1.cpp)
add_executable(throwing_copy_ctor2 throwing_copy_ctor2.cpp)

set_target_properties(
	throwing_copy_ctor1
	throwing_copy_ctor2
	PROPERTIES
		EXCLUDE_FROM_ALL TRUE
		EXCLUDE_FROM_DEFAULT_BUILD TRUE)

add_test(NAME ThrowingCopyCtor1
	COMMAND ${CMAKE_COMMAND} --build . --target throwing_copy_ctor1 --config $<CONFIGURATION>
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_test(NAME ThrowingCopyCtor2
	COMMAND ${CMAKE_COMMAND} --build . --target throwing_copy_ctor2 --config $<CONFIGURATION>
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

set_tests_properties(ThrowingCopyCtor1 PROPERTIES WILL_FAIL TRUE)
set_tests_properties(ThrowingCopyCtor2 PROPERTIES WILL_FAIL TRUE)
