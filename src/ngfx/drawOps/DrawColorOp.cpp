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
#include "ngfx/drawOps/DrawColorOp.h"
#include "ngfx/graphics/BufferUtil.h"
#include "ngfx/graphics/Config.h"
#include "ngfx/graphics/ShaderModule.h"
using namespace ngfx;
using namespace glm;

DrawColorOp::DrawColorOp(GraphicsContext *ctx,
                         const std::vector<glm::vec2> &pos,
                         const glm::vec4 &color)
    : DrawOp(ctx) {
  bPos.reset(createVertexBuffer<vec2>(ctx, pos));
  bUbo.reset(createUniformBuffer(ctx, &color, sizeof(color)));
  numVerts = uint32_t(pos.size());
  createPipeline();
  graphicsPipeline->getBindings({&U_UBO}, {&B_POS});
}

void DrawColorOp::draw(CommandBuffer *commandBuffer, Graphics *graphics) {
  graphics->bindGraphicsPipeline(commandBuffer, graphicsPipeline);
  graphics->bindVertexBuffer(commandBuffer, bPos.get(), B_POS, sizeof(vec2));
  graphics->bindUniformBuffer(commandBuffer, bUbo.get(), U_UBO,
                              SHADER_STAGE_FRAGMENT_BIT);
  graphics->draw(commandBuffer, numVerts);
}
void DrawColorOp::createPipeline() {
  const std::string key = "drawColorOp";
  graphicsPipeline = (GraphicsPipeline *)ctx->pipelineCache->get(key);
  if (graphicsPipeline)
    return;
  GraphicsPipeline::State state;
  state.renderPass = ctx->defaultRenderPass;
  state.primitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  auto device = ctx->device;
  graphicsPipeline = GraphicsPipeline::create(
      ctx, state,
      VertexShaderModule::create(device, NGFX_DATA_DIR "/drawColor.vert").get(),
      FragmentShaderModule::create(device, NGFX_DATA_DIR "/drawColor.frag")
          .get(),
      ctx->surfaceFormat, ctx->depthFormat);
  ctx->pipelineCache->add(key, graphicsPipeline);
}
