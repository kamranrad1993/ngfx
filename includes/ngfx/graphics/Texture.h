/*
 * Copyright 2020 GoPro Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once
#include "ngfx/compute/ComputePipeline.h"
#include "ngfx/graphics/CommandBuffer.h"
#include "ngfx/graphics/GraphicsPipeline.h"

namespace ngfx {
class Graphics;
class GraphicsContext;
class Texture {
public:
  static Texture *
  create(GraphicsContext *graphicsContext, Graphics *graphics,
         const char *filename,
         ImageUsageFlags imageUsageFlags = ImageUsageFlags(
             IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT |
             IMAGE_USAGE_TRANSFER_DST_BIT),
         TextureType textureType = TEXTURE_TYPE_2D, bool genMipmaps = false,
         FilterMode minFilter = FILTER_NEAREST,
         FilterMode magFilter = FILTER_NEAREST,
         FilterMode mipFilter = FILTER_NEAREST, uint32_t numSamples = 1);
  static Texture *
  create(GraphicsContext *graphicsContext, Graphics *graphics, void *data,
         PixelFormat format, uint32_t size, uint32_t w, uint32_t h, uint32_t d,
         uint32_t arrayLayers,
         ImageUsageFlags imageUsageFlags = ImageUsageFlags(
             IMAGE_USAGE_SAMPLED_BIT | IMAGE_USAGE_TRANSFER_SRC_BIT |
             IMAGE_USAGE_TRANSFER_DST_BIT),
         TextureType textureType = TEXTURE_TYPE_2D, bool genMipmaps = false,
         FilterMode minFilter = FILTER_NEAREST,
         FilterMode magFilter = FILTER_NEAREST,
         FilterMode mipFilter = FILTER_NEAREST, uint32_t numSamples = 1);
  virtual ~Texture() {}
  virtual void upload(void *data, uint32_t size, uint32_t x = 0, uint32_t y = 0,
                      uint32_t z = 0, int32_t w = -1, int32_t h = -1,
                      int32_t d = -1, int32_t arrayLayers = -1) = 0;
  virtual void download(void *data, uint32_t size, uint32_t x = 0,
                        uint32_t y = 0, uint32_t z = 0, int32_t w = -1,
                        int32_t h = -1, int32_t d = -1,
                        int32_t arrayLayers = -1) = 0;
  virtual void changeLayout(CommandBuffer *commandBuffer,
                            ImageLayout imageLayout) = 0;
  virtual void generateMipmaps(CommandBuffer *commandBuffer) = 0;
  PixelFormat format;
  uint32_t w = 0, h = 0, d = 1, arrayLayers = 1, mipLevels = 1, numSamples = 1;
  ImageUsageFlags imageUsageFlags;
  TextureType textureType;
};
} // namespace ngfx
