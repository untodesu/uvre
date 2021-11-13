/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <uvre/uvre.hpp>
#include <GLFW/glfw3.h>
#include <exception>
#include <iostream>

// GLFW error callback
static void onGlfwError(int, const char *message)
{
    std::cerr << message << std::endl;
}

// GLFW framebuffer size callback.
// Handles the window size changes.
static void onGlfwFramebufferSize(GLFWwindow *window, int width, int height)
{
    uvre::IRenderDevice *device = reinterpret_cast<uvre::IRenderDevice *>(glfwGetWindowUserPointer(window));
    device->mode(width, height);
}

// Debug callback
static void onDebugMessage(const uvre::DebugMessageInfo &msg)
{
    std::cout << msg.text << std::endl;
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
    uvre::ImplInfo impl_info;
    uvre::pollImplInfo(impl_info);

    // Do not require any client API by default
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Resizable.
    // This is unnecessary if you don't want the window
    // to be resizable. You are free to change this to GLFW_FALSE.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // If the implementation is OpenGL-ish
    if(impl_info.family == uvre::ImplFamily::OPENGL) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_OPENGL_PROFILE, impl_info.gl.core_profile ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, impl_info.gl.version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, impl_info.gl.version_minor);

#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    }

    constexpr const int WINDOW_WIDTH = 800;
    constexpr const int WINDOW_HEIGHT = 600;

    // Open a new window
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "UVRE", nullptr, nullptr);
    if(!window)
        std::terminate();

    // Now the windowing API also needs to pass some
    // data to the library before creating a rendering
    // device. Usually this data is API-specific callbacks.
    uvre::DeviceCreateInfo device_info = {};

    // OpenGL-specific callbacks
    if(impl_info.family == uvre::ImplFamily::OPENGL) {
        device_info.gl.user_data = window;
        device_info.gl.getProcAddr = [](void *, const char *procname) { return reinterpret_cast<void *>(glfwGetProcAddress(procname)); };
        device_info.gl.makeContextCurrent = [](void *arg) { glfwMakeContextCurrent(reinterpret_cast<GLFWwindow *>(arg)); };
        device_info.gl.setSwapInterval = [](void *, int interval) { glfwSwapInterval(interval); };
        device_info.gl.swapBuffers = [](void *arg) { glfwSwapBuffers(reinterpret_cast<GLFWwindow *>(arg)); };
    }

    // Message callback
    device_info.onDebugMessage = &onDebugMessage;

    // Now we create the rendering device.
    // Rendering device is an object that works
    // with other objects: creates them, destroys
    // them, operates with their internal data, etc.
    uvre::IRenderDevice *device = uvre::createDevice(device_info);
    if(!device)
        std::terminate();

    // Setup the device pointer as a user pointer for the
    // window so we can call its functions while handling
    // various window-related events.
    glfwSetWindowUserPointer(window, device);

    // Set window event callbacks
    // This is unnecessary if you don't want the window
    // to be resizable. You are free to remove this.
    glfwSetFramebufferSizeCallback(window, onGlfwFramebufferSize);

    // A command list is basically an object that
    // records the commands related to drawing. Command
    // lists can be immediate (OpenGL) or deferred (Vulkan).
    // Each command list must be submitted after recording.
    // The create function is guaranteed to return a non-null value.
    uvre::ICommandList *commands = device->createCommandList();

    // Now the main loop. It should look pretty much the same
    // for all the implementations. UVRE is not an exception.
    while(!glfwWindowShouldClose(window)) {
        // Get the current window size.
        // This is unnecessary if you don't want the window
        // to be resizable. You are free to remove this.
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // Prepare the state to a new frame
        device->prepare();

        // Begin recording drawing commands
        // This does nothing for OpenGL.
        device->startRecording(commands);

        // Set the viewport.
        // This is unnecessary if you don't want the window
        // to be resizable. You are free to remove this.
        commands->setViewport(0, 0, width, height);

        // Setup the clear color and clear the screen.
        // After that the screen should turn a nice dark magenta.
        commands->setClearColor3f(0.25f, 0.00f, 0.25f);
        commands->clear(uvre::RT_COLOR_BUFFER);

        // Finish recording and submit the command list.
        // This does nothing for OpenGL.
        device->submit(commands);

        // Finish the frame
        device->present();

        // Handle window's events
        glfwPollEvents();
    }

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
