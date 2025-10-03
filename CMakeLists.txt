cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(qsp VERSION 5.9.4)

set(QSP_GAMEMIN_VER "5.9.4")

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_C_STANDARD 90)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	set(PROJECT_COMPILER_FLAGS "-Wall")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-char-subscripts -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
	set(PROJECT_COMPILER_FLAGS "/W4")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "AppleClang")
	set(PROJECT_COMPILER_FLAGS "")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -DNDEBUG")
else()
	message(FATAL_ERROR "Compiler is not supported")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

option(USE_INSTALLED_ONIGURUMA "Use system-wide oniguruma library instead of bundling it" OFF)

if (USE_INSTALLED_ONIGURUMA)
	find_package(oniguruma REQUIRED)
else()
	# Download and unpack oniguruma at configure time
	configure_file(CMakeLists-oniguruma.txt.in "${CMAKE_CURRENT_BINARY_DIR}/oniguruma-download/CMakeLists.txt")
	execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" "-DCMAKE_MAKE_PROGRAM:FILEPATH=${CMAKE_MAKE_PROGRAM}" .
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/oniguruma-download"
	)
	execute_process(COMMAND "${CMAKE_COMMAND}" --build .
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/oniguruma-download"
	)

	# Disable oniguruma features we don't need
	set(INSTALL_DOCUMENTATION OFF)
	set(INSTALL_EXAMPLES OFF)
	set(BUILD_TEST OFF)
	set(BUILD_SHARED_LIBS OFF)
	set(MSVC_STATIC_RUNTIME ON)

	# For Windows: Prevent overriding the parent project's compiler/linker settings
	set(oniguruma_force_shared_crt ON CACHE BOOL "" FORCE)

	# Configure the CRNL handling
	add_compile_definitions(USE_CRNL_AS_LINE_TERMINATOR=1)

	# Add oniguruma directly to our build
	set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
	add_subdirectory("${CMAKE_CURRENT_BINARY_DIR}/oniguruma-src"
					 "${CMAKE_CURRENT_BINARY_DIR}/oniguruma-build"
	)
	add_library(oniguruma::onig ALIAS onig)
endif()

configure_file(qsp_config.h.cmakein qsp_config.h @ONLY)

set(QSP_SOURCES
	qsp/bindings/bindings_config.h
	qsp/bindings/default/default_callbacks.c
	qsp/bindings/default/default_control.c
	qsp/bindings/default/qsp_default.h
	qsp/bindings/java/java_callbacks.c
	qsp/bindings/java/java_control.c
	qsp/bindings/java/qsp_java.h
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
add_library(qsp SHARED ${QSP_SOURCES})
target_compile_definitions(qsp PUBLIC _UNICODE)
if (WIN32)
	target_compile_definitions(qsp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_compile_options(qsp PRIVATE ${PROJECT_COMPILER_FLAGS})
target_link_options(qsp PRIVATE ${PROJECT_LINKER_FLAGS})
target_link_libraries(qsp PRIVATE oniguruma::onig)

if (BUILD_JAVA)
	set_property(TARGET qsp PROPERTY C_STANDARD 11)
	target_compile_definitions(qsp PRIVATE _JAVA)
	if (NOT ANDROID)
		find_package(JNI)
		target_include_directories(qsp PRIVATE ${JNI_INCLUDE_DIRS})
		target_link_libraries(qsp PRIVATE ${JNI_LIBRARIES})
	endif()
endif()

target_include_directories(qsp
	INTERFACE
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/qsp/bindings/default>"
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qsp>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qsp/default>"
)

generate_export_header(qsp
	BASE_NAME Qsp
	EXPORT_MACRO_NAME QSP_EXTERN
)

set(QSP_INSTALL_TARGETS qsp)
if (TARGET onig)
	get_target_property(ONIG_IS_IMPORTED onig IMPORTED)
	if (NOT ONIG_IS_IMPORTED)
		list(APPEND QSP_INSTALL_TARGETS onig)
	endif()
endif()

install(TARGETS ${QSP_INSTALL_TARGETS} EXPORT QspTargets)
export(TARGETS ${QSP_INSTALL_TARGETS} NAMESPACE Qsp:: FILE "${CMAKE_CURRENT_BINARY_DIR}/QspTargets.cmake")

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/cmake/Qsp/QspConfigVersion.cmake" COMPATIBILITY AnyNewerVersion)
configure_package_config_file(QspConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/cmake/Qsp/QspConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Qsp"
)

install(EXPORT QspTargets NAMESPACE Qsp:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Qsp")

install(
	DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

install(FILES
		qsp/bindings/qsp.h
		qsp/bindings/bindings_config.h
		"${CMAKE_CURRENT_BINARY_DIR}/qsp_export.h"
	DESTINATION
		"${CMAKE_INSTALL_INCLUDEDIR}/qsp"
)

install(FILES
		qsp/bindings/default/qsp_default.h
	DESTINATION
		"${CMAKE_INSTALL_INCLUDEDIR}/qsp/default"
)
