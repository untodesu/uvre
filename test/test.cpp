/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <exception>
#include <uvre/uvre.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <random>
#include <ctime>

static SDL_Window *window = nullptr;
static SDL_GLContext gl_context = nullptr;

static void uvreSwapInterval(int interval)
{
    SDL_GL_SetSwapInterval(interval);
}

static void urveSwapBuffers()
{
    SDL_GL_SwapWindow(window);
}

static void urveMakeCurrent()
{
    SDL_GL_MakeCurrent(window, gl_context);
}

static uvre::device_info::proc urveGetProcAddr(const char *procname)
{
    return reinterpret_cast<uvre::device_info::proc>(SDL_GL_GetProcAddress(procname));
}

static void urveOnMessage(const char *message)
{
    SDL_Log("%s", message);
}

static const char *vert_src =
    "layout(location = 0) in vec2 position;\n"
    "layout(location = 1) in vec2 texcoord;\n"
    "layout(location = 0) out vec2 fs_texcoord;\n"
    "out gl_PerVertex { vec4 gl_Position; };\n"
    "void main()\n"
    "{\n"
    "fs_texcoord = texcoord;\n"
    "gl_Position = vec4(position, 0.0, 1.0);\n"
    "}\n";

static const char *frag_src =
    "layout(location = 0) in vec2 texcoord;\n"
    "layout(location = 0) out vec4 target;\n"
    "layout(binding = 0) uniform sampler2D samp;\n"
    "void main()\n"
    "{\n"
    "target = texture(samp, texcoord) * vec4(texcoord, 1.0, 1.0);\n"
    "}\n";

