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

namespace uvre
{
struct shader;
struct pipeline;
struct buffer;
struct sampler;
struct texture;
struct rendertarget;

using std::size_t;
using std::uint16_t;
using std::uint32_t;

using index16 = uint16_t;
using index32 = uint32_t;

enum class index_type {
    INDEX16,
    INDEX32
};

enum class primitive_type {
    POINTS,
    LINES,
    LINE_STRIP,
    LINE_LOOP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
};

enum class vertex_attrib_type {
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

enum class texture_type {
    TEXTURE_2D,
    TEXTURE_CUBE,
    TEXTURE_ARRAY
};

enum class pixel_format {
    R8_UNORM,
    R8_SINT,
    R8_UINT,
    R8G8_UNORM,
    R8G8_SINT,
    R8G8_UINT,
    R8G8B8_UNORM,
    R8G8B8_SINT,
    R8G8B8_UINT,
    R8G8B8A8_UNORM,
    R8G8B8A8_SINT,
    R8G8B8A8_UINT,
    R16_UNORM,
    R16_SINT,
    R16_UINT,
    R16_FLOAT,
    R16G16_UNORM,
    R16G16_SINT,
    R16G16_UINT,
    R16G16_FLOAT,
    R16G16B16_UNORM,
    R16G16B16_SINT,
    R16G16B16_UINT,
    R16G16B16_FLOAT,
    R16G16B16A16_UNORM,
    R16G16B16A16_SINT,
    R16G16B16A16_UINT,
    R16G16B16A16_FLOAT,
    R32_SINT,
    R32_UINT,
    R32_FLOAT,
    R32G32_SINT,
    R32G32_UINT,
    R32G32_FLOAT,
    R32G32B32_SINT,
    R32G32B32_UINT,
    R32G32B32_FLOAT,
    R32G32B32A32_SINT,
    R32G32B32A32_UINT,
    R32G32B32A32_FLOAT,
    D16_UNORM,
    D32_FLOAT,
    S8_UINT,
};

enum class blend_equation {
    ADD,
    SUBTRACT,
    REVERSE_SUBTRACT,
    MIN,
    MAX
};

enum class blend_func {
    ZERO,
    ONE,
    SRC_COLOR,
    ONE_MINUS_SRC_COLOR,
    SRC_ALPHA,
    ONE_MINUS_SRC_ALPHA,
    DST_COLOR,
    ONE_MINUS_DST_COLOR,
    DST_ALPHA,
    ONE_MINUS_DST_ALPHA
};

enum class depth_func {
    NEVER,
    ALWAYS,
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_OR_EQUAL,
    GREATER,
    GREATER_OR_EQUAL
};

enum class fill_mode {
    FILLED,
    POINTS,
    WIREFRAME
};

enum class backend_family {
    OPENGL
};

using rendertarget_mask = uint16_t;
static constexpr const rendertarget_mask RT_COLOR_BUFFER = (1 << 0);
static constexpr const rendertarget_mask RT_DEPTH_BUFFER = (1 << 1);
static constexpr const rendertarget_mask RT_STENCIL_BUFFER = (1 << 2);

using sampler_flags = uint16_t;
static constexpr const sampler_flags SAMPLER_CLAMP_S = (1 << 0);
static constexpr const sampler_flags SAMPLER_CLAMP_T = (1 << 1);
static constexpr const sampler_flags SAMPLER_CLAMP_R = (1 << 2);
static constexpr const sampler_flags SAMPLER_FILTER = (1 << 3);
static constexpr const sampler_flags SAMPLER_FILTER_ANISO = (1 << 4);

struct vertex_attrib final {
    uint32_t id;
    vertex_attrib_type type;
    size_t count;
    size_t offset;
    bool normalized;
};

struct color_attachment final {
    uint32_t id;
    texture *color;
};

struct shader_info final {
    shader_stage stage;
    shader_format format;
    size_t code_size { 0 };
    const void *code;
};

struct pipeline_info final {
    struct {
        bool enabled;
        blend_equation equation;
        blend_func sfactor;
        blend_func dfactor;
    } blending;
    struct {
        bool enabled;
        depth_func func;
    } depth_testing;
    struct {
        bool enabled;
        bool clockwise;
        bool cull_back;
        bool cull_front;
    } face_culling;
    index_type index;
    primitive_type primitive;
    fill_mode fill;
    size_t vertex_stride;
    size_t num_vertex_attribs;
    const vertex_attrib *vertex_attribs;
    size_t num_shaders;
    shader **shaders;
};

struct buffer_info final {
    buffer_type type;
    size_t size;
    const void *data { nullptr };
};

struct sampler_info final {
    sampler_flags flags;
    float aniso_level { 0.0f };
    float min_lod { -1000.0f };
    float max_lod { +1000.0f };
    float lod_bias { 0.0f };
};

struct texture_info final {
    texture_type type;
    pixel_format format;
    uint32_t width;
    uint32_t height;
    uint32_t depth { 0 };
    size_t mip_levels { 0 };
    bool write;
    uint32_t wface;
    uint32_t wx, wy, wz;
    uint32_t ww, wh, wd;
    pixel_format wformat;
    const void *wdata { nullptr };
};

struct rendertarget_info final {
    texture *depth_attachment { nullptr };
    texture *stencil_attachment { nullptr };
    size_t num_color_attachments;
    const color_attachment *color_attachments;
};

struct backend_info final {
    backend_family family;
    struct {
        bool core_profile;
        int version_major;
        int version_minor;
    } gl;
};

struct device_info final {
    struct {
        void *user_data;
        void*(*getProcAddr)(void *user_data, const char *procname);
        void(*makeContextCurrent)(void *user_data);
        void(*setSwapInterval)(void *user_data, int interval);
        void(*swapBuffers)(void *user_data);
    } gl;
    void(*onMessage)(const char *message);
};

class ICommandList {
public:
    virtual ~ICommandList() = default;

