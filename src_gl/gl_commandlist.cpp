/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "gl_private.hpp"

static inline uint32_t getTargetMask(uvre::RenderTargetMask mask)
{
    uint32_t result = 0;
    if(mask & uvre::RT_COLOR_BUFFER)
        result |= GL_COLOR_BUFFER_BIT;
    if(mask & uvre::RT_DEPTH_BUFFER)
        result |= GL_DEPTH_BUFFER_BIT;
    if(mask & uvre::RT_STENCIL_BUFFER)
        result |= GL_STENCIL_BUFFER_BIT;
    return result;
}

uvre::GLCommandList::GLCommandList(uvre::GLRenderDevice *owner)
    : owner(owner)
{
}

void uvre::GLCommandList::setScissor(int x, int y, int width, int height)
{
    glScissor(x, y, width, height);
}

void uvre::GLCommandList::setViewport(int x, int y, int width, int height)
{
    glScissor(x, y, width, height);
}

void uvre::GLCommandList::setClearColor3f(float r, float g, float b)
{
    glClearColor(r, g, b, 1.0f);
}

void uvre::GLCommandList::setClearColor4f(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void uvre::GLCommandList::clear(uvre::RenderTargetMask mask)
{
    glClear(getTargetMask(mask));
}

void uvre::GLCommandList::bindPipeline(uvre::Pipeline pipeline)
{
    owner->bound_pipeline = pipeline ? pipeline : owner->null_pipeline;

    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if(owner->bound_pipeline->blending.enabled) {
        glEnable(GL_BLEND);
        glBlendEquation(owner->bound_pipeline->blending.equation);
        glBlendFunc(owner->bound_pipeline->blending.sfactor, owner->bound_pipeline->blending.dfactor);
    }

    if(owner->bound_pipeline->depth_testing.enabled) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(owner->bound_pipeline->depth_testing.func);
    }

    if(owner->bound_pipeline->face_culling.enabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(owner->bound_pipeline->face_culling.cull_face);
        glFrontFace(owner->bound_pipeline->face_culling.front_face);
    }

    glPolygonMode(GL_FRONT_AND_BACK, owner->bound_pipeline->fill_mode);

    glBindProgramPipeline(owner->bound_pipeline->ppobj);
    glBindVertexArray(owner->bound_pipeline->vaobj);
}

void uvre::GLCommandList::bindStorageBuffer(uvre::Buffer buffer, uint32_t index)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer ? buffer->bufobj : 0);
}

void uvre::GLCommandList::bindUniformBuffer(uvre::Buffer buffer, uint32_t index)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer ? buffer->bufobj : 0);
}

void uvre::GLCommandList::bindIndexBuffer(uvre::Buffer buffer)
{
    if(!owner->bound_pipeline->vaobj)
        return;
    glVertexArrayElementBuffer(owner->bound_pipeline->vaobj, buffer ? buffer->bufobj : 0);
}

void uvre::GLCommandList::bindVertexBuffer(uvre::Buffer buffer)
{
    if(!buffer->vbo || !owner->bound_pipeline->vaobj)
        return;
    for(const uvre::VertexAttrib &attrib : owner->bound_pipeline->attributes)
        glVertexArrayAttribBinding(owner->bound_pipeline->vaobj, attrib.id, buffer->vbo->index);
}

void uvre::GLCommandList::bindSampler(uvre::Sampler sampler, uint32_t index)
{
    glBindSampler(index, sampler ? sampler->ssobj : 0);
}

void uvre::GLCommandList::bindTexture(uvre::Texture texture, uint32_t index)
{
    glBindTextureUnit(index, texture ? texture->texobj : 0);
}

void uvre::GLCommandList::bindRenderTarget(uvre::RenderTarget target)
{
    glBindFramebuffer(GL_FRAMEBUFFER, target ? target->fbobj : 0);
}

void uvre::GLCommandList::writeBuffer(uvre::Buffer buffer, size_t offset, size_t size, const void *data)
{
    owner->writeBuffer(buffer, offset, size, data);
}

void uvre::GLCommandList::copyRenderTarget(uvre::RenderTarget src, uvre::RenderTarget dst, int sx0, int sy0, int sx1, int sy1, int dx0, int dy0, int dx1, int dy1, uvre::RenderTargetMask mask, bool filter)
{
    glBlitNamedFramebuffer(src ? src->fbobj : 0, dst ? dst->fbobj : 0, sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, getTargetMask(mask), filter ? GL_LINEAR : GL_NEAREST);
}

void uvre::GLCommandList::draw(size_t vertices, size_t instances, size_t base_vertex, size_t base_instance)
{
    uvre::DrawCmd cmd;
    cmd.a.vertices = static_cast<uint32_t>(vertices);
    cmd.a.instances = static_cast<uint32_t>(instances);
    cmd.a.base_vertex = static_cast<uint32_t>(base_vertex);
    cmd.a.base_instance = static_cast<uint32_t>(base_instance);
    glNamedBufferSubData(owner->idbo, 0, static_cast<GLsizeiptr>(sizeof(cmd.a)), &cmd.a);
    glDrawArraysIndirect(owner->bound_pipeline->primitive_mode, nullptr);
}

void uvre::GLCommandList::idraw(size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance)
{
    uvre::DrawCmd cmd;
    cmd.e.indices = static_cast<uint32_t>(indices);
    cmd.e.instances = static_cast<uint32_t>(instances);
    cmd.e.base_index = static_cast<uint32_t>(base_index);
    cmd.e.base_vertex = static_cast<uint32_t>(base_vertex);
    cmd.e.base_instance = static_cast<uint32_t>(base_instance);
    glNamedBufferSubData(owner->idbo, 0, static_cast<GLsizeiptr>(sizeof(cmd.e)), &cmd.e);
    glDrawElementsIndirect(owner->bound_pipeline->primitive_mode, owner->bound_pipeline->index_type, nullptr);
}
