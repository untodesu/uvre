/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <uvre/const.hpp>
#include <uvre/exports.hpp>
#include <uvre/fwd.hpp>

namespace uvre
{
struct VertexAttrib final {
    uint32_t id;
    VertexAttribType type;
    size_t count;
    size_t offset;
    bool normalized;
};

struct ColorAttachment final {
    uint32_t id;
    Texture color;
};

struct ShaderCreateInfo final {
    ShaderStage stage;
    ShaderFormat format;
    size_t code_size { 0 };
    const void *code;
};

struct PipelineCreateInfo final {
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
    Shader *shaders;
};

struct BufferCreateInfo final {
    BufferType type;
    size_t size;
    const void *data { nullptr };
};

struct SamplerCreateInfo final {
    SamplerFlags flags;
    float aniso_level { 0.0f };
    float min_lod { -1000.0f };
    float max_lod { +1000.0f };
    float lod_bias { 0.0f };
};

struct TextureCreateInfo final {
    TextureType type;
    PixelFormat format;
    int width;
    int height;
    int depth { 0 };
    size_t mip_levels { 0 };
};

struct RenderTargetCreateInfo final {
    Texture depth_attachment { nullptr };
    Texture stencil_attachment { nullptr };
    size_t num_color_attachments;
    const ColorAttachment *color_attachments;
};

struct DeviceInfo final {
    ImplFamily impl_family;
    int impl_version_major;
    int impl_version_minor;
    bool supports_anisotropic;
    bool supports_storage_buffers;
    bool supports_shader_format[static_cast<int>(ShaderFormat::NUM_SHADER_FORMATS)];
};

struct DebugMessageInfo;
struct DeviceCreateInfo final {
    struct {
        void *user_data;
        void*(*getProcAddr)(void *user_data, const char *procname);
        void(*makeContextCurrent)(void *user_data);
        void(*setSwapInterval)(void *user_data, int interval);
        void(*swapBuffers)(void *user_data);
    } gl;
    void(*onDebugMessage)(const DebugMessageInfo &msg);
};

struct ImplInfo final {
    ImplFamily family;
    struct {
        bool core_profile;
        int version_major;
        int version_minor;
    } gl;
};

struct DebugMessageInfo final {
    DebugMessageLevel level;
    const char *text;
};

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual const DeviceInfo &getInfo() const = 0;

    virtual Shader createShader(const ShaderCreateInfo &info) = 0;
    virtual Pipeline createPipeline(const PipelineCreateInfo &info) = 0;
    virtual Buffer createBuffer(const BufferCreateInfo &info) = 0;
    virtual Sampler createSampler(const SamplerCreateInfo &info) = 0;
    virtual Texture createTexture(const TextureCreateInfo &info) = 0;
    virtual RenderTarget createRenderTarget(const RenderTargetCreateInfo &info) = 0;

    virtual void writeBuffer(Buffer buffer, size_t offset, size_t size, const void *data) = 0;
    virtual void writeTexture2D(Texture texture, int x, int y, int w, int h, PixelFormat format, const void *data) = 0;
    virtual void writeTextureCube(Texture texture, int face, int x, int y, int w, int h, PixelFormat format, const void *data) = 0;
    virtual void writeTextureArray(Texture texture, int x, int y, int z, int w, int h, int d, PixelFormat format, const void *data) = 0;

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

UVRE_API void pollImplInfo(ImplInfo &info);
UVRE_API IRenderDevice *createDevice(const DeviceCreateInfo &info);
UVRE_API void destroyDevice(IRenderDevice *device);
} // namespace uvre
