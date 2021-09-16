/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <memory>

namespace uvre
{
using Shader = std::shared_ptr<struct Shader_S>;
using Pipeline = std::shared_ptr<struct Pipeline_S>;
using Buffer = std::shared_ptr<struct Buffer_S>;
using Sampler = std::shared_ptr<struct Sampler_S>;
using Texture = std::shared_ptr<struct Texture_S>;
using RenderTarget = std::shared_ptr<struct RenderTarget_S>;
class ICommandList;
class IRenderDevice;
} // namespace uvre
