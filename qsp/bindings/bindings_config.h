/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef QSP_BINDINGSCONFIG
    #define QSP_BINDINGSCONFIG

    /* Java binding */
    #ifdef _JAVA
        #define QSP_BINDING
        #define _JAVA_BINDING
        #include "java/qsp_java.h"
    #endif

    /* Place your bindings here */

    #ifndef QSP_BINDING
        #define QSP_BINDING
        #define _DEFAULT_BINDING
        #include "default/qsp_default.h"
    #endif

#endif
