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
struct vbo_binding final {
    uint32_t index;
    bool is_free;
    vbo_binding *next;
};

struct shader final {
    uint32_t prog;
    uint32_t stage_bit;
    shader_stage stage;
};

struct pipeline final {
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
    uint32_t index;
    uint32_t primitive;
    uint32_t fill;
    size_t vertex_stride;
    std::vector<vertex_attrib> attributes;
};

struct buffer final {
    uint32_t bufobj;
    vbo_binding *vbo;
    size_t size;
};

struct texture final {
    uint32_t texobj;
    uint32_t format;
    int width;
    int height;
    int depth;
};

struct sampler final {
    uint32_t ssobj;
};

struct rendertarget final {
    uint32_t fbobj;
};

union drawcmd final {
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
    void clear(rendertarget_mask mask);

    void bindPipeline(pipeline *pipeline);
    void bindUniformBuffer(buffer *buffer, uint32_t index);
    void bindStorageBuffer(buffer *buffer, uint32_t index);
    void bindIndexBuffer(buffer *buffer);
    void bindVertexBuffer(buffer *buffer);
    void bindSampler(sampler *sampler, uint32_t index);
    void bindTexture(texture *texture, uint32_t index);
    void bindRenderTarget(rendertarget *target);

    void copyRenderTarget(rendertarget *src, rendertarget *dst, int sx0, int sy0, int sx1, int sy1, int dx0, int dy0, int dx1, int dy1, rendertarget_mask mask, bool filter);

    void draw(size_t vertices, size_t instances, size_t base_vertex, size_t base_instance);
    void idraw(size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance);

public:
    GLRenderDevice *owner;
    pipeline *bound_pipeline;
};

class GLRenderDevice final : public IRenderDevice {
public:
    GLRenderDevice(const device_info &info);
    virtual ~GLRenderDevice();

    shader *createShader(const shader_info &info);
    void destroyShader(shader *shader);

    pipeline *createPipeline(const pipeline_info &info);
    void destroyPipeline(pipeline *pipeline);

    buffer *createBuffer(const buffer_info &info);
    void destroyBuffer(buffer *buffer);
    void resizeBuffer(buffer *buffer, size_t size, const void *data);
    bool writeBuffer(buffer *buffer, size_t offset, size_t size, const void *data);

    sampler *createSampler(const sampler_info &info);
    void destroySampler(sampler *sampler);

    texture *createTexture(const texture_info &info);
    void destroyTexture(texture *texture);
    bool writeTexture2D(texture *tex, int x, int y, int w, int h, pixel_format format, const void *data);
    bool writeTextureCube(texture *tex, int face, int x, int y, int w, int h, pixel_format format, const void *data);
    bool writeTextureArray(texture *tex, int x, int y, int z, int w, int h, int d, pixel_format format, const void *data);

    rendertarget *createRenderTarget(const rendertarget_info &info);
    void destroyRenderTarget(rendertarget *target);

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
    device_info info;
    vbo_binding *vbos;
    pipeline null_pipeline;
    std::vector<shader *> shaders;
    std::vector<pipeline *> pipelines;
    std::vector<buffer *> buffers;
    std::vector<sampler *> samplers;
    std::vector<texture *> textures;
    std::vector<rendertarget *> rendertargets;
    std::vector<GLCommandList *> commandlists;
};

} // namespace uvre
