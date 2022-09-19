cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(qsp VERSION 5.8.0)

set(QSP_GAMEMIN_VER "5.8.0")

if(WIN32)
	if(NOT MSVC OR MSVC_TOOLSET_VERSION LESS 110)
		message(FATAL_ERROR "Only Visual Studio 2012 and later is supported for Windows targets")
	endif()
elseif(APPLE OR NOT UNIX)
	message(FATAL_ERROR "Only Windows and Linux targets are supported")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(CMAKE_INCLUDE_CURRENT_DIR True)

include(CMakePackageConfigHelpers)
include(GenerateExportHeader)
include(GNUInstallDirs)

if(CMAKE_COMPILER_IS_GNUCC)
	set(PROJECT_COMPILER_FLAGS "-Wall")
	set(PROJECT_LINKER_FLAGS "-no-pie")
	set(CMAKE_CXX_FLAGS "-Wall -Wextra")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG -DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()
if(MSVC)
	set(PROJECT_COMPILER_FLAGS "/W4")
	set(PROJECT_LINKER_FLAGS "")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /DDEBUG")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

option(USE_INSTALLED_ONIGURUMA "Use system-wide oniguruma library instead of bundling it" OFF)

if(USE_INSTALLED_ONIGURUMA)
	find_package(oniguruma REQUIRED)
else()
	# Download and unpack oniguruma at configure time
	configure_file(CMakeLists-oniguruma.txt.in "${CMAKE_BINARY_DIR}/oniguruma-download/CMakeLists.txt")
	execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/oniguruma-download"
	)
	execute_process(COMMAND "${CMAKE_COMMAND}" --build .
			WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/oniguruma-download"
	)
	# Prevent oniguruma from overriding our compiler/linker options
	# when building with Visual Studio
	set(oniguruma_force_shared_crt ON CACHE BOOL "" FORCE)

	# Configure the CRNL handling
	add_compile_definitions(USE_CRNL_AS_LINE_TERMINATOR)

	# Add oniguruma directly to our build
	add_subdirectory("${CMAKE_BINARY_DIR}/oniguruma-src"
					 "${CMAKE_BINARY_DIR}/oniguruma-build"
	)
endif()

configure_file(qsp_config.h.cmakein qsp_config.h @ONLY)