int main(int argc, char **argv)
{
    if(SDL_Init(SDL_INIT_VIDEO))
        return 1;
    
    Uint32 windowflags = 0;

    uvre::api_info api_info;
    uvre::pollApiInfo(api_info);

    if(api_info.is_gl) {
        windowflags |= SDL_WINDOW_OPENGL;
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, api_info.gl.core_profile ? SDL_GL_CONTEXT_PROFILE_CORE : SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, api_info.gl.version_major);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, api_info.gl.version_minor);
    }

    window = SDL_CreateWindow("UVRE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, windowflags);
    if(!window)
        std::terminate();

    uvre::device_info device_info;
    device_info.onMessage = urveOnMessage;

    if(api_info.is_gl) {
        gl_context = SDL_GL_CreateContext(window);
        device_info.gl_swapInterval = uvreSwapInterval;
        device_info.gl_swapBuffers = urveSwapBuffers;
        device_info.gl_makeCurrent = urveMakeCurrent;
        device_info.gl_getProcAddr = urveGetProcAddr;
    }

    uvre::IRenderDevice *device = uvre::createDevice(device_info);
    if(!device)
        std::terminate();

    // My laptop heats up real bad after some time. WTF?
    device->vsync(true);

    uvre::ICommandList *commands = device->createCommandList();

    uvre::shader_info vert_info = {};
    vert_info.stage = uvre::shader_stage::VERTEX;
    vert_info.format = uvre::shader_format::SOURCE_GLSL;
    vert_info.code_size = 0;
    vert_info.code = vert_src;

    uvre::shader_info frag_info = {};
    frag_info.stage = uvre::shader_stage::FRAGMENT;
    frag_info.format = uvre::shader_format::SOURCE_GLSL;
    frag_info.code_size = 0;
    frag_info.code = frag_src;

    uvre::shader *vert = device->createShader(vert_info);
    uvre::shader *frag = device->createShader(frag_info);

    const uvre::index16 indices[3] = {
        0, 1, 2
    };

    using vec2_t = float[2];
    const vec2_t vertices[3 * 2] = {
        { -0.5f, -0.5f }, { 0.0f, 1.0f },
        {  0.0f,  0.5f }, { 0.5f, 0.0f },
        {  0.5f, -0.5f }, { 1.0f, 1.0f }
    };

    uvre::buffer_info ibo_info = {};
    ibo_info.type = uvre::buffer_type::INDEX_BUFFER;
    ibo_info.size = sizeof(indices);
    ibo_info.data = indices;

    uvre::buffer_info vbo_info = {};
    vbo_info.type = uvre::buffer_type::VERTEX_BUFFER;
    vbo_info.size = sizeof(vertices);
    vbo_info.data = vertices;

    uvre::buffer *ibo = device->createBuffer(ibo_info);
    uvre::buffer *vbo = device->createBuffer(vbo_info);

    uvre::vertex_attrib attribs[2];

    // Position
    attribs[0].id = 0;
    attribs[0].type = uvre::vertex_attrib_type::FLOAT32;
    attribs[0].count = 2;
    attribs[0].offset = sizeof(vec2_t) * 0;
    attribs[0].normalized = false;

    // Texcoord
    attribs[1].id = 1;
    attribs[1].type = uvre::vertex_attrib_type::FLOAT32;
    attribs[1].count = 2;
    attribs[1].offset = sizeof(vec2_t) * 1;
    attribs[1].normalized = false;

    const uvre::shader *shaders[2] = { vert, frag };

    uvre::pipeline_info pl_info = {};
    pl_info.blending.enabled = false;
    pl_info.depth_testing.enabled = false;
    pl_info.index = uvre::index_type::INDEX16;
    pl_info.primitive = uvre::primitive_type::TRIANGLES;
    pl_info.vertex_stride = sizeof(vec2_t) * 2;
    pl_info.num_vertex_attribs = 2;
    pl_info.vertex_attribs = attribs;
    pl_info.num_shaders = 2;
    pl_info.shaders = shaders;

    uvre::pipeline *pl = device->createPipeline(pl_info);

    uvre::texture_info color_info = {};
    color_info.type = uvre::texture_type::TEXTURE_2D;
    color_info.format = uvre::pixel_format::R16G16B16_UNORM;
    color_info.width = 200;
    color_info.height = 150;

    uvre::color_attachment attachment;
    attachment.id = 0;
    attachment.color = device->createTexture(color_info);

    uvre::rendertarget_info target_info = {};
    target_info.depth_attachment = nullptr;
    target_info.stencil_attachment = nullptr;
    target_info.num_color_attachments = 1;
    target_info.color_attachments = &attachment;

    uvre::rendertarget *target = device->createRenderTarget(target_info);

    uvre::sampler_info sampler_info = {};
    sampler_info.flags = uvre::SAMPLER_CLAMP_S | uvre::SAMPLER_CLAMP_T | uvre::SAMPLER_FILTER;

    uvre::sampler *sampler = device->createSampler(sampler_info);

    std::mt19937_64 mtdev(static_cast<uint_fast64_t>(std::time(nullptr)));
    std::uniform_int_distribution<std::uint8_t> id;
    std::uint8_t *tex_data = new std::uint8_t[64 * 64 * 3];
    for(std::size_t i = 0; i < 64 * 64 * 3; i++)
        tex_data[i] = id(mtdev);

    uvre::texture_info texture_info = {};
    texture_info.type = uvre::texture_type::TEXTURE_2D;
    texture_info.format = uvre::pixel_format::R8G8B8_UNORM;
    texture_info.width = 64;
    texture_info.height = 64;

    uvre::texture *texture = device->createTexture(texture_info);
    device->writeTexture2D(texture, 0, 0, 64, 64, uvre::pixel_format::R8G8B8_UINT, tex_data);
    delete[] tex_data;

    for(;;) {
        bool should_quit = false;
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                should_quit = true;
                break;
            }
        }

        if(should_quit)
            break;
        
        device->prepare();

        device->startRecording(commands);

        commands->bindRenderTarget(target);
        commands->setViewport(uvre::rect { 0, 0, 200, 150 });
        commands->clearColor3f(0.0f, 0.0f, 0.0f);
        commands->clear(uvre::RT_COLOR_BUFFER);
        commands->bindPipeline(pl);
        commands->bindIndexBuffer(ibo);
        commands->bindVertexBuffer(vbo);
        commands->bindSampler(sampler, 0);
        commands->bindTexture(texture, 0);
        commands->idraw(3, 1, 0, 0, 0);

        commands->bindRenderTarget(nullptr);
        commands->setViewport(uvre::rect { 0, 0, 800, 600 });
        commands->clearColor3f(0.0f, 0.0f, 0.25f);
        commands->clear(uvre::RT_COLOR_BUFFER);
        commands->copyRenderTarget(target, nullptr, uvre::rect { 0, 0, 200, 150 }, uvre::rect { 50, 50, 750, 550 }, uvre::RT_COLOR_BUFFER, false);

        device->submit(commands);

        device->present();
    }

    // We can omit cleaning up in this example
    // but I'd consider manual cleanup a good
    // practice so that you don't forget anything.
    uvre::destroyDevice(device);
    device = nullptr;

    if(api_info.is_gl) {
        SDL_GL_DeleteContext(gl_context);
        gl_context = nullptr;
    }

    SDL_DestroyWindow(window);
    window = nullptr;

    SDL_Quit();

    return 0;
}
