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
/**
 * A GPU-side program.
 * Cannot be used without pipelines.
 */
struct shader;

/**
 * A set of options and internal objects
 * setting up a needed backend state.
 */
struct pipeline;

/**
 * A GPU-side chunk of data.
 * Universal as an object but can break
 * things real bad when used in wrong methods.
 */
struct buffer;

/**
 * A set of options used to tell the backend
 * how to treat texture objects properly.
 */
struct sampler;

/**
 * A GPU-side image buffer.
 * Can be bound to the same slot as samplers.
 */
struct texture;

/**
 * A set of options and textures used
 * to be drawn into. Needs to be bound.
 * There's a backend-defined default render
 * target, use NULL to bind it.
 */
struct rendertarget;

using std::size_t;
using std::uint16_t;
using std::uint32_t;

using index16 = uint16_t;
using index32 = uint32_t;

/**
 * Represents the index type
 * used for indexed drawing.
 */
enum class index_type {
    INDEX16,
    INDEX32
};

/**
 * Represents the way rasterizer
 * treats the vertex data.
 */
enum class primitive_type {
    POINTS,
    LINES,
    LINE_STRIP,
    LINE_LOOP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
};

/**
 * Represents a type of an attribute that
 * is passed to a vertex shader program.
 */
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

/**
 * A hint for the backend to create buffer
 * objects in the way they are supposed to be created.
 */
enum class buffer_type {
    DATA_BUFFER,
    INDEX_BUFFER,
    VERTEX_BUFFER
};

/**
 * Represents the stage of a shader program.
 * Vertex shaders run per-vertex.
 * Fragment shaders run per-fragment (pixel).
 */
enum class shader_stage {
    VERTEX,
    FRAGMENT
};

/**
 * A hint for the backend in order for it to
 * parse the passed shader code correctly.
 */
enum class shader_format {
    BINARY_SPIRV,
    SOURCE_GLSL
};

/**
 * A hint for the backend to create texture
 * objects in the way they are supposed to be created.
 */
enum class texture_type {
    TEXTURE_2D,
    TEXTURE_CUBE,
    TEXTURE_ARRAY
};

/**
 * A format description (hint) for the backend
 * in order to treat the texture data correctly.
 */
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

/**
 * The equation used when blending
 * two colors together.
 */
enum class blend_equation {
    ADD,
    SUBTRACT,
    REVERSE_SUBTRACT,
    MIN,
    MAX
};

/**
 * A function used in blending
 * two colors together. Used twice.
 */
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

/**
 * A function used to compare pixel
 * depths in order to determine which
 * one is closer to the camera.
 */
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

/**
 * A hint for the backend to draw primitives 
 * in the way they they are supposed to be drawn.
 */
enum class fill_mode {
    FILLED,
    POINTS,
    WIREFRAME
};

/**
 * A hint for the client application about
 * the backend API implemented by the library.
 */
enum class backend_family {
    OPENGL
};

/**
 * A mask for render target operations
 * such as clearing or copying (blitting).
 */
using rendertarget_mask = uint16_t;
static constexpr const rendertarget_mask RT_COLOR_BUFFER = (1 << 0);
static constexpr const rendertarget_mask RT_DEPTH_BUFFER = (1 << 1);
static constexpr const rendertarget_mask RT_STENCIL_BUFFER = (1 << 2);

/**
 * A bit flags used to determine how
 * sampler affects the textures' data
 * in fragment shaders' code.
 */
using sampler_flags = uint16_t;
static constexpr const sampler_flags SAMPLER_CLAMP_S = (1 << 0);
static constexpr const sampler_flags SAMPLER_CLAMP_T = (1 << 1);
static constexpr const sampler_flags SAMPLER_CLAMP_R = (1 << 2);
static constexpr const sampler_flags SAMPLER_FILTER = (1 << 3);
static constexpr const sampler_flags SAMPLER_FILTER_ANISO = (1 << 4);

/**
 * A single variable passed to the
 * vertex shader during rendering
 */
struct vertex_attrib final {
    uint32_t id;
    vertex_attrib_type type;
    size_t count;
    size_t offset;
    bool normalized;
};

/**
 * A texture object supposed to be
 * written into when the owning render
 * target is bound during rendering.
 */
struct color_attachment final {
    uint32_t id;
    texture *color;
};

/**
 * A set of options passed to the rendering
 * device in order to create a valid shader.
 */
struct shader_info final {
    shader_stage stage;
    shader_format format;
    size_t code_size { 0 };
    const void *code;
};

/**
 * A set of options passed to the rendering
 * device in order to create a valid pipeline.
 */
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

/**
 * A set of options passed to the rendering
 * device in order to create a valid buffer.
 */
struct buffer_info final {
    buffer_type type;
    size_t size;
    const void *data { nullptr };
};

/**
 * A set of options passed to the rendering
 * device in order to create a valid sampler.
 */
struct sampler_info final {
    sampler_flags flags;
    float aniso_level { 0.0f };
    float min_lod { -1000.0f };
    float max_lod { +1000.0f };
    float lod_bias { 0.0f };
};

/**
 * A set of options passed to the rendering
 * device in order to create a valid texture.
 */
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

/**
 * A set of options passed to the rendering
 * device in order to create a valid render target.
 */
struct rendertarget_info final {
    texture *depth_attachment { nullptr };
    texture *stencil_attachment { nullptr };
    size_t num_color_attachments;
    const color_attachment *color_attachments;
};

/**
 * Information needed for the client application
 * to construct a correct device_info contents.
 */
struct backend_info final {
    backend_family family;
    struct {
        bool core_profile;
        int version_major;
        int version_minor;
    } gl;
};

/**
 * A set of callbacks and values passed to the
 * render device creation function in order to
 * create a valid render device object.
 */
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

/**
 * Responsible for recording and submitting
 * drawing commands and buffer/texture IO.
 */
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

/**
 * Responsible for creating and destroying objects.
 * Also contains an API for a built-in swapchain.
 */
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

/**
 * Gets the information about the backend
 * needed for the client application.
 */
UVRE_API void pollBackendInfo(backend_info &info);

/**
 * Creates a new IRenderDevice objects.
 * @return Null on failure.
 */
UVRE_API IRenderDevice *createDevice(const device_info &info);

/**
 * Destroys an existing IRenderDevice object.
 * Destroys all the objects related to the device.
 */
UVRE_API void destroyDevice(IRenderDevice *device);
} // namespace uvre
