/* Copyright (C) 2001-2025 Val Argunov (byte AT qsp DOT org) */
/*
* This library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
