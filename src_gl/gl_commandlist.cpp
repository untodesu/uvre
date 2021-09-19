/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "gl_private.hpp"

static inline void pushCommand(std::vector<uvre::Command> &commands, const uvre::Command &cmd, size_t index)
{
    if(commands.size() <= index) {
        commands.push_back(cmd);
        return;
    }

    std::vector<uvre::Command>::iterator it = commands.begin() + index;
    if(it->type == uvre::CommandType::WRITE_BUFFER)
        delete[] it->buffer_write.data_ptr;
    *it = cmd;
}

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

uvre::GLCommandList::GLCommandList()
    : commands(), num_commands(0)
{
}

void uvre::GLCommandList::setScissor(int x, int y, int width, int height)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::SET_SCISSOR;
    cmd.scvp.x = x;
    cmd.scvp.y = y;
    cmd.scvp.w = width;
    cmd.scvp.h = height;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::setViewport(int x, int y, int width, int height)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::SET_VIEWPORT;
    cmd.scvp.x = x;
    cmd.scvp.y = y;
    cmd.scvp.w = width;
    cmd.scvp.h = height;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::setClearColor3f(float r, float g, float b)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::SET_CLEAR_COLOR;
    cmd.color[0] = r;
    cmd.color[1] = g;
    cmd.color[2] = b;
    cmd.color[3] = 1.0f;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::setClearColor4f(float r, float g, float b, float a)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::SET_CLEAR_COLOR;
    cmd.color[0] = r;
    cmd.color[1] = g;
    cmd.color[2] = b;
    cmd.color[3] = a;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::clear(uvre::RenderTargetMask mask)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::CLEAR;
    cmd.clear_mask = getTargetMask(mask);
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::bindPipeline(uvre::Pipeline pipeline)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::BIND_PIPELINE;
    cmd.pipeline = *pipeline;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::bindStorageBuffer(uvre::Buffer buffer, uint32_t index)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::BIND_STORAGE_BUFFER;
    cmd.bind_index = index;
    cmd.object = buffer ? buffer->bufobj : 0;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::bindUniformBuffer(uvre::Buffer buffer, uint32_t index)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::BIND_UNIFORM_BUFFER;
    cmd.bind_index = index;
    cmd.object = buffer ? buffer->bufobj : 0;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::bindIndexBuffer(uvre::Buffer buffer)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::BIND_INDEX_BUFFER;
    cmd.object = buffer ? buffer->bufobj : 0;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::bindVertexBuffer(uvre::Buffer buffer)
{
    if(buffer) {
        uvre::Command cmd = {};
        cmd.type = uvre::CommandType::BIND_VERTEX_BUFFER;
        cmd.buffer = *buffer;
        pushCommand(commands, cmd, num_commands++);
    }
}

void uvre::GLCommandList::bindSampler(uvre::Sampler sampler, uint32_t index)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::BIND_SAMPLER;
    cmd.bind_index = index;
    cmd.object = sampler ? sampler->ssobj : 0;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::bindTexture(uvre::Texture texture, uint32_t index)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::BIND_TEXTURE;
    cmd.bind_index = index;
    cmd.object = texture ? texture->texobj : 0;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::bindRenderTarget(uvre::RenderTarget target)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::BIND_RENDER_TARGET;
    cmd.object = target ? target->fbobj : 0;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::writeBuffer(uvre::Buffer buffer, size_t offset, size_t size, const void *data)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::WRITE_BUFFER;
    cmd.buffer_write.buffer = buffer->bufobj;
    cmd.buffer_write.offset = offset;
    cmd.buffer_write.size = size;
    cmd.buffer_write.data_ptr = new uint8_t[size];
    std::copy(reinterpret_cast<const uint8_t *>(data), reinterpret_cast<const uint8_t *>(data) + size, cmd.buffer_write.data_ptr);
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::copyRenderTarget(uvre::RenderTarget src, uvre::RenderTarget dst, int sx0, int sy0, int sx1, int sy1, int dx0, int dy0, int dx1, int dy1, uvre::RenderTargetMask mask, bool filter)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::COPY_RENDER_TARGET;
    cmd.rt_copy.src = src ? src->fbobj : 0;
    cmd.rt_copy.dst = dst ? dst->fbobj : 0;
    cmd.rt_copy.sx0 = sx0;
    cmd.rt_copy.sy0 = sy0;
    cmd.rt_copy.sx1 = sx1;
    cmd.rt_copy.sy1 = sy1;
    cmd.rt_copy.dx0 = dx0;
    cmd.rt_copy.dy0 = dy0;
    cmd.rt_copy.dx1 = dx1;
    cmd.rt_copy.dy1 = dy1;
    cmd.rt_copy.mask = getTargetMask(mask);
    cmd.rt_copy.filter = filter ? GL_LINEAR : GL_NEAREST;
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::draw(size_t vertices, size_t instances, size_t base_vertex, size_t base_instance)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::DRAW;
    cmd.draw.a.vertices = static_cast<uint32_t>(vertices);
    cmd.draw.a.instances = static_cast<uint32_t>(instances);
    cmd.draw.a.base_vertex = static_cast<uint32_t>(base_vertex);
    cmd.draw.a.base_instance = static_cast<uint32_t>(base_instance);
    pushCommand(commands, cmd, num_commands++);
}

void uvre::GLCommandList::idraw(size_t indices, size_t instances, size_t base_index, size_t base_vertex, size_t base_instance)
{
    uvre::Command cmd = {};
    cmd.type = uvre::CommandType::IDRAW;
    cmd.draw.e.indices = static_cast<uint32_t>(indices);
    cmd.draw.e.instances = static_cast<uint32_t>(instances);
    cmd.draw.e.base_index = static_cast<uint32_t>(base_index);
    cmd.draw.e.base_vertex = static_cast<uint32_t>(base_vertex);
    cmd.draw.e.base_instance = static_cast<uint32_t>(base_instance);
    pushCommand(commands, cmd, num_commands++);
}
