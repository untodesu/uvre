/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#if defined(_MSC_VER)
#    if defined(UVRE_SHARED)
#        define UVRE_API __declspec(dllexport)
#    elif defined(UVRE_STATIC)
#        define UVRE_API
#    else
#        define UVRE_API __declspec(dllimport)
#    endif
#elif defined(__GNUC__)
#    if defined(UVRE_SHARED)
#        define UVRE_API __attribute__((visibility("default")))
#    else
#        define UVRE_API
#    endif
#else
#    warning Unknown compiler!
#    define UVRE_API
#endif
