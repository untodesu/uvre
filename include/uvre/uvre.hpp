/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <cstddef>
#include <cstdint>

#if defined(_MSC_VER)
#if defined(UVRE_SHARED)
#define UVRE_API __declspec(dllexport)
#elif defined(UVRE_STATIC)
#define UVRE_API
#else
#define UVRE_API __declspec(dllimport)
#endif
#elif defined(__GNUC__)
#if defined(UVRE_SHARED)
#define UVRE_API __attribute__((visibility("default")))
#else
#define UVRE_API
#endif
#else
#warning Unknown compiler!
#define UVRE_API
#endif

namespace uvre
{
struct buffer;
struct shader;
struct texture;

// STL aliases
using std::uint16_t;
using std::uint32_t;
using std::size_t;

using index16 = uint16_t;
using index32 = uint32_t;

enum class index_type {
    INDEX16,
    INDEX32
};

enum class attrib_type {
    FLOAT32,
    FLOAT64,
    SIGNED_INT8,
    SIGNED_INT16,
    SIGNED_INT32,
    UNSIGNED_INT8,
    UNSIGNED_INT16,
    UNSIGNED_INT32
};

enum class buffer_type {
    DATA_BUFFER,
    INDEX_BUFFER,
    VERTEX_BUFFER
};

enum class shader_stage {
    VERTEX,
    FRAGMENT
};

enum class shader_format {
    BINARY_SPIRV,
    SOURCE_GLSL
};

enum class pixel_format {
    R_U8,
    R_F16,
    R_F32,
    RG_U8,
    RG_F16,
    RG_F32,
    RGB_U8,
    RGB_F16,
    RGB_F32,
    RGBA_U8,
    RGBA_F16,
    RGBA_F32,
};

struct attrib final {
    uint32_t id;
    attrib_type type;
    size_t count;
    bool normalized;
};

struct buffer_info final {
    buffer_type type;
    size_t size;
    const void *data;
};

struct shader_info final {
    shader_stage stage;
    shader_format format;
    size_t size;
    const void *data;
};

struct texture_info final {
    pixel_format format;
    int width, height;
};

class ICommandList {
public:
    virtual ~ICommandList() = default;

    virtual void setViewport(int x, int y, int width, int height) = 0;

    virtual void clearColor3f(float r, float g, float b) = 0;
    virtual void clearColor4f(float r, float g, float b, float a) = 0;
    virtual void clear(bool color, bool depth, bool stencil) = 0;

    virtual void bindStorageBuffer(buffer *buf, uint32_t index) = 0;
    virtual void bindUniformBuffer(buffer *buf, uint32_t index) = 0;
    virtual void bindIndexBuffer(buffer *buf) = 0;
    virtual void bindVertexBuffer(buffer *buf) = 0;
    virtual void bindTexture(texture *tex, uint32_t index) = 0;
    virtual void bindShader(shader *sh) = 0;

    virtual void draw(size_t vertices, size_t instances, size_t base_vertex, size_t base_instance) = 0;
    virtual void idraw(size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance) = 0;
};

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual void setVertexFormat(const attrib *attributes, size_t num_attributes) = 0;

    virtual buffer *createBuffer(const buffer_info &info) = 0;
    virtual void destroyBuffer(buffer *buf) = 0;
    virtual void resizeBuffer(buffer *buf, size_t size, const void *data) = 0;
    virtual bool writeBuffer(buffer *buf, size_t offset, size_t size, const void *data) = 0;
    virtual size_t bufferSize(buffer *buf) = 0;

    virtual shader *createShader(const shader_info &info) = 0;
    virtual void destroyShader(shader *sh) = 0;
    virtual shader_stage shaderStage(shader *sh) = 0;

    virtual texture *createTexture(const texture_info &info) = 0;
    virtual void destroyTexture(texture *tex) = 0;
    virtual void writeTexture(texture *tex, int x, int y, int width, int height, pixel_format format, const void *data) = 0;
    virtual void textureSize(texture *tex, int *width, int *height) = 0;
};

struct api_info final {
    bool is_gl;
    struct {
        bool core_profile;
        int version_major;
        int version_minor;
    } gl;
};

struct device_info final {
    using gl_proc = void *;
    using gl_swapinterval = void(*)(int);
    using gl_swapbuffers = void(*)();
    using gl_makecurrent = void(*)();
    using gl_getproc = gl_proc(*)(const char *);
    using onmessage = void(*)(const char *);

    struct {
        gl_swapinterval swapInterval;
        gl_swapbuffers swapBuffers;
        gl_makecurrent makeCurrent;
        gl_getproc getProcAddress;
    } gl;

    onmessage onMessage;
};

} // namespace uvre
