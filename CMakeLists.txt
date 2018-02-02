cmake_minimum_required (VERSION 3.9.0)
add_definitions(
	-D_CRT_SECURE_NO_WARNINGS # disable warning about using C++ functions (M$ wants us to use their own non-conformant variants)
	-D_SCL_SECURE_NO_WARNINGS # disable more dumbstuff
	/wd4577 # disable warnings about `noexcept` when we've disabled exceptions (lets me slap noexcept into places that it should be)
	/wd4530 # disable warnings about `try{}catch{}` when we've disabled exceptions (VS includes them)
 	-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING # not so much a V$ problem
)

if (NOT TARGET whippet)
project (whippet)

# gmocktest
add_subdirectory(../ppc/googletest-release-1.8.0/ googletest-release-1.8.0/)

# pal
add_subdirectory(../pal/ pal/)

add_library(whippet
		build/.whippet-KICK.cpp
		src/main/cgc/hanoi.hpp
		src/main/cgc/whippet-component.cpp
		src/main/cgc/whippet-entity.cpp
		src/main/cgc/whippet-porcelain.cpp
		src/main/cgc/whippet-system.cpp
		src/main/cgc/whippet-universe.cpp
		src/main/cgc/whippet.hpp
		src/main/cgc/whippet.inline.hpp
		build/generated-src/tinMainTinHeader-cgc/tin-main.hpp
	)
	target_include_directories(whippet
		PUBLIC src/main/cgc/
		PUBLIC build/generated-src/tinMainTinHeader-cgc/
	)
	target_link_libraries(
		whippet
			pal
	)
	add_dependencies(whippet
		pal
	)

add_executable(whippet-test
		src/test/cgc/whippet-test.cpp
		build/generated-src/tinTestTinHeader-cgc/tin-test.hpp
	)
	set_target_properties(whippet-test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	target_include_directories(whippet-test
		PUBLIC src/test/cgc/
		PUBLIC build/generated-src/tinTestTinHeader-cgc/
	)
	target_link_libraries(
		whippet-test
			gmocktest
			whippet
	)
	add_dependencies(whippet-test
		gmocktest
		whippet
	)

# sub-projects
endif (NOT TARGET whippet)
