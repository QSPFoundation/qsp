cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(qsp VERSION 5.9.6)

set(QSP_GAMEMIN_VER "5.9.6")

option(BUILD_JAVA "Build with JVM bindings" OFF)
option(BUILD_WASM "Build WebAssembly module (requires the Emscripten toolchain)" OFF)
option(BUILD_STATIC "Build a static library instead of a shared one" OFF)
option(USE_INSTALLED_ONIGURUMA "Use system-wide oniguruma library instead of bundling it" OFF)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(FetchContent)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_STANDARD 90)

set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-char-subscripts -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
else()
	message(FATAL_ERROR "Compiler is not supported")
endif()

if (USE_INSTALLED_ONIGURUMA)
	find_package(oniguruma REQUIRED)
else()
	FetchContent_Declare(oniguruma-custom
		GIT_REPOSITORY https://github.com/kkos/oniguruma.git
		GIT_TAG master
		GIT_SHALLOW true # it works only with branch names and tags
		GIT_PROGRESS true
		SUBBUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/oniguruma-download"
		SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/oniguruma-src"
		BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/oniguruma-build"
	)
	# Configure the CRNL handling
	add_compile_definitions(USE_CRNL_AS_LINE_TERMINATOR=1)
	# Disable oniguruma features we don't need
	set(INSTALL_DOCUMENTATION OFF)
	set(INSTALL_EXAMPLES OFF)
	set(BUILD_TEST OFF)
	set(BUILD_SHARED_LIBS OFF)
	set(MSVC_STATIC_RUNTIME ON)

	FetchContent_MakeAvailable(oniguruma-custom)
	add_library(oniguruma::onig ALIAS onig)
endif()

configure_file(qsp_config.h.cmakein qsp_config.h @ONLY)

set(QSP_SOURCES
	qsp/bindings/bindings_config.h
	qsp/bindings/qsp.h
	qsp/actions.c qsp/actions.h
	qsp/callbacks.c qsp/callbacks.h
	qsp/codetools.c qsp/codetools.h
	qsp/coding.c qsp/coding.h
	qsp/common.c qsp/common.h
	qsp/errors.c qsp/errors.h
	qsp/game.c qsp/game.h
	qsp/locations.c qsp/locations.h
	qsp/mathops.c qsp/mathops.h
	qsp/memory.c qsp/memory.h
	qsp/memwatch.c qsp/memwatch.h
	qsp/menu.c qsp/menu.h
	qsp/objects.c qsp/objects.h
	qsp/playlist.c qsp/playlist.h
	qsp/regexp.c qsp/regexp.h
	qsp/statements.c qsp/statements.h
	qsp/text.c qsp/text.h
	qsp/time.c qsp/time.h
	qsp/towlower.c
	qsp/towupper.c
	qsp/tuples.c qsp/tuples.h
	qsp/variables.c qsp/variables.h
	qsp/variant.c qsp/variant.h
)
if (BUILD_STATIC)
	add_library(qsp STATIC ${QSP_SOURCES})
else()
	add_library(qsp SHARED ${QSP_SOURCES})
endif()
target_compile_definitions(qsp PUBLIC _UNICODE)
if (WIN32)
	target_compile_definitions(qsp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_link_libraries(qsp PRIVATE oniguruma::onig)

if (BUILD_JAVA)
	add_subdirectory(qsp/bindings/java)
elseif (BUILD_WASM)
	add_subdirectory(qsp/bindings/wasm)
else()
	add_subdirectory(qsp/bindings/default)
endif()

set(QSP_INSTALL_TARGETS qsp)
if (TARGET onig)
	get_target_property(ONIG_IS_IMPORTED onig IMPORTED)
	if (NOT ONIG_IS_IMPORTED)
		list(APPEND QSP_INSTALL_TARGETS onig)
	endif()
endif()

install(TARGETS ${QSP_INSTALL_TARGETS} EXPORT QspTargets)
export(TARGETS ${QSP_INSTALL_TARGETS} NAMESPACE Qsp:: FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/Qsp/QspTargets.cmake")

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/cmake/Qsp/QspConfigVersion.cmake" COMPATIBILITY AnyNewerVersion)
configure_package_config_file(QspConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/cmake/Qsp/QspConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Qsp"
)

install(EXPORT QspTargets FILE QspTargets.cmake NAMESPACE Qsp:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Qsp")
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
