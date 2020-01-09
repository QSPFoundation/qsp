cmake_minimum_required(VERSION 3.15)
project(qsp)

if(NOT MSVC OR MSVC_TOOLSET_VERSION LESS 141)
    message(FATAL_ERROR "Only Visual Studio 2017 and later are supported")
endif()

option(BUILD_QSPGUI "Build qspgui application")
if(BUILD_QSPGUI)
    set(QSP_SOURCES
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
        qsp/onig/enc/utf16_be.c
        qsp/onig/enc/utf16_le.c
        qsp/onig/enc/utf32_be.c
        qsp/onig/enc/utf32_le.c
        qsp/bindings/default/default_callbacks.c
        qsp/bindings/default/default_control.c
        qsp/bindings/default/default_callbacks.c
        qsp/bindings/default/default_control.c
        qsp/bindings/flash/flash_callbacks.c
        qsp/bindings/flash/flash_platform.c
        qsp/bindings/flash/flash_control.c
        qsp/bindings/java/java_callbacks.c
        qsp/bindings/java/java_platform.c
        qsp/bindings/java/java_control.c
        qsp/bindings/android/android_callbacks.c
        qsp/bindings/android/android_platform.c
        qsp/bindings/android/android_control.c
        qsp/actions.h
        qsp/bindings/qsp.h
        qsp/callbacks.h
        qsp/codetools.h
        qsp/coding.h
        qsp/common.h
        qsp/declarations.h
        qsp/errors.h
        qsp/game.h
        qsp/locations.h
        qsp/mathops.h
        qsp/memwatch.h
        qsp/menu.h
        qsp/objects.h
        qsp/playlist.h
        qsp/regexp.h
        qsp/statements.h
        qsp/text.h
        qsp/time.h
        qsp/variables.h
        qsp/variant.h
        qsp/onig/config.h
        qsp/onig/oniguruma.h
        qsp/onig/regenc.h
        qsp/onig/regint.h
        qsp/onig/regparse.h
        qsp/onig/st.h
        qsp/bindings/bindings_config.h
        qsp/bindings/default/qsp_default.h
        qsp/bindings/flash/flash.h
        qsp/bindings/java/java.h
        qsp/bindings/android/android.h
    )
    add_library(qsp SHARED ${QSP_SOURCES})
    target_compile_definitions(qsp PRIVATE _CRT_SECURE_NO_WARNINGS _WIN _UNICODE EXPORT)

    find_package(wxWidgets REQUIRED base core adv aui html)
    include(${wxWidgets_USE_FILE})

    add_library(fmod SHARED IMPORTED)
    set(FMOD_DLL "${CMAKE_SOURCE_DIR}/players/classic/misc/win32/fmodex.dll")
    set(FMOD_LIBRARY "${CMAKE_SOURCE_DIR}/players/classic/qspgui/misc/win32/libs/fmodex_vc.lib")
    set_target_properties(fmod PROPERTIES IMPORTED_LOCATION ${FMOD_DLL} IMPORTED_IMPLIB ${FMOD_LIBRARY})

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
        players/classic/qspgui/src/animwin.h
        players/classic/qspgui/src/app.h
        players/classic/qspgui/src/callbacks_gui.h
        players/classic/qspgui/src/comtools.h
        players/classic/qspgui/src/fmod.h
        players/classic/qspgui/src/fmod_codec.h
        players/classic/qspgui/src/fmod_dsp.h
        players/classic/qspgui/src/fmod_memoryinfo.h
        players/classic/qspgui/src/frame.h
        players/classic/qspgui/src/imgcanvas.h
        players/classic/qspgui/src/initevent.h
        players/classic/qspgui/src/inputbox.h
        players/classic/qspgui/src/inputdlg.h
        players/classic/qspgui/src/listbox.h
        players/classic/qspgui/src/msgdlg.h
        players/classic/qspgui/src/textbox.h
        players/classic/qspgui/src/transhelper.h
        players/classic/qspgui/misc/win32/rsc/res.rc
    )
    add_executable(qspgui WIN32 ${QSPGUI_SOURCES})
    target_include_directories(qspgui PRIVATE qsp/bindings qsp/bindings/default)
    target_link_libraries(qspgui qsp fmod ${wxWidgets_LIBRARIES})
    configure_file("${CMAKE_SOURCE_DIR}/players/classic/qspgui/misc/icons/logo.ico" "${CMAKE_BINARY_DIR}/misc/icons/logo.ico" COPYONLY)
endif()

option(BUILD_TXT2GAM "Build txt2gam program")
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
    target_compile_definitions(txt2gam PRIVATE _CRT_SECURE_NO_WARNINGS _UNICODE)
endif()
