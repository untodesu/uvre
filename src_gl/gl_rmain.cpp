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
    return reinterpret_cast<GLADapiproc>(info->gl.getProcAddr(procname));
}

UVRE_API void uvre::pollBackendInfo(uvre::backend_info &info)
{
    info.family = uvre::backend_family::OPENGL;
    info.gl.core_profile = true;
    info.gl.version_major = 4;
    info.gl.version_minor = 6;
}

UVRE_API uvre::IRenderDevice *uvre::createDevice(const uvre::device_info &info)
{
    if(!info.gl.setSwapInterval || !info.gl.swapBuffers || !info.gl.makeContextCurrent || !info.gl.getProcAddr)
        return nullptr;

    info.gl.makeContextCurrent();

    uvre::device_info hack_info = info;
    GLADuserptrloadfunc loadfunc_u = reinterpret_cast<GLADuserptrloadfunc>(getProcAddrWrap);
    if(gladLoadGLUserPtr(loadfunc_u, &hack_info))
        return new uvre::GLRenderDevice(info);

    // We are doomed!!!
    return nullptr;
}

UVRE_API void uvre::destroyDevice(uvre::IRenderDevice *device)
{
    delete device;
}