set(QSP_SOURCES
	qsp/bindings/default/default_callbacks.c
	qsp/bindings/default/default_control.c
	qsp/actions.c
	qsp/callbacks.c
	qsp/codetools.c
	qsp/coding.c
	qsp/common.c
	qsp/errors.c
	qsp/game.c
	qsp/locations.c
	qsp/mathops.c
	qsp/memwatch.c
	qsp/menu.c
	qsp/objects.c
	qsp/playlist.c
	qsp/regexp.c
	qsp/statements.c
	qsp/text.c
	qsp/time.c
	qsp/towlower.c
	qsp/towupper.c
	qsp/variables.c
	qsp/variant.c
)
add_library(qsp SHARED ${QSP_SOURCES})
target_compile_definitions(qsp PUBLIC _UNICODE)
if(WIN32)
	target_compile_definitions(qsp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
target_compile_options(qsp PRIVATE ${PROJECT_COMPILER_FLAGS})
target_link_options(qsp PRIVATE ${PROJECT_LINKER_FLAGS})
if(USE_INSTALLED_ONIGURUMA)
	target_link_libraries(qsp PRIVATE oniguruma::onig)
else()
	target_link_libraries(qsp PRIVATE onig)
	install(TARGETS onig)
endif()

target_include_directories(qsp
	INTERFACE
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qsp>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qsp/default>"
)

install(TARGETS qsp EXPORT QspTargets DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime)

generate_export_header(qsp
  BASE_NAME Qsp
  EXPORT_MACRO_NAME QSP_EXTERN
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/QspConfigVersion.cmake COMPATIBILITY AnyNewerVersion)
configure_package_config_file(QspConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/QspConfig.cmake
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Qsp"
)

export(TARGETS qsp NAMESPACE Qsp:: FILE "${PROJECT_BINARY_DIR}/QspTargets.cmake")
install(EXPORT QspTargets NAMESPACE Qsp:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Qsp")
install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/QspConfigVersion.cmake
		${CMAKE_CURRENT_BINARY_DIR}/QspConfig.cmake
	DESTINATION
		"${CMAKE_INSTALL_LIBDIR}/cmake/Qsp"
)

install(FILES
		${CMAKE_SOURCE_DIR}/cmake/Modules/Findoniguruma.cmake
	DESTINATION
		"${CMAKE_INSTALL_DATAROOTDIR}/cmake/Modules"
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

option(BUILD_QSPGUI "Build qspgui application")
if(BUILD_QSPGUI)
	if(USE_INSTALLED_WX)
		find_package(wxWidgets REQUIRED base core adv aui html)
		include(${wxWidgets_USE_FILE})
	else()
		# Download and unpack wxWidgets at configure time
		configure_file(CMakeLists-wxWidgets.txt.in "${CMAKE_BINARY_DIR}/wxWidgets-download/CMakeLists.txt")
		execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-download"
		)
		execute_process(COMMAND "${CMAKE_COMMAND}" --build .
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-download"
		)
		# Patch wxWidgets
		execute_process(COMMAND git checkout -- .
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-src"
		)
		execute_process(COMMAND git apply --ignore-whitespace --reject "${CMAKE_SOURCE_DIR}/players/classic/build_wx/wxPatch.diff"
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-src"
		)

		# Disable wxWidgets features we don't need
		set(wxUSE_LIBTIFF OFF)
		set(wxUSE_REGEX OFF)
		set(wxUSE_SECRETSTORE OFF)
		set(wxUSE_LIBSDL OFF)
		set(wxUSE_LIBMSPACK OFF)
		set(wxUSE_GSTREAMER OFF)
		set(wxUSE_OPENGL OFF)
		set(wxUSE_RICHTEXT OFF)
		set(wxUSE_RIBBON OFF)
		set(wxUSE_MEDIACTRL OFF)
		set(wxUSE_PROPGRID OFF)
		set(wxUSE_XRC OFF)
		set(wxUSE_DEBUGREPORT OFF)
		set(wxUSE_SOCKETS OFF)
		set(wxUSE_PROTOCOL OFF)
		set(wxUSE_URL OFF)
		set(wxUSE_FS_INET OFF)

		# Prevent wxWidgets from overriding our compiler/linker options
		# when building with Visual Studio
		set(wxWidgets_force_shared_crt ON CACHE BOOL "" FORCE)

		# Add wxWidgets directly to our build
		add_subdirectory("${CMAKE_BINARY_DIR}/wxWidgets-src"
						 "${CMAKE_BINARY_DIR}/wxWidgets-build"
		)
	endif()

	add_library(fmod SHARED IMPORTED)
	if(WIN32)
		set(FMOD_DLL "${CMAKE_SOURCE_DIR}/players/classic/misc/win32/fmodex.dll")
		set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/players/classic/misc/win32/fmodex_vc.lib")
		set_target_properties(fmod PROPERTIES IMPORTED_LOCATION ${FMOD_DLL} IMPORTED_IMPLIB ${FMOD_LIBRARY})
	else()
		set(TARGET_ARCH "x86_64" CACHE STRING "Target architecture (x86;x86_64)")
		if("${TARGET_ARCH}" STREQUAL "x86")
			set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/players/classic/misc/linux_${TARGET_ARCH}/libfmodex.so")
		else()
			set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/players/classic/misc/linux_${TARGET_ARCH}/libfmodex64.so")
		endif()
		set_target_properties(fmod PROPERTIES
			IMPORTED_NO_SONAME TRUE
			IMPORTED_LOCATION ${FMOD_LIBRARY})
	endif()

	set(QSPGUI_SOURCES
		players/classic/qspgui/src/animwin.cpp
		players/classic/qspgui/src/app.cpp
		players/classic/qspgui/src/callbacks_gui.cpp
		players/classic/qspgui/src/comtools.cpp
		players/classic/qspgui/src/frame.cpp
		players/classic/qspgui/src/imgcanvas.cpp
		players/classic/qspgui/src/initevent.cpp
		players/classic/qspgui/src/inputbox.cpp
		players/classic/qspgui/src/inputdlg.cpp
		players/classic/qspgui/src/listbox.cpp
		players/classic/qspgui/src/msgdlg.cpp
		players/classic/qspgui/src/textbox.cpp
		players/classic/qspgui/src/transhelper.cpp
	)
	if(WIN32)
		list(APPEND QSPGUI_SOURCES players/classic/qspgui/src/rsc/res.rc)
		configure_file("${CMAKE_SOURCE_DIR}/players/classic/misc/common/icons/logo.ico" "${CMAKE_BINARY_DIR}/misc/icons/logo.ico" COPYONLY)
		add_executable(qspgui WIN32 ${QSPGUI_SOURCES})
		target_compile_definitions(qspgui PRIVATE _CRT_SECURE_NO_WARNINGS)
		set(RESOURCES_DIR "${CMAKE_INSTALL_PREFIX}/bin")
		install(FILES "${FMOD_DLL}" TYPE BIN)
	else()
		add_executable(qspgui ${QSPGUI_SOURCES})
		set(RESOURCES_DIR "${CMAKE_INSTALL_PREFIX}/share/qspgui")
		install(FILES "${FMOD_LIBRARY}" TYPE LIB)
	endif()
	target_compile_definitions(qspgui PRIVATE WXUSINGDLL _UNICODE)
	target_include_directories(qspgui PRIVATE qsp/bindings qsp/bindings/default)
	target_compile_options(qspgui PRIVATE ${PROJECT_COMPILER_FLAGS})
	target_link_options(qspgui PRIVATE ${PROJECT_LINKER_FLAGS})
	target_link_libraries(qspgui PRIVATE qsp fmod wxbase wxcore wxadv wxaui wxhtml)
	install(TARGETS qspgui qsp wxbase wxcore wxadv wxaui wxhtml)

	install(DIRECTORY "${CMAKE_SOURCE_DIR}/players/classic/misc/common/langs" DESTINATION "${RESOURCES_DIR}")
	install(DIRECTORY "${CMAKE_SOURCE_DIR}/players/classic/misc/common/sound" DESTINATION "${RESOURCES_DIR}")
endif()
