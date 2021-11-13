/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <uvre/types.hpp>

namespace uvre
{
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
    SIGNED_INT32,
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
    SOURCE_GLSL,
    NUM_SHADER_FORMATS
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

enum class DebugMessageLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR
};

enum class ImplFamily {
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
} // namespace uvre
