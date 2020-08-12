cmake_minimum_required(VERSION 3.13)
project(qsp)

if(WIN32)
	if(NOT MSVC OR MSVC_TOOLSET_VERSION LESS 110)
		message(FATAL_ERROR "Only Visual Studio 2012 and later is supported for Windows targets")
	endif()
elseif(APPLE OR NOT UNIX)
	message(FATAL_ERROR "Only Windows and Linux targets are supported")
endif()

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
	qsp/onig/regcomp.c
	qsp/onig/regenc.c
	qsp/onig/regerror.c
	qsp/onig/regexec.c
	qsp/onig/regparse.c
	qsp/onig/regsyntax.c
	qsp/onig/regtrav.c
	qsp/onig/regversion.c
	qsp/onig/st.c
	qsp/onig/enc/ascii.c
	qsp/onig/enc/cp1251.c
	qsp/onig/enc/koi8_r.c
	qsp/onig/enc/unicode.c
	qsp/onig/enc/utf16_le.c
	qsp/onig/enc/utf16_be.c
	qsp/onig/enc/utf32_le.c
	qsp/onig/enc/utf32_be.c
)
add_library(qsp SHARED ${QSP_SOURCES})
target_compile_definitions(qsp PRIVATE _UNICODE EXPORT)
if(WIN32)
	target_compile_definitions(qsp PRIVATE _CRT_SECURE_NO_WARNINGS _WIN)
endif()

option(BUILD_QSPGUI "Build qspgui application")
if(BUILD_QSPGUI)
	if(DEFAULT_WX)
		find_package(wxWidgets REQUIRED base core adv aui html)
		include(${wxWidgets_USE_FILE})
	else()
		# Download and unpack wxWidgets at configure time
		configure_file(CMakeLists.txt.in wxWidgets-download/CMakeLists.txt)
		execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-download"
		)
		execute_process(COMMAND "${CMAKE_COMMAND}" --build .
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-download"
		)
		# Patch wxWidgets
		execute_process(COMMAND git apply "${CMAKE_SOURCE_DIR}/players/classic/build_wx/wxPatch.diff"
				WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/wxWidgets-src"
		)

		# Disable wxWidgets features we don't need
		set(wxUSE_LIBTIFF OFF)

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
		set(FMOD_DLL "${CMAKE_SOURCE_DIR}/players/classic/misc/win32/fmodexp.dll")
		set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/players/classic/qspgui/misc/win32/libs/fmodex_vc.lib")
		set_target_properties(fmod PROPERTIES IMPORTED_LOCATION ${FMOD_DLL} IMPORTED_IMPLIB ${FMOD_LIBRARY})
	else()
		set(TARGET_ARCH "x86_64" CACHE STRING "Target architecture (x86;x86_64)")
		if("${TARGET_ARCH}" STREQUAL "x86")
			set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/players/classic/misc/linux_${TARGET_ARCH}/libfmodex.so")
		else()
			set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/players/classic/misc/linux_${TARGET_ARCH}/libfmodex64.so")
		endif()
		set_target_properties(fmod PROPERTIES IMPORTED_LOCATION ${FMOD_LIBRARY})
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
		list(APPEND QSPGUI_SOURCES players/classic/qspgui/misc/win32/rsc/res.rc)
		configure_file("${CMAKE_SOURCE_DIR}/players/classic/qspgui/misc/icons/logo.ico" "${CMAKE_BINARY_DIR}/misc/icons/logo.ico" COPYONLY)
		add_executable(qspgui WIN32 ${QSPGUI_SOURCES})
		target_compile_definitions(qspgui PRIVATE WXUSINGDLL _CRT_SECURE_NO_WARNINGS)
	else()
		add_executable(qspgui ${QSPGUI_SOURCES})
	endif()
	target_compile_definitions(qspgui PRIVATE _UNICODE)
	target_include_directories(qspgui PRIVATE qsp/bindings qsp/bindings/default)
	target_link_libraries(qspgui qsp fmod wxbase wxcore wxadv wxaui wxhtml)
endif()

option(BUILD_TXT2GAM "Build txt2gam tool")
if(BUILD_TXT2GAM)
	set(TXT2GAM_SOURCES
		txt2gam/src/coding.c
		txt2gam/src/locations.c
		txt2gam/src/main.c
		txt2gam/src/memwatch.c
		txt2gam/src/text.c
		txt2gam/src/coding.h
		txt2gam/src/declarations.h
		txt2gam/src/locations.h
		txt2gam/src/main.h
		txt2gam/src/memwatch.h
		txt2gam/src/text.h
	)
	add_executable(txt2gam ${TXT2GAM_SOURCES})
	target_compile_definitions(txt2gam PRIVATE _UNICODE)
	if(WIN32)
		target_compile_definitions(txt2gam PRIVATE _CRT_SECURE_NO_WARNINGS)
	endif()
endif()
