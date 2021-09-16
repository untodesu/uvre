/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <uvre/uvre.hpp>
#include <algorithm>
#include <glad/gl.h>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace uvre
{
struct VBOBinding final {
    uint32_t index;
    bool is_free;
    VBOBinding *next;
};

struct Shader_S final {
    uint32_t prog;
    uint32_t stage_bit;
    ShaderStage stage;
};

struct Pipeline_S final {
    uint32_t ppobj;
    uint32_t vaobj;
    struct {
        bool enabled;
        uint32_t equation;
        uint32_t sfactor;
        uint32_t dfactor;
    } blending;
    struct {
        bool enabled;
        uint32_t func;
    } depth_testing;
    struct {
        bool enabled;
        uint32_t front_face;
        uint32_t cull_face;
    } face_culling;
    uint32_t index_type;
    uint32_t primitive_mode;
    uint32_t fill_mode;
    size_t vertex_stride;
    std::vector<VertexAttrib> attributes;
};

struct Buffer_S final {
    uint32_t bufobj;
    VBOBinding *vbo;
    size_t size;
};

struct Texture_S final {
    uint32_t texobj;
    uint32_t format;
    int width;
    int height;
    int depth;
};

struct Sampler_S final {
    uint32_t ssobj;
};

struct RenderTarget_S final {
    uint32_t fbobj;
};

union DrawCmd final {
    struct {
        uint32_t vertices;
        uint32_t instances;
        uint32_t base_vertex;
        uint32_t base_instance;
    } a;
    struct {
        uint32_t indices;
        uint32_t instances;
        uint32_t base_index;
        uint32_t base_vertex;
        uint32_t base_instance;
    } e;
};

class GLRenderDevice;
class GLCommandList final : public ICommandList {
public:
    GLCommandList(GLRenderDevice *owner);

    void setScissor(int x, int y, int width, int height) override;
    void setViewport(int x, int y, int width, int height) override;

    void setClearColor3f(float r, float g, float b) override;
    void setClearColor4f(float r, float g, float b, float a) override;
    void clear(RenderTargetMask mask) override;

    void bindPipeline(Pipeline pipeline) override;
    void bindUniformBuffer(Buffer buffer, uint32_t index) override;
    void bindStorageBuffer(Buffer buffer, uint32_t index) override;
    void bindIndexBuffer(Buffer buffer) override;
    void bindVertexBuffer(Buffer buffer) override;
    void bindSampler(Sampler sampler, uint32_t index) override;
    void bindTexture(Texture texture, uint32_t index) override;
    void bindRenderTarget(RenderTarget target) override;

    void writeBuffer(Buffer buffer, size_t offset, size_t size, const void *data) override;
    void copyRenderTarget(RenderTarget src, RenderTarget dst, int sx0, int sy0, int sx1, int sy1, int dx0, int dy0, int dx1, int dy1, RenderTargetMask mask, bool filter) override;

    void draw(size_t vertices, size_t instances, size_t base_vertex, size_t base_instance) override;
    void idraw(size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance) override;

public:
    GLRenderDevice *owner;
};

class GLRenderDevice final : public IRenderDevice {
public:
    GLRenderDevice(const DeviceInfo &info);
    virtual ~GLRenderDevice();

    Shader createShader(const ShaderInfo &info) override;
    Pipeline createPipeline(const PipelineInfo &info) override;
    Buffer createBuffer(const BufferInfo &info) override;
    Sampler createSampler(const SamplerInfo &info) override;
    Texture createTexture(const TextureInfo &info) override;
    RenderTarget createRenderTarget(const RenderTargetInfo &info) override;
    
    void resizeBuffer(Buffer buffer, size_t size, const void *data) override;
    void writeBuffer(Buffer buffer, size_t offset, size_t size, const void *data) override;
    void writeTexture2D(Texture texture, int x, int y, int w, int h, PixelFormat format, const void *data) override;
    void writeTextureCube(Texture texture, int face, int x, int y, int w, int h, PixelFormat format, const void *data) override;
    void writeTextureArray(Texture texture, int x, int y, int z, int w, int h, int d, PixelFormat format, const void *data) override;

    ICommandList *createCommandList() override;
    void destroyCommandList(ICommandList *commands) override;
    void startRecording(ICommandList *commands) override;
    void submit(ICommandList *commands) override;

    void prepare() override;
    void present() override;
    void vsync(bool enable) override;
    void mode(int width, int height) override;

public:
    uint32_t idbo;
    DeviceInfo info;
    VBOBinding *vbos;
    Pipeline null_pipeline;
    Pipeline bound_pipeline;
    std::vector<Pipeline_S *> pipelines;
    std::vector<Buffer_S *> buffers;
    std::vector<GLCommandList *> commandlists;
};

} // namespace uvre
