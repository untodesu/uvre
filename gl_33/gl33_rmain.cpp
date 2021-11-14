/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "gl33_private.hpp"

static void pushErrorMessage(const uvre::DeviceCreateInfo &info, const char *text)
{
    if(info.onDebugMessage) {
        uvre::DebugMessageInfo msg = {};
        msg.level = uvre::DebugMessageLevel::ERROR;
        msg.text = text;
        info.onDebugMessage(msg);
    }
}

UVRE_API void uvre::pollImplInfo(uvre::ImplInfo &info)
{
    info.family = uvre::ImplFamily::OPENGL;
    info.gl.core_profile = true;
    info.gl.version_major = 3;
    info.gl.version_minor = 3;
}

UVRE_API uvre::IRenderDevice *uvre::createDevice(const uvre::DeviceCreateInfo &info)
{
    if(!info.gl.setSwapInterval || !info.gl.swapBuffers || !info.gl.makeContextCurrent || !info.gl.getProcAddr)
        return nullptr;

    info.gl.makeContextCurrent(info.gl.user_data);
    if(gladLoadGLUserPtr(reinterpret_cast<GLADuserptrloadfunc>(info.gl.getProcAddr), info.gl.user_data)) {
        if(!GLAD_GL_ARB_base_instance || !GLAD_GL_ARB_vertex_attrib_binding) {
            // Unfortunately we still require some extensions
            // to be present in order for UVRE to completely
            // work and be cool. And some of these extensions
            // are required to be here.
            if(!GLAD_GL_ARB_base_instance)
                pushErrorMessage(info, "GL_ARB_base_instance is required");
            if(!GLAD_GL_ARB_vertex_attrib_binding)
                pushErrorMessage(info, "GL_ARB_vertex_attrib_binding is required");
            return nullptr;
        }

        return new uvre::RenderDeviceImpl(info);
    }

    // We are doomed!!!
    return nullptr;
}

UVRE_API void uvre::destroyDevice(uvre::IRenderDevice *device)
{
    delete device;
}
