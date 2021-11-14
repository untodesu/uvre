/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <uvre/const.hpp>
#include <uvre/fwd.hpp>

namespace uvre
{
class ICommandList {
public:
    virtual ~ICommandList() = default;

    virtual void setScissor(int x, int y, int width, int height) = 0;
    virtual void setViewport(int x, int y, int width, int height) = 0;

    virtual void setClearDepth(float d) = 0;
    virtual void setClearColor3f(float r, float g, float b) = 0;
    virtual void setClearColor4f(float r, float g, float b, float a) = 0;
    virtual void clear(RenderTargetMask mask) = 0;

    virtual void bindPipeline(Pipeline pipeline) = 0;
    virtual void bindStorageBuffer(Buffer buffer, uint32_t index) = 0;
    virtual void bindUniformBuffer(Buffer buffer, uint32_t index) = 0;
    virtual void bindIndexBuffer(Buffer buffer) = 0;
    virtual void bindVertexBuffer(Buffer buffer) = 0;
    virtual void bindSampler(Sampler sampler, uint32_t index) = 0;
    virtual void bindTexture(Texture texture, uint32_t index) = 0;
    virtual void bindRenderTarget(RenderTarget target) = 0;

    virtual void writeBuffer(Buffer buffer, size_t offset, size_t size, const void *data) = 0;
    virtual void copyRenderTarget(RenderTarget src, RenderTarget dst, int sx0, int sy0, int sx1, int sy1, int dx0, int dy0, int dx1, int dy1, RenderTargetMask mask, bool filter) = 0;

    virtual void draw(size_t vertices, size_t instances, size_t base_vertex, size_t base_instance) = 0;
    virtual void idraw(size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance) = 0;
};
} // namespace uvre
