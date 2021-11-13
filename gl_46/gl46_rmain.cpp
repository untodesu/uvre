/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "gl46_private.hpp"

UVRE_API void uvre::pollImplApiInfo(uvre::ImplApiInfo &info)
{
    info.family = uvre::ImplApiFamily::OPENGL;
    info.gl.core_profile = true;
    info.gl.version_major = 4;
    info.gl.version_minor = 6;
}

UVRE_API uvre::IRenderDevice *uvre::createDevice(const uvre::DeviceInfo &info)
{
    if(!info.gl.setSwapInterval || !info.gl.swapBuffers || !info.gl.makeContextCurrent || !info.gl.getProcAddr)
        return nullptr;

    info.gl.makeContextCurrent(info.gl.user_data);
    if(gladLoadGLUserPtr(reinterpret_cast<GLADuserptrloadfunc>(info.gl.getProcAddr), info.gl.user_data))
        return new uvre::RenderDeviceImpl(info);

    // We are doomed!!!
    return nullptr;
}

UVRE_API void uvre::destroyDevice(uvre::IRenderDevice *device)
{
    delete device;
}
