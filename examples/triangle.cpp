/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <GLFW/glfw3.h>
#include <exception>
#include <iostream>
#include <uvre/uvre.hpp>

using vec2_t = float[2];
struct vertex final {
    vec2_t position;
    vec2_t texcoord;
};

// Vertex shader source
static const char *vert_source = R"(
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
layout(location = 0) out vec2 fs_texcoord;
out gl_PerVertex { vec4 gl_Position; };
void main()
{
    fs_texcoord = texcoord;
    gl_Position = vec4(position, 0.0, 1.0);
})";

// Fragment shader source
static const char *frag_source = R"(
layout(location = 0) in vec2 texcoord;
layout(location = 0) out vec4 fs_target;
void main()
{
    fs_target = vec4(texcoord, 1.0, 1.0);
})";

// GLFW error callback
static void onGlfwError(int code, const char *message)
{
    std::cerr << message << std::endl;
}

int main()
{
    // Initialize GLFW
    glfwSetErrorCallback(onGlfwError);
    if(!glfwInit())
        std::terminate();

    // Since UVRE is windowing API-agnostic,
    // some information must be passed back
    // to the windowing API in order for it
    // to be correctly set up for UVRE.
    uvre::api_info api_info;
    uvre::pollApiInfo(api_info);

    // Do not require any client API by default
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Non-resizable.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // If the backend API is OpenGL-ish
    if(api_info.is_gl) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_OPENGL_PROFILE, api_info.gl.core_profile ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, api_info.gl.version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, api_info.gl.version_minor);

#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    }

    constexpr const int WINDOW_WIDTH = 800;
    constexpr const int WINDOW_HEIGHT = 600;

    // Open a new window
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "UVRE - Triangle", nullptr, nullptr);
    if(!window)
        std::terminate();

    // Now the windowing API also needs to pass some
    // data to the library before creating a rendering
    // device. Usually this data is API-specific callbacks.
    uvre::device_info device_info = {};

    // OpenGL-specific callbacks
    if(api_info.is_gl) {
        device_info.gl_swapInterval = [&](int interval) { glfwSwapInterval(interval); };
        device_info.gl_swapBuffers = [&]() { glfwSwapBuffers(window); };
        device_info.gl_makeCurrent = [&]() { glfwMakeContextCurrent(window); };
        device_info.gl_getProcAddr = [&](const char *procname) { return reinterpret_cast<uvre::device_info::proc>(glfwGetProcAddress(procname)); };
    }

    // Message callback
    device_info.onMessage = [&](const char *message) { std::cerr << message << std::endl; };

    // Now we create the rendering device.
    // Rendering device is an object that works
    // with other objects: creates them, destroys
    // them, operates with their internal data, etc.
    uvre::IRenderDevice *device = uvre::createDevice(device_info);
    if(!device)
        std::terminate();

    // A command list is basically an object that
    // records the commands related to drawing. Command
    // lists can be immediate (OpenGL) or deferred (Vulkan).
    // Each command list must be submitted after recording.
    // The create function is guaranteed to return a non-null value.
    uvre::ICommandList *commands = device->createCommandList();

    // Vertex shader description
    uvre::shader_info vert_info = {};
    vert_info.stage = uvre::shader_stage::VERTEX;
    vert_info.format = uvre::shader_format::SOURCE_GLSL;
    vert_info.code = vert_source;

    // Fragment shader description
    uvre::shader_info frag_info = {};
    frag_info.stage = uvre::shader_stage::FRAGMENT;
    frag_info.format = uvre::shader_format::SOURCE_GLSL;
    frag_info.code = frag_source;

    // Create shaders.
    // We assume that the GLSL code is valid.
    uvre::shader *shaders[2];
    shaders[0] = device->createShader(vert_info);
    shaders[1] = device->createShader(frag_info);

    // Vertex format description
    uvre::vertex_attrib attributes[2];
    attributes[0] = uvre::vertex_attrib { 0, uvre::vertex_attrib_type::FLOAT32, 2, offsetof(vertex, position), false };
    attributes[1] = uvre::vertex_attrib { 1, uvre::vertex_attrib_type::FLOAT32, 2, offsetof(vertex, texcoord), false };

    // Pipeline description
    uvre::pipeline_info pipeline_info = {};
    pipeline_info.blending.enabled = false;
    pipeline_info.depth_testing.enabled = false;
    pipeline_info.index = uvre::index_type::INDEX16;
    pipeline_info.primitive = uvre::primitive_type::TRIANGLES;
    pipeline_info.vertex_stride = sizeof(vertex);
    pipeline_info.num_vertex_attribs = 2;
    pipeline_info.vertex_attribs = attributes;
    pipeline_info.num_shaders = 2;
    pipeline_info.shaders = shaders;

    // Create pipeline
    uvre::pipeline *pipeline = device->createPipeline(pipeline_info);

    // Vertex buffer data
    const vertex vertices[3] = {
        vertex { { -0.8f, -0.8f }, { 0.0f, 1.0f } },
        vertex { {  0.0f,  0.8f }, { 0.5f, 0.0f } },
        vertex { {  0.8f, -0.8f }, { 1.0f, 1.0f } },
    };

    // Vertex buffer description
    uvre::buffer_info vbo_info = {};
    vbo_info.type = uvre::buffer_type::VERTEX_BUFFER;
    vbo_info.size = sizeof(vertices);
    vbo_info.data = vertices;

    // Create vertex buffer
    uvre::buffer *vbo = device->createBuffer(vbo_info);

    // Now the main loop. It should look pretty much the same
    // for all the backend APIs. UVRE is not an exception.
    while(!glfwWindowShouldClose(window)) {
        // Prepare the state to a new frame
        device->prepare();
        
        // Begin recording drawing commands
        // This does nothing for OpenGL.
        device->startRecording(commands);

        // Setup the clear color and clear the screen.
        // After that the screen should turn a nice dark magenta.
        commands->clearColor3f(0.25f, 0.00f, 0.25f);
        commands->clear(uvre::RT_COLOR_BUFFER);

        // Bind the pipeline.
        commands->bindPipeline(pipeline);

        // Bind VBO and draw
        commands->bindVertexBuffer(vbo);
        commands->draw(3, 1, 0, 0);

        // Finish recording and submit the command list.
        // This does nothing for OpenGL.
        device->submit(commands);

        // Finish the frame
        device->present();

        // Handle window's events
        glfwPollEvents();
    }

    // Destroy the vertex buffer
    device->destroyBuffer(vbo);

    // Destroy the pipeline
    device->destroyPipeline(pipeline);

    // Destroy shaders
    device->destroyShader(shaders[0]);
    device->destroyShader(shaders[1]);

    // Destroy the command list
    device->destroyCommandList(commands);

    // Destroy the device
    uvre::destroyDevice(device);

    // Destroy the window
    glfwDestroyWindow(window);

    // De-init GLFW
    glfwTerminate();

    return 0;
}
