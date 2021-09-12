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

struct Shader final {
    uint32_t prog;
    uint32_t stage_bit;
    ShaderStage stage;
};

struct Pipeline final {
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

struct Buffer final {
    uint32_t bufobj;
    VBOBinding *vbo;
    size_t size;
};

struct Texture final {
    uint32_t texobj;
    uint32_t format;
    int width;
    int height;
    int depth;
};

struct Sampler final {
    uint32_t ssobj;
};

struct RenderTarget final {
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
    GLCommandList(GLRenderDevice *device);

    void setScissor(int x, int y, int width, int height);
    void setViewport(int x, int y, int width, int height);

    void clearColor3f(float r, float g, float b);
    void clearColor4f(float r, float g, float b, float a);
    void clear(RenderTargetMask mask);

    void bindPipeline(Pipeline *pipeline);
    void bindUniformBuffer(Buffer *buffer, uint32_t index);
    void bindStorageBuffer(Buffer *buffer, uint32_t index);
    void bindIndexBuffer(Buffer *buffer);
    void bindVertexBuffer(Buffer *buffer);
    void bindSampler(Sampler *sampler, uint32_t index);
    void bindTexture(Texture *texture, uint32_t index);
    void bindRenderTarget(RenderTarget *target);

    bool writeBuffer(Buffer *buffer, size_t offset, size_t size, const void *data);
    void copyRenderTarget(RenderTarget *src, RenderTarget *dst, int sx0, int sy0, int sx1, int sy1, int dx0, int dy0, int dx1, int dy1, RenderTargetMask mask, bool filter);

    void draw(size_t vertices, size_t instances, size_t base_vertex, size_t base_instance);
    void idraw(size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance);

public:
    GLRenderDevice *owner;
};

class GLRenderDevice final : public IRenderDevice {
public:
    GLRenderDevice(const DeviceInfo &info);
    virtual ~GLRenderDevice();

    Shader *createShader(const ShaderInfo &info);
    void destroyShader(Shader *shader);

    Pipeline *createPipeline(const PipelineInfo &info);
    void destroyPipeline(Pipeline *pipeline);

    Buffer *createBuffer(const BufferInfo &info);
    void destroyBuffer(Buffer *buffer);
    void resizeBuffer(Buffer *buffer, size_t size, const void *data);
    bool writeBuffer(Buffer *buffer, size_t offset, size_t size, const void *data);

    Sampler *createSampler(const SamplerInfo &info);
    void destroySampler(Sampler *sampler);

    Texture *createTexture(const TextureInfo &info);
    void destroyTexture(Texture *texture);
    bool writeTexture2D(Texture *texture, int x, int y, int w, int h, PixelFormat format, const void *data);
    bool writeTextureCube(Texture *texture, int face, int x, int y, int w, int h, PixelFormat format, const void *data);
    bool writeTextureArray(Texture *texture, int x, int y, int z, int w, int h, int d, PixelFormat format, const void *data);

    RenderTarget *createRenderTarget(const RenderTargetInfo &info);
    void destroyRenderTarget(RenderTarget *target);

    ICommandList *createCommandList();
    void destroyCommandList(ICommandList *commands);
    void startRecording(ICommandList *commands);
    void submit(ICommandList *commands);

    // TODO: ISwapChain? Are we gonna support headless rendering?
    void prepare();
    void present();
    void vsync(bool enable);
    void mode(int width, int height);

public:
    uint32_t idbo;
    DeviceInfo info;
    VBOBinding *vbos;
    Pipeline null_pipeline;
    Pipeline *bound_pipeline;
    std::vector<Shader *> shaders;
    std::vector<Pipeline *> pipelines;
    std::vector<Buffer *> buffers;
    std::vector<Sampler *> samplers;
    std::vector<Texture *> textures;
    std::vector<RenderTarget *> rendertargets;
    std::vector<GLCommandList *> commandlists;
};

} // namespace uvre
