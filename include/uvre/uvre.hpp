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
struct Shader;
struct Pipeline;
struct Buffer;
struct Sampler;
struct Texture;
struct RenderTarget;

using std::size_t;
using std::uint16_t;
using std::uint32_t;

using Index16 = uint16_t;
using Index32 = uint32_t;

enum class IndexType {
    INDEX16,
    INDEX32
};

enum class PrimitiveMode {
    POINTS,
    LINES,
    LINE_STRIP,
    LINE_LOOP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
};

enum class VertexAttribType {
    FLOAT32,
    FLOAT64,
    SIGNED_INT8,
    SIGNED_INT16,
    SIGNED_INT32,
    UNSIGNED_INT8,
    UNSIGNED_INT16,
    UNSIGNED_INT32
};

enum class BufferType {
    DATA_BUFFER,
    INDEX_BUFFER,
    VERTEX_BUFFER
};

enum class ShaderStage {
    VERTEX,
    FRAGMENT
};

enum class ShaderFormat {
    BINARY_SPIRV,
    SOURCE_GLSL
};

enum class TextureType {
    TEXTURE_2D,
    TEXTURE_CUBE,
    TEXTURE_ARRAY
};

enum class PixelFormat {
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

enum class BlendEquation {
    ADD,
    SUBTRACT,
    REVERSE_SUBTRACT,
    MIN,
    MAX
};

enum class BlendFunc {
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

enum class DepthFunc {
    NEVER,
    ALWAYS,
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_OR_EQUAL,
    GREATER,
    GREATER_OR_EQUAL
};

enum class FillMode {
    FILLED,
    POINTS,
    WIREFRAME
};

enum class BackendFamily {
    OPENGL
};

using RenderTargetMask = uint16_t;
static constexpr const RenderTargetMask RT_COLOR_BUFFER = (1 << 0);
static constexpr const RenderTargetMask RT_DEPTH_BUFFER = (1 << 1);
static constexpr const RenderTargetMask RT_STENCIL_BUFFER = (1 << 2);

using SamplerFlags = uint16_t;
static constexpr const SamplerFlags SAMPLER_CLAMP_S = (1 << 0);
static constexpr const SamplerFlags SAMPLER_CLAMP_T = (1 << 1);
static constexpr const SamplerFlags SAMPLER_CLAMP_R = (1 << 2);
static constexpr const SamplerFlags SAMPLER_FILTER = (1 << 3);
static constexpr const SamplerFlags SAMPLER_FILTER_ANISO = (1 << 4);

using CullFlags = uint16_t;
static constexpr const CullFlags CULL_CLOCKWISE = (1 << 0);
static constexpr const CullFlags CULL_FRONT = (1 << 1);
static constexpr const CullFlags CULL_BACK = (1 << 2);

struct VertexAttrib final {
    uint32_t id;
    VertexAttribType type;
    size_t count;
    size_t offset;
    bool normalized;
};

struct ColorAttachment final {
    uint32_t id;
    Texture *color;
};

struct ShaderInfo final {
    ShaderStage stage;
    ShaderFormat format;
    size_t code_size { 0 };
    const void *code;
};

struct PipelineInfo final {
    struct {
        bool enabled;
        BlendEquation equation;
        BlendFunc sfactor;
        BlendFunc dfactor;
    } blending;
    struct {
        bool enabled;
        DepthFunc func;
    } depth_testing;
    struct {
        bool enabled;
        CullFlags flags;
    } face_culling;
    IndexType index_type;
    PrimitiveMode primitive_mode;
    FillMode fill_mode;
    size_t vertex_stride;
    size_t num_vertex_attribs;
    const VertexAttrib *vertex_attribs;
    size_t num_shaders;
    Shader **shaders;
};

struct BufferInfo final {
    BufferType type;
    size_t size;
    const void *data { nullptr };
};

struct SamplerInfo final {
    SamplerFlags flags;
    float aniso_level { 0.0f };
    float min_lod { -1000.0f };
    float max_lod { +1000.0f };
    float lod_bias { 0.0f };
};

struct TextureInfo final {
    TextureType type;
    PixelFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t depth { 0 };
    size_t mip_levels { 0 };
};

struct RenderTargetInfo final {
    Texture *depth_attachment { nullptr };
    Texture *stencil_attachment { nullptr };
    size_t num_color_attachments;
    const ColorAttachment *color_attachments;
};

struct BackendInfo final {
    BackendFamily family;
    struct {
        bool core_profile;
        int version_major;
        int version_minor;
    } gl;
};

struct DeviceInfo final {
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
    virtual void clear(RenderTargetMask mask) = 0;

    virtual void bindPipeline(Pipeline *pipeline) = 0;
    virtual void bindUniformBuffer(Buffer *buffer, uint32_t index) = 0;
    virtual void bindStorageBuffer(Buffer *buffer, uint32_t index) = 0;
    virtual void bindIndexBuffer(Buffer *buffer) = 0;
    virtual void bindVertexBuffer(Buffer *buffer) = 0;
    virtual void bindSampler(Sampler *sampler, uint32_t index) = 0;
    virtual void bindTexture(Texture *texture, uint32_t index) = 0;
    virtual void bindRenderTarget(RenderTarget *target) = 0;

    virtual bool writeBuffer(Buffer *buffer, size_t offset, size_t size, const void *data) = 0;
    virtual void copyRenderTarget(RenderTarget *src, RenderTarget *dst, int sx0, int sy0, int sx1, int sy1, int dx0, int dy0, int dx1, int dy1, RenderTargetMask mask, bool filter) = 0;

    virtual void draw(size_t vertices, size_t instances, size_t base_vertex, size_t base_instance) = 0;
    virtual void idraw(size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance) = 0;
};

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual Shader *createShader(const ShaderInfo &info) = 0;
    virtual void destroyShader(Shader *shader) = 0;

    virtual Pipeline *createPipeline(const PipelineInfo &info) = 0;
    virtual void destroyPipeline(Pipeline *pipeline) = 0;

    virtual Buffer *createBuffer(const BufferInfo &info) = 0;
    virtual void destroyBuffer(Buffer *buffer) = 0;
    virtual void resizeBuffer(Buffer *buffer, size_t size, const void *data) = 0;
    virtual bool writeBuffer(Buffer *buffer, size_t offset, size_t size, const void *data) = 0;

    virtual Sampler *createSampler(const SamplerInfo &info) = 0;
    virtual void destroySampler(Sampler *sampler) = 0;

    virtual Texture *createTexture(const TextureInfo &info) = 0;
    virtual void destroyTexture(Texture *texture) = 0;
    virtual bool writeTexture2D(Texture *texture, int x, int y, int w, int h, PixelFormat format, const void *data) = 0;
    virtual bool writeTextureCube(Texture *texture, int face, int x, int y, int w, int h, PixelFormat format, const void *data) = 0;
    virtual bool writeTextureArray(Texture *texture, int x, int y, int z, int w, int h, int d, PixelFormat format, const void *data) = 0;

    virtual RenderTarget *createRenderTarget(const RenderTargetInfo &info) = 0;
    virtual void destroyRenderTarget(RenderTarget *target) = 0;

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

UVRE_API void pollBackendInfo(BackendInfo &info);
UVRE_API IRenderDevice *createDevice(const DeviceInfo &info);
UVRE_API void destroyDevice(IRenderDevice *device);
} // namespace uvre
