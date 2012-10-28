TEMPLATE  = app
LANGUAGE  = C++
CONFIG   += precompile_header

SOURCES += \
    source/tool_bar.cpp \
    source/settings.cpp \
    source/menu_bar.cpp \
    source/main_window_presenter.cpp \
    source/main_window.cpp \
    source/main.cpp \
    source/locations_list_box.cpp \
    precompiled/precompiled.cpp

INCLUDEPATH = ./include ./precompiled

HEADERS += \
    include/tool_bar.h \
    include/settings.h \
    include/menu_bar.h \
    include/main_window_presenter.h \
    include/main_window.h \
    include/locations_list_box.h \
    include/i_main_window_view.h \
    include/i_main_window_presenter.h \
    include/i_main_window_model.h \
    precompiled/precompiled.h

RESOURCES += \
    resource/main_window.qrc

PRECOMPILED_HEADER += precompiled/precompiled.h

CONFIG += core gui
