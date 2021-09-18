/*
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <functional>
#include "gl_private.hpp"

static uvre::VertexArray dummy_vao = { 0, 0, 0, nullptr };

static void GLAPIENTRY debugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const char *message, const void *arg)
{
    const uvre::DeviceInfo *info = reinterpret_cast<const uvre::DeviceInfo *>(arg);
    info->onMessage(message);
}

static void destroyShader(uvre::Shader_S *shader)
{
    glDeleteProgram(shader->prog);
    delete shader;
}

static void destroyPipeline(uvre::Pipeline_S *pipeline, uvre::GLRenderDevice *device)
{
    // Remove ourselves from the notify list.
    for(std::vector<uvre::Pipeline_S *>::const_iterator it = device->pipelines.cbegin(); it != device->pipelines.cend(); it++) {
        if(*it != pipeline)
            continue;
        device->pipelines.erase(it);
        break;
    }

    // Chain-free the VAO list
    for(uvre::VertexArray *node = pipeline->vaos; node;) {
        uvre::VertexArray *next = node->next;
        glDeleteVertexArrays(1, &node->vaobj);
        delete node;
        node = next;
    }

    glDeleteProgramPipelines(1, &pipeline->ppobj);
    delete pipeline;
}

static void destroyBuffer(uvre::Buffer_S *buffer, uvre::GLRenderDevice *device)
{
    // Remove ourselves from the notify list.
    for(std::vector<uvre::Buffer_S *>::const_iterator it = device->buffers.cbegin(); it != device->buffers.cend(); it++) {
        if(*it != buffer)
            continue;
        device->buffers.erase(it);
        buffer->vbo->is_free = true;
        break;
    }

    glDeleteBuffers(1, &buffer->bufobj);
    delete buffer;
}

static void destroySampler(uvre::Sampler_S *sampler)
{
    glDeleteSamplers(1, &sampler->ssobj);
    delete sampler;
}

static void destroyTexture(uvre::Texture_S *texture)
{
    glDeleteTextures(1, &texture->texobj);
    delete texture;
}

static void destroyRenderTarget(uvre::RenderTarget_S *target)
{
    glDeleteFramebuffers(1, &target->fbobj);
    delete target;
}

uvre::GLRenderDevice::GLRenderDevice(const uvre::DeviceInfo &info)
    : info(info), vbos(nullptr), bound_pipeline(), null_pipeline(), pipelines(), buffers(), commandlists()
{
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &max_vbo_bindings);

    glCreateBuffers(1, &idbo);
    glNamedBufferData(idbo, static_cast<GLsizeiptr>(sizeof(uvre::DrawCmd)), nullptr, GL_DYNAMIC_DRAW);

    null_pipeline.blending.enabled = false;
    null_pipeline.depth_testing.enabled = false;
    null_pipeline.face_culling.enabled = false;
    null_pipeline.index_type = GL_UNSIGNED_SHORT;
    null_pipeline.primitive_mode = GL_TRIANGLES;
    null_pipeline.fill_mode = GL_LINES;
    null_pipeline.vertex_stride = 0;
    null_pipeline.num_attributes = 0;
    null_pipeline.attributes = 0;
    null_pipeline.vaos = nullptr;
    bound_pipeline = null_pipeline;

    vbos = new uvre::VBOBinding;
    vbos->index = 0;
    vbos->is_free = true;
    vbos->next = nullptr;

    if(this->info.onMessage) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugCallback, &this->info);
    }
}

uvre::GLRenderDevice::~GLRenderDevice()
{
    for(uvre::GLCommandList *commandlist : commandlists)
        delete commandlist;

    pipelines.clear();
    buffers.clear();
    commandlists.clear();

    // Make sure that the GL context doesn't use it anymore
    glDisable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(nullptr, nullptr);

    glDeleteBuffers(1, &idbo);
}

uvre::Shader uvre::GLRenderDevice::createShader(const uvre::ShaderInfo &info)
{
    std::stringstream ss;
    ss << "#version 460 core\n";
    ss << "#define _UVRE_ 1\n";

    uint32_t stage = 0;
    uint32_t stage_bit = 0;
    switch(info.stage) {
        case uvre::ShaderStage::VERTEX:
            stage = GL_VERTEX_SHADER;
            stage_bit = GL_VERTEX_SHADER_BIT;
            ss << "#define _VERTEX_SHADER_ 1\n";
            break;
        case uvre::ShaderStage::FRAGMENT:
            stage = GL_FRAGMENT_SHADER;
            stage_bit = GL_FRAGMENT_SHADER_BIT;
            ss << "#define _FRAGMENT_SHADER_ 1\n";
            break;
    }

    int32_t status, info_log_length;
    std::string info_log;
    uint32_t shobj = glCreateShader(stage);
    const char *source_cstr;
    std::string source;

    switch(info.format) {
        case uvre::ShaderFormat::BINARY_SPIRV:
            glShaderBinary(1, &shobj, GL_SHADER_BINARY_FORMAT_SPIR_V, info.code, static_cast<GLsizei>(info.code_size));
            glSpecializeShader(shobj, "main", 0, nullptr, nullptr);
            break;
        case uvre::ShaderFormat::SOURCE_GLSL:
            ss << "#define _GLSL_ 1\n";
            source = ss.str() + reinterpret_cast<const char *>(info.code);
            source_cstr = source.c_str();
            glShaderSource(shobj, 1, &source_cstr, nullptr);
            glCompileShader(shobj);
            break;
        default:
            glDeleteShader(shobj);
            return nullptr;
    }

    if(this->info.onMessage) {
        glGetShaderiv(shobj, GL_INFO_LOG_LENGTH, &info_log_length);
        if(info_log_length > 1) {
            info_log.resize(info_log_length);
            glGetShaderInfoLog(shobj, static_cast<GLsizei>(info_log.size()), nullptr, &info_log[0]);
            this->info.onMessage(info_log.c_str());
        }
    }

    glGetShaderiv(shobj, GL_COMPILE_STATUS, &status);
    if(!status) {
        glDeleteShader(shobj);
        return nullptr;
    }

    uint32_t prog = glCreateProgram();
    glProgramParameteri(prog, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(prog, shobj);
    glLinkProgram(prog);
    glDeleteShader(shobj);

    if(this->info.onMessage) {
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &info_log_length);
        if(info_log_length > 1) {
            info_log.resize(info_log_length);
            glGetProgramInfoLog(prog, static_cast<GLsizei>(info_log.size()), nullptr, &info_log[0]);
            this->info.onMessage(info_log.c_str());
        }
    }

    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if(!status) {
        glDeleteProgram(prog);
        return nullptr;
    }

    uvre::Shader shader(new uvre::Shader_S, destroyShader);
    shader->prog = prog;
    shader->stage = info.stage;
    shader->stage_bit = stage_bit;

    return shader;
}

static inline uint32_t getBlendEquation(uvre::BlendEquation equation)
{
    switch(equation) {
        case uvre::BlendEquation::ADD:
            return GL_FUNC_ADD;
        case uvre::BlendEquation::SUBTRACT:
            return GL_FUNC_SUBTRACT;
        case uvre::BlendEquation::REVERSE_SUBTRACT:
            return GL_FUNC_REVERSE_SUBTRACT;
        case uvre::BlendEquation::MIN:
            return GL_MIN;
        case uvre::BlendEquation::MAX:
            return GL_MAX;
        default:
            return 0;
    }
}

static inline uint32_t getBlendFunc(uvre::BlendFunc func)
{
    switch(func) {
        case uvre::BlendFunc::ZERO:
            return GL_ZERO;
        case uvre::BlendFunc::ONE:
            return GL_ONE;
        case uvre::BlendFunc::SRC_COLOR:
            return GL_SRC_COLOR;
        case uvre::BlendFunc::ONE_MINUS_SRC_COLOR:
            return GL_ONE_MINUS_SRC_COLOR;
        case uvre::BlendFunc::SRC_ALPHA:
            return GL_SRC_ALPHA;
        case uvre::BlendFunc::ONE_MINUS_SRC_ALPHA:
            return GL_ONE_MINUS_SRC_ALPHA;
        case uvre::BlendFunc::DST_COLOR:
            return GL_DST_COLOR;
        case uvre::BlendFunc::ONE_MINUS_DST_COLOR:
            return GL_ONE_MINUS_DST_COLOR;
        case uvre::BlendFunc::DST_ALPHA:
            return GL_DST_ALPHA;
        case uvre::BlendFunc::ONE_MINUS_DST_ALPHA:
            return GL_ONE_MINUS_DST_ALPHA;
        default:
            return 0;
    }
}

static inline uint32_t getDepthFunc(uvre::DepthFunc func)
{
    switch(func) {
        case uvre::DepthFunc::NEVER:
            return GL_NEVER;
        case uvre::DepthFunc::ALWAYS:
            return GL_ALWAYS;
        case uvre::DepthFunc::EQUAL:
            return GL_EQUAL;
        case uvre::DepthFunc::NOT_EQUAL:
            return GL_NOTEQUAL;
        case uvre::DepthFunc::LESS:
            return GL_LESS;
        case uvre::DepthFunc::LESS_OR_EQUAL:
            return GL_LEQUAL;
        case uvre::DepthFunc::GREATER:
            return GL_GREATER;
        case uvre::DepthFunc::GREATER_OR_EQUAL:
            return GL_GEQUAL;
        default:
            return 0;
    }
}

static uint32_t getAttribType(uvre::VertexAttribType type)
{
    switch(type) {
        case uvre::VertexAttribType::FLOAT32:
            return GL_FLOAT;
        case uvre::VertexAttribType::SIGNED_INT32:
            return GL_INT;
        case uvre::VertexAttribType::UNSIGNED_INT32:
            return GL_UNSIGNED_INT;
        default:
            return 0;
    }
}

static inline uint32_t getIndexType(uvre::IndexType type)
{
    switch(type) {
        case uvre::IndexType::INDEX16:
            return GL_UNSIGNED_SHORT;
        case uvre::IndexType::INDEX32:
            return GL_UNSIGNED_INT;
        default:
            return 0;
    }
}

static inline uint32_t getPrimitiveType(uvre::PrimitiveMode type)
{
    switch(type) {
        case uvre::PrimitiveMode::POINTS:
            return GL_POINTS;
        case uvre::PrimitiveMode::LINES:
            return GL_LINES;
        case uvre::PrimitiveMode::LINE_STRIP:
            return GL_LINE_STRIP;
        case uvre::PrimitiveMode::LINE_LOOP:
            return GL_LINE_LOOP;
        case uvre::PrimitiveMode::TRIANGLES:
            return GL_TRIANGLES;
        case uvre::PrimitiveMode::TRIANGLE_STRIP:
            return GL_TRIANGLE_STRIP;
        case uvre::PrimitiveMode::TRIANGLE_FAN:
            return GL_TRIANGLE_FAN;
        default:
            return GL_LINE_STRIP;
    }
}

static inline uint32_t getCullFace(bool back, bool front)
{
    if(back && front)
        return GL_FRONT_AND_BACK;
    if(back)
        return GL_BACK;
    if(front)
        return GL_FRONT;
    return GL_BACK;
}

static inline uint32_t getFillMode(uvre::FillMode mode)
{
    switch(mode) {
        case uvre::FillMode::FILLED:
            return GL_FILL;
        case uvre::FillMode::POINTS:
            return GL_POINT;
        case uvre::FillMode::WIREFRAME:
            return GL_LINE;
        default:
            return GL_LINE;
    }
}

static inline void setVertexFormat(uvre::VertexArray *vao, const uvre::Pipeline_S *pipeline)
{
    if(vao && vao != &dummy_vao) {
        for(size_t i = 0; i < pipeline->num_attributes; i++) {
            uvre::VertexAttrib &attrib = pipeline->attributes[i];
            glEnableVertexArrayAttrib(vao->vaobj, attrib.id);
            switch(attrib.type) {
                case uvre::VertexAttribType::FLOAT32:
                    glVertexArrayAttribFormat(vao->vaobj, attrib.id, static_cast<GLint>(attrib.count), getAttribType(attrib.type), attrib.normalized ? GL_TRUE : GL_FALSE, static_cast<GLuint>(attrib.offset));
                    break;
                case uvre::VertexAttribType::SIGNED_INT32:
                case uvre::VertexAttribType::UNSIGNED_INT32:
                    // Oh, OpenGL, you did it again. You shat itself.
                    glVertexArrayAttribIFormat(vao->vaobj, attrib.id, static_cast<GLint>(attrib.count), getAttribType(attrib.type), static_cast<GLuint>(attrib.offset));
                    break;
            }
        }
    }
}

static inline uvre::VertexArray *getVertexArray(uvre::VertexArray **head, uint32_t index, const uvre::Pipeline_S *pipeline)
{
    for(uvre::VertexArray *node = *head; node; node = node->next) {
        if(index != node->index)
            continue;
        return node;
    }

    uvre::VertexArray *next = new uvre::VertexArray;
    next->index = (*head)->index + 1;
    glCreateVertexArrays(1, &next->vaobj);
    next->vbobj = 0;
    setVertexFormat(next, pipeline);
    next->next = *head;
    *head = next;
    return next;
}

uvre::Pipeline uvre::GLRenderDevice::createPipeline(const uvre::PipelineInfo &info)
{
    uvre::Pipeline pipeline(new uvre::Pipeline_S, std::bind(destroyPipeline, std::placeholders::_1, this));

    glCreateProgramPipelines(1, &pipeline->ppobj);

    pipeline->bound_ibo = 0;
    pipeline->bound_vao = 0;
    pipeline->blending.enabled = info.blending.enabled;
    pipeline->blending.equation = getBlendEquation(info.blending.equation);
    pipeline->blending.sfactor = getBlendFunc(info.blending.sfactor);
    pipeline->blending.dfactor = getBlendFunc(info.blending.dfactor);
    pipeline->depth_testing.enabled = info.depth_testing.enabled;
    pipeline->depth_testing.func = getDepthFunc(info.depth_testing.func);
    pipeline->face_culling.enabled = info.face_culling.enabled;
    pipeline->face_culling.front_face = (info.face_culling.flags & uvre::CULL_CLOCKWISE) ? GL_CW : GL_CCW;
    pipeline->face_culling.cull_face = getCullFace(info.face_culling.flags & uvre::CULL_BACK, info.face_culling.flags & uvre::CULL_FRONT);
    pipeline->index_type = getIndexType(info.index_type);
    pipeline->primitive_mode = getPrimitiveType(info.primitive_mode);
    pipeline->fill_mode = getFillMode(info.fill_mode);
    pipeline->vertex_stride = info.vertex_stride;
    pipeline->num_attributes = info.num_vertex_attribs;
    pipeline->attributes = new uvre::VertexAttrib[pipeline->num_attributes];
    std::copy(info.vertex_attribs, info.vertex_attribs + info.num_vertex_attribs, pipeline->attributes);

    pipeline->vaos = new uvre::VertexArray;
    pipeline->vaos->index = 0;
    glCreateVertexArrays(1, &pipeline->vaos->vaobj);
    pipeline->vaos->next = nullptr;
    setVertexFormat(pipeline->vaos, pipeline.get());

    for(size_t i = 0; i < info.num_shaders; i++) {
        if(info.shaders[i]) {
            // Use this shader stage
            glUseProgramStages(pipeline->ppobj, info.shaders[i]->stage_bit, info.shaders[i]->prog);
        }
    }

    // Notify the buffers
    for(uvre::Buffer_S *buffer : buffers) {
        // offset is zero and that is hardcoded
        glVertexArrayVertexBuffer(getVertexArray(&pipeline->vaos, buffer->vbo->index / max_vbo_bindings, pipeline.get())->vaobj, buffer->vbo->index % max_vbo_bindings, buffer->bufobj, 0, static_cast<GLsizei>(pipeline->vertex_stride));
    }

    // Add ourselves to the notify list.
    pipelines.push_back(pipeline.get());

    return pipeline;
}

static uvre::VBOBinding *getFreeVBOBinding(uvre::VBOBinding **head)
{
    for(uvre::VBOBinding *node = *head; node; node = node->next) {
        if(!node->is_free)
            continue;
        return node;
    }

    uvre::VBOBinding *next = new uvre::VBOBinding;
    next->index = (*head)->index + 1;
    next->is_free = true;
    next->next = *head;
    *head = next;
    return next;
}

uvre::Buffer uvre::GLRenderDevice::createBuffer(const uvre::BufferInfo &info)
{
    uvre::Buffer buffer(new uvre::Buffer_S, std::bind(destroyBuffer, std::placeholders::_1, this));

    glCreateBuffers(1, &buffer->bufobj);

    buffer->size = info.size;
    buffer->vbo = nullptr;

    if(info.type == uvre::BufferType::VERTEX_BUFFER) {
        buffer->vbo = getFreeVBOBinding(&vbos);
        buffer->vbo->is_free = false;

        // Notify the pipeline objects
        for(uvre::Pipeline_S *pipeline : pipelines) {
            // offset is zero and that is hardcoded
            glVertexArrayVertexBuffer(getVertexArray(&pipeline->vaos, buffer->vbo->index / max_vbo_bindings, pipeline)->vaobj, buffer->vbo->index % max_vbo_bindings, buffer->bufobj, 0, static_cast<GLsizei>(pipeline->vertex_stride));
        }

        // Add ourselves to the notify list
        buffers.push_back(buffer.get());
    }

    glNamedBufferStorage(buffer->bufobj, static_cast<GLsizeiptr>(buffer->size), info.data, GL_DYNAMIC_STORAGE_BIT);
    return buffer;
}

void uvre::GLRenderDevice::writeBuffer(uvre::Buffer buffer, size_t offset, size_t size, const void *data)
{
    if(offset + size > buffer->size)
        return;
    glNamedBufferSubData(buffer->bufobj, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
}

uvre::Sampler uvre::GLRenderDevice::createSampler(const uvre::SamplerInfo &info)
{
    uint32_t ssobj;
    glCreateSamplers(1, &ssobj);

    glSamplerParameteri(ssobj, GL_TEXTURE_WRAP_S, (info.flags & SAMPLER_CLAMP_S) ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glSamplerParameteri(ssobj, GL_TEXTURE_WRAP_T, (info.flags & SAMPLER_CLAMP_T) ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glSamplerParameteri(ssobj, GL_TEXTURE_WRAP_R, (info.flags & SAMPLER_CLAMP_R) ? GL_CLAMP_TO_EDGE : GL_REPEAT);

    if(info.flags & SAMPLER_FILTER) {
        if(info.flags & SAMPLER_FILTER_ANISO)
            glSamplerParameterf(ssobj, GL_TEXTURE_MAX_ANISOTROPY, info.aniso_level);
        glSamplerParameterf(ssobj, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameterf(ssobj, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        glSamplerParameterf(ssobj, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameterf(ssobj, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    glSamplerParameterf(ssobj, GL_TEXTURE_MIN_LOD, info.min_lod);
    glSamplerParameterf(ssobj, GL_TEXTURE_MAX_LOD, info.max_lod);
    glSamplerParameterf(ssobj, GL_TEXTURE_LOD_BIAS, info.lod_bias);

    uvre::Sampler sampler(new uvre::Sampler_S, destroySampler);
    sampler->ssobj = ssobj;

    return sampler;
}

static inline uint32_t getInternalFormat(uvre::PixelFormat format)
{
    switch(format) {
        case uvre::PixelFormat::R8_UNORM:
            return GL_R8;
        case uvre::PixelFormat::R8_SINT:
            return GL_R8I;
        case uvre::PixelFormat::R8_UINT:
            return GL_R8UI;
        case uvre::PixelFormat::R8G8_UNORM:
            return GL_RG8;
        case uvre::PixelFormat::R8G8_SINT:
            return GL_RG8I;
        case uvre::PixelFormat::R8G8_UINT:
            return GL_RG8UI;
        case uvre::PixelFormat::R8G8B8_UNORM:
            return GL_RGB8;
        case uvre::PixelFormat::R8G8B8_SINT:
            return GL_RGB8I;
        case uvre::PixelFormat::R8G8B8_UINT:
            return GL_RGB8UI;
        case uvre::PixelFormat::R8G8B8A8_UNORM:
            return GL_RGBA8;
        case uvre::PixelFormat::R8G8B8A8_SINT:
            return GL_RGBA8I;
        case uvre::PixelFormat::R8G8B8A8_UINT:
            return GL_RGBA8UI;
        case uvre::PixelFormat::R16_UNORM:
            return GL_R16;
        case uvre::PixelFormat::R16_SINT:
            return GL_R16I;
        case uvre::PixelFormat::R16_UINT:
            return GL_R16UI;
        case uvre::PixelFormat::R16_FLOAT:
            return GL_R16F;
        case uvre::PixelFormat::R16G16_UNORM:
            return GL_RG16;
        case uvre::PixelFormat::R16G16_SINT:
            return GL_RG16I;
        case uvre::PixelFormat::R16G16_UINT:
            return GL_RG16UI;
        case uvre::PixelFormat::R16G16_FLOAT:
            return GL_RG16F;
        case uvre::PixelFormat::R16G16B16_UNORM:
            return GL_RGB16;
        case uvre::PixelFormat::R16G16B16_SINT:
            return GL_RGB16I;
        case uvre::PixelFormat::R16G16B16_UINT:
            return GL_RGB16UI;
        case uvre::PixelFormat::R16G16B16_FLOAT:
            return GL_RGB16F;
        case uvre::PixelFormat::R16G16B16A16_UNORM:
            return GL_RGBA16;
        case uvre::PixelFormat::R16G16B16A16_SINT:
            return GL_RGBA16I;
        case uvre::PixelFormat::R16G16B16A16_UINT:
            return GL_RGBA16UI;
        case uvre::PixelFormat::R16G16B16A16_FLOAT:
            return GL_RGBA16F;
        case uvre::PixelFormat::R32_SINT:
            return GL_R32I;
        case uvre::PixelFormat::R32_UINT:
            return GL_R32UI;
        case uvre::PixelFormat::R32_FLOAT:
            return GL_R32F;
        case uvre::PixelFormat::R32G32_SINT:
            return GL_RG32I;
        case uvre::PixelFormat::R32G32_UINT:
            return GL_RG32UI;
        case uvre::PixelFormat::R32G32_FLOAT:
            return GL_RG32F;
        case uvre::PixelFormat::R32G32B32_SINT:
            return GL_RGB32I;
        case uvre::PixelFormat::R32G32B32_UINT:
            return GL_RGB32UI;
        case uvre::PixelFormat::R32G32B32_FLOAT:
            return GL_RGB32F;
        case uvre::PixelFormat::R32G32B32A32_SINT:
            return GL_RGBA32I;
        case uvre::PixelFormat::R32G32B32A32_UINT:
            return GL_RGBA32UI;
        case uvre::PixelFormat::R32G32B32A32_FLOAT:
            return GL_RGBA32F;
        case uvre::PixelFormat::D16_UNORM:
            return GL_DEPTH_COMPONENT16;
        case uvre::PixelFormat::D32_FLOAT:
            return GL_DEPTH_COMPONENT32F;
        case uvre::PixelFormat::S8_UINT:
            return GL_STENCIL_INDEX8;
        default:
            return 0;
    }
}

uvre::Texture uvre::GLRenderDevice::createTexture(const uvre::TextureInfo &info)
{
    uint32_t texobj;
    uint32_t format = getInternalFormat(info.format);

    switch(info.type) {
        case uvre::TextureType::TEXTURE_2D:
            glCreateTextures(GL_TEXTURE_2D, 1, &texobj);
            glTextureStorage2D(texobj, std::max<uint32_t>(1, info.mip_levels), format, info.width, info.height);
            break;
        case uvre::TextureType::TEXTURE_CUBE:
            glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texobj);
            glTextureStorage2D(texobj, std::max<uint32_t>(1, info.mip_levels), format, info.width, info.height);
            break;
        case uvre::TextureType::TEXTURE_ARRAY:
            glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &texobj);
            glTextureStorage3D(texobj, std::max<uint32_t>(1, info.mip_levels), format, info.width, info.height, info.depth);
            break;
        default:
            return nullptr;
    }

    uvre::Texture texture(new uvre::Texture_S, destroyTexture);
    texture->texobj = texobj;
    texture->format = format;
    texture->width = info.width;
    texture->height = info.height;
    texture->depth = info.depth;

    return texture;
}

static bool getExternalFormat(uvre::PixelFormat format, uint32_t &fmt, uint32_t &type)
{
    switch(format) {
        case uvre::PixelFormat::R8_UNORM:
        case uvre::PixelFormat::R8_SINT:
        case uvre::PixelFormat::R8_UINT:
        case uvre::PixelFormat::R16_UNORM:
        case uvre::PixelFormat::R16_SINT:
        case uvre::PixelFormat::R16_UINT:
        case uvre::PixelFormat::R16_FLOAT:
        case uvre::PixelFormat::R32_SINT:
        case uvre::PixelFormat::R32_UINT:
        case uvre::PixelFormat::R32_FLOAT:
            fmt = GL_RED;
            break;
        case uvre::PixelFormat::R8G8_UNORM:
        case uvre::PixelFormat::R8G8_SINT:
        case uvre::PixelFormat::R8G8_UINT:
        case uvre::PixelFormat::R16G16_UNORM:
        case uvre::PixelFormat::R16G16_SINT:
        case uvre::PixelFormat::R16G16_UINT:
        case uvre::PixelFormat::R16G16_FLOAT:
        case uvre::PixelFormat::R32G32_SINT:
        case uvre::PixelFormat::R32G32_UINT:
        case uvre::PixelFormat::R32G32_FLOAT:
            fmt = GL_RG;
            break;
        case uvre::PixelFormat::R8G8B8_UNORM:
        case uvre::PixelFormat::R8G8B8_SINT:
        case uvre::PixelFormat::R8G8B8_UINT:
        case uvre::PixelFormat::R16G16B16_UNORM:
        case uvre::PixelFormat::R16G16B16_SINT:
        case uvre::PixelFormat::R16G16B16_UINT:
        case uvre::PixelFormat::R16G16B16_FLOAT:
        case uvre::PixelFormat::R32G32B32_SINT:
        case uvre::PixelFormat::R32G32B32_UINT:
        case uvre::PixelFormat::R32G32B32_FLOAT:
            fmt = GL_RGB;
            break;
        case uvre::PixelFormat::R8G8B8A8_UNORM:
        case uvre::PixelFormat::R8G8B8A8_SINT:
        case uvre::PixelFormat::R8G8B8A8_UINT:
        case uvre::PixelFormat::R16G16B16A16_UNORM:
        case uvre::PixelFormat::R16G16B16A16_SINT:
        case uvre::PixelFormat::R16G16B16A16_UINT:
        case uvre::PixelFormat::R16G16B16A16_FLOAT:
        case uvre::PixelFormat::R32G32B32A32_SINT:
        case uvre::PixelFormat::R32G32B32A32_UINT:
        case uvre::PixelFormat::R32G32B32A32_FLOAT:
            fmt = GL_RGBA;
            break;
        default:
            return false;
    }

    switch(format) {
        case uvre::PixelFormat::R8_SINT:
        case uvre::PixelFormat::R8G8_SINT:
        case uvre::PixelFormat::R8G8B8_SINT:
        case uvre::PixelFormat::R8G8B8A8_SINT:
            type = GL_BYTE;
            break;
        case uvre::PixelFormat::R8_UNORM:
        case uvre::PixelFormat::R8_UINT:
        case uvre::PixelFormat::R8G8_UNORM:
        case uvre::PixelFormat::R8G8_UINT:
        case uvre::PixelFormat::R8G8B8_UNORM:
        case uvre::PixelFormat::R8G8B8_UINT:
        case uvre::PixelFormat::R8G8B8A8_UNORM:
        case uvre::PixelFormat::R8G8B8A8_UINT:
            type = GL_UNSIGNED_BYTE;
            break;
        case uvre::PixelFormat::R16_SINT:
        case uvre::PixelFormat::R16G16_SINT:
        case uvre::PixelFormat::R16G16B16_SINT:
        case uvre::PixelFormat::R16G16B16A16_SINT:
            type = GL_SHORT;
            break;
        case uvre::PixelFormat::R16_UNORM:
        case uvre::PixelFormat::R16_UINT:
        case uvre::PixelFormat::R16G16_UNORM:
        case uvre::PixelFormat::R16G16_UINT:
        case uvre::PixelFormat::R16G16B16_UNORM:
        case uvre::PixelFormat::R16G16B16_UINT:
        case uvre::PixelFormat::R16G16B16A16_UNORM:
        case uvre::PixelFormat::R16G16B16A16_UINT:
            type = GL_UNSIGNED_SHORT;
            break;
        case uvre::PixelFormat::R32_SINT:
        case uvre::PixelFormat::R32G32_SINT:
        case uvre::PixelFormat::R32G32B32_SINT:
        case uvre::PixelFormat::R32G32B32A32_SINT:
            type = GL_INT;
            break;
        case uvre::PixelFormat::R32_UINT:
        case uvre::PixelFormat::R32G32_UINT:
        case uvre::PixelFormat::R32G32B32_UINT:
        case uvre::PixelFormat::R32G32B32A32_UINT:
            type = GL_UNSIGNED_INT;
            break;
        case uvre::PixelFormat::R32_FLOAT:
        case uvre::PixelFormat::R32G32_FLOAT:
        case uvre::PixelFormat::R32G32B32_FLOAT:
        case uvre::PixelFormat::R32G32B32A32_FLOAT:
            type = GL_FLOAT;
            break;
        default:
            return false;
    }

    return true;
}

void uvre::GLRenderDevice::writeTexture2D(uvre::Texture texture, int x, int y, int w, int h, uvre::PixelFormat format, const void *data)
{
    uint32_t fmt, type;
    if(!getExternalFormat(format, fmt, type))
        return;
    glTextureSubImage2D(texture->texobj, 0, x, y, w, h, fmt, type, data);
}

void uvre::GLRenderDevice::writeTextureCube(uvre::Texture texture, int face, int x, int y, int w, int h, uvre::PixelFormat format, const void *data)
{
    uint32_t fmt, type;
    if(!getExternalFormat(format, fmt, type))
        return;
    glTextureSubImage3D(texture->texobj, 0, x, y, face, w, h, 1, fmt, type, data);
}

void uvre::GLRenderDevice::writeTextureArray(uvre::Texture texture, int x, int y, int z, int w, int h, int d, uvre::PixelFormat format, const void *data)
{
    uint32_t fmt, type;
    if(!getExternalFormat(format, fmt, type)) 
        return;
    glTextureSubImage3D(texture->texobj, 0, x, y, z, w, h, d, fmt, type, data);
}

uvre::RenderTarget uvre::GLRenderDevice::createRenderTarget(const uvre::RenderTargetInfo &info)
{
    uint32_t fbobj;
    glCreateFramebuffers(1, &fbobj);
    if(info.depth_attachment)
        glNamedFramebufferTexture(fbobj, GL_DEPTH_ATTACHMENT, info.depth_attachment->texobj, 0);
    if(info.stencil_attachment)
        glNamedFramebufferTexture(fbobj, GL_STENCIL_ATTACHMENT, info.stencil_attachment->texobj, 0);
    for(size_t i = 0; i < info.num_color_attachments; i++)
        glNamedFramebufferTexture(fbobj, GL_COLOR_ATTACHMENT0 + info.color_attachments[i].id, info.color_attachments[i].color->texobj, 0);

    if(glCheckNamedFramebufferStatus(fbobj, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &fbobj);
        return nullptr;
    }

    uvre::RenderTarget target(new uvre::RenderTarget_S, destroyRenderTarget);
    target->fbobj = fbobj;

    return target;
}

uvre::ICommandList *uvre::GLRenderDevice::createCommandList()
{
    uvre::GLCommandList *commands = new uvre::GLCommandList();
    commandlists.push_back(commands);
    return commands;
}

void uvre::GLRenderDevice::destroyCommandList(uvre::ICommandList *commands)
{
    for(std::vector<uvre::GLCommandList *>::const_iterator it = commandlists.cbegin(); it != commandlists.cend(); it++) {
        if(*it == commands) {
            commandlists.erase(it);
            delete commands;
            return;
        }
    }
}

void uvre::GLRenderDevice::startRecording(uvre::ICommandList *commands)
{
    uvre::GLCommandList *glcommands = static_cast<uvre::GLCommandList *>(commands);
    glcommands->num_commands = 0;
}

void uvre::GLRenderDevice::submit(uvre::ICommandList *commands)
{
    uvre::GLCommandList *glcommands = static_cast<uvre::GLCommandList *>(commands);
    for(size_t i = 0; i < glcommands->num_commands; i++) {
        const uvre::Command &cmd = glcommands->commands[i];
        uvre::VertexArray *vaonode = nullptr;
        switch(cmd.type) {
            case uvre::CommandType::SET_SCISSOR:
                glScissor(cmd.scvp.x, cmd.scvp.y, cmd.scvp.w, cmd.scvp.h);
                break;
            case uvre::CommandType::SET_VIEWPORT:
                glViewport(cmd.scvp.x, cmd.scvp.y, cmd.scvp.w, cmd.scvp.h);
                break;
            case uvre::CommandType::SET_CLEAR_COLOR:
                glClearColor(cmd.color[0], cmd.color[1], cmd.color[2], cmd.color[3]);
                break;
            case uvre::CommandType::CLEAR:
                glClear(cmd.clear_mask);
                break;
            case uvre::CommandType::BIND_PIPELINE:
                bound_pipeline = cmd.pipeline;
                glDisable(GL_BLEND);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                if(bound_pipeline.blending.enabled) {
                    glEnable(GL_BLEND);
                    glBlendEquation(bound_pipeline.blending.equation);
                    glBlendFunc(bound_pipeline.blending.sfactor, bound_pipeline.blending.dfactor);
                }
                if(bound_pipeline.depth_testing.enabled) {
                    glEnable(GL_DEPTH_TEST);
                    glDepthFunc(bound_pipeline.depth_testing.func);
                }
                if(bound_pipeline.face_culling.enabled) {
                    glEnable(GL_CULL_FACE);
                    glCullFace(bound_pipeline.face_culling.cull_face);
                    glFrontFace(bound_pipeline.face_culling.front_face);
                }
                glPolygonMode(GL_FRONT_AND_BACK, bound_pipeline.fill_mode);
                glBindProgramPipeline(bound_pipeline.ppobj);
                break;
            case uvre::CommandType::BIND_STORAGE_BUFFER:
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, cmd.bind_index, cmd.object);
                break;
            case uvre::CommandType::BIND_UNIFORM_BUFFER:
                glBindBufferBase(GL_UNIFORM_BUFFER, cmd.bind_index, cmd.object);
                break;
            case uvre::CommandType::BIND_INDEX_BUFFER: // OPTIMIZE
                bound_pipeline.bound_ibo = cmd.object;
                break;
            case uvre::CommandType::BIND_VERTEX_BUFFER: // OPTIMIZE
                vaonode = getVertexArray(&bound_pipeline.vaos, cmd.buffer.vbo->index / max_vbo_bindings, &bound_pipeline);
                if(vaonode->vaobj != bound_pipeline.bound_vao) {
                    bound_pipeline.bound_vao = vaonode->vaobj;
                    glBindVertexArray(vaonode->vaobj);
                }
                if(vaonode->vbobj != cmd.buffer.bufobj) {
                    vaonode->vbobj = cmd.buffer.bufobj;
                    for(size_t j = 0; j < bound_pipeline.num_attributes; j++)
                        glVertexArrayAttribBinding(vaonode->vaobj, bound_pipeline.attributes[j].id, cmd.buffer.vbo->index % max_vbo_bindings);
                    glVertexArrayElementBuffer(vaonode->vaobj, bound_pipeline.bound_ibo);
                }
                break;
            case uvre::CommandType::BIND_SAMPLER:
                glBindSampler(cmd.bind_index, cmd.object);
                break;
            case uvre::CommandType::BIND_TEXTURE:
                glBindTextureUnit(cmd.bind_index, cmd.object);
                break;
            case uvre::CommandType::BIND_RENDER_TARGET:
                glBindFramebuffer(GL_FRAMEBUFFER, cmd.object);
                break;
            case uvre::CommandType::WRITE_BUFFER:
                glNamedBufferSubData(cmd.buffer_write.buffer, static_cast<GLintptr>(cmd.buffer_write.offset), static_cast<GLsizeiptr>(cmd.buffer_write.size), cmd.buffer_write.data_ptr);
                break;
            case uvre::CommandType::COPY_RENDER_TARGET:
                glBlitNamedFramebuffer(cmd.rt_copy.src, cmd.rt_copy.dst, cmd.rt_copy.sx0, cmd.rt_copy.sy0, cmd.rt_copy.sx1, cmd.rt_copy.sy1, cmd.rt_copy.dx0, cmd.rt_copy.dy0, cmd.rt_copy.dx1, cmd.rt_copy.dy1, cmd.rt_copy.mask, cmd.rt_copy.filter);
                break;
            case uvre::CommandType::DRAW:
                glNamedBufferSubData(idbo, 0, static_cast<GLsizeiptr>(sizeof(cmd.draw.a)), &cmd.draw.a);
                glDrawArraysIndirect(bound_pipeline.primitive_mode, nullptr);
                break;
            case uvre::CommandType::IDRAW:
                glNamedBufferSubData(idbo, 0, static_cast<GLsizeiptr>(sizeof(cmd.draw.e)), &cmd.draw.e);
                glDrawElementsIndirect(bound_pipeline.primitive_mode, bound_pipeline.index_type, nullptr);
                break;
        }
    }
}

void uvre::GLRenderDevice::prepare()
{
    // Third-party overlay applications
    // can cause mayhem if this is not called.
    glUseProgram(0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, idbo);
}

void uvre::GLRenderDevice::present()
{
    info.gl.swapBuffers(info.gl.user_data);
}

void uvre::GLRenderDevice::vsync(bool enable)
{
    info.gl.setSwapInterval(info.gl.user_data, enable ? 1 : 0);
}

void uvre::GLRenderDevice::mode(int, int)
{
    // Nothing in OpenGL
}