    virtual void setScissor(int x, int y, int width, int height) = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;

    virtual void clearColor3f(float r, float g, float b) = 0;
    virtual void clearColor4f(float r, float g, float b, float a) = 0;
    virtual void clear(rendertarget_mask mask) = 0;

    virtual void bindPipeline(pipeline *pipeline) = 0;
    virtual void bindUniformBuffer(buffer *buffer, uint32_t index) = 0;
    virtual void bindStorageBuffer(buffer *buffer, uint32_t index) = 0;
    virtual void bindIndexBuffer(buffer *buffer) = 0;
    virtual void bindVertexBuffer(buffer *buffer) = 0;
    virtual void bindSampler(sampler *sampler, uint32_t index) = 0;
    virtual void bindTexture(texture *texture, uint32_t index) = 0;
    virtual void bindRenderTarget(rendertarget *target) = 0;

    virtual bool writeBuffer(buffer *buffer, size_t offset, size_t size, const void *data) = 0;
    virtual void copyRenderTarget(rendertarget *src, rendertarget *dst, int sx0, int sy0, int sx1, int sy1, int dx0, int dy0, int dx1, int dy1, rendertarget_mask mask, bool filter) = 0;

    virtual void draw(size_t vertices, size_t instances, size_t base_vertex, size_t base_instance) = 0;
    virtual void idraw(size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance) = 0;
};

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual shader *createShader(const shader_info &info) = 0;
    virtual void destroyShader(shader *sh) = 0;

    virtual pipeline *createPipeline(const pipeline_info &info) = 0;
    virtual void destroyPipeline(pipeline *pl) = 0;

    virtual buffer *createBuffer(const buffer_info &info) = 0;
    virtual void destroyBuffer(buffer *buffer) = 0;

    virtual sampler *createSampler(const sampler_info &info) = 0;
    virtual void destroySampler(sampler *sampler) = 0;

    virtual texture *createTexture(const texture_info &info) = 0;
    virtual void destroyTexture(texture *texture) = 0;

    virtual rendertarget *createRenderTarget(const rendertarget_info &info) = 0;
    virtual void destroyRenderTarget(rendertarget *target) = 0;

    virtual ICommandList *createCommandList() = 0;
    virtual void destroyCommandList(ICommandList *commands) = 0;
    virtual void startRecording(ICommandList *commands) = 0;
    virtual void submit(ICommandList *commands) = 0;

    // TODO: ISwapChain? Are we gonna support headless rendering?
    virtual void prepare() = 0;
    virtual void present() = 0;
    virtual void vsync(bool enable) = 0;
    virtual void mode(int width, int height) = 0;
};

UVRE_API void pollBackendInfo(backend_info &info);
UVRE_API IRenderDevice *createDevice(const device_info &info);
UVRE_API void destroyDevice(IRenderDevice *device);
} // namespace uvre
