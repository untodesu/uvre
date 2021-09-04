/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "gl_private.hpp"

static GLADapiproc getProcAddrWrap(void *arg, const char *procname)
{
    const uvre::device_info *info = reinterpret_cast<const uvre::device_info *>(arg);
    return reinterpret_cast<GLADapiproc>(info->gl_getProcAddr(procname));
}

UVRE_API void uvre::pollApiInfo(uvre::api_info &info)
{
    info.is_gl = true;
    info.gl.core_profile = true;
    info.gl.version_major = 4;
    info.gl.version_minor = 6;
}

UVRE_API uvre::IRenderDevice *uvre::createDevice(const uvre::device_info &info)
{
    if(!info.gl_swapInterval || !info.gl_swapBuffers || !info.gl_makeCurrent || !info.gl_getProcAddr)
        return nullptr;

    info.gl_makeCurrent();

    // Attempt #1: use C++ API
    GLADloadfunc loadfunc = reinterpret_cast<GLADloadfunc>(info.gl_getProcAddr.target<GLADloadfunc>());
    if(loadfunc && gladLoadGL(loadfunc))
        return new uvre::GLRenderDevice(info);

    // Attempt #2: wrap the C++ API in a C function
    uvre::device_info hack_info = info;
    GLADuserptrloadfunc loadfunc_u = reinterpret_cast<GLADuserptrloadfunc>(getProcAddrWrap);
    if(gladLoadGLUserPtr(loadfunc_u, reinterpret_cast<void *>(&hack_info)))
        return new uvre::GLRenderDevice(info);

    // We are doomed!!!    
    return nullptr;
}

UVRE_API void uvre::destroyDevice(uvre::IRenderDevice *device)
{
    delete device;
}
