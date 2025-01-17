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
#include "ngfx/porting/d3d/D3DGraphics.h"
#include "ngfx/porting/d3d/D3DBuffer.h"
#include "ngfx/porting/d3d/D3DCommandList.h"
#include "ngfx/porting/d3d/D3DComputePipeline.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DGraphicsPipeline.h"
#include "ngfx/porting/d3d/D3DTexture.h"
#include <glm/gtc/type_ptr.hpp>
using namespace ngfx;

void D3DGraphics::bindComputePipeline(CommandBuffer *commandBuffer,
                                      ComputePipeline *computePipeline) {
  auto d3dCtx = d3d(ctx);

  auto d3dCommandList = d3d(commandBuffer)->v;
  auto d3dComputePipeline = d3d(computePipeline);
  D3D_TRACE(d3dCommandList->SetPipelineState(
      d3dComputePipeline->d3dPipelineState.Get()));
  D3D_TRACE(d3dCommandList->SetComputeRootSignature(
      d3dComputePipeline->d3dRootSignature.Get()));
  auto cbvSrvUavHeap = d3dCtx->d3dCbvSrvUavDescriptorHeap.v.Get();
  auto samplerDescriptorHeap = d3dCtx->d3dSamplerDescriptorHeap.v.Get();
  std::vector<ID3D12DescriptorHeap *> descriptorHeaps = {cbvSrvUavHeap,
                                                         samplerDescriptorHeap};
  D3D_TRACE(d3dCommandList->SetDescriptorHeaps(UINT(descriptorHeaps.size()),
                                               descriptorHeaps.data()));
  currentPipeline = computePipeline;
}

void D3DGraphics::bindGraphicsPipeline(CommandBuffer *commandBuffer,
                                       GraphicsPipeline *graphicsPipeline) {
  auto d3dCommandList = d3d(commandBuffer)->v;
  auto d3dGraphicsPipeline = d3d(graphicsPipeline);
  D3D_TRACE(d3dCommandList->SetPipelineState(
      d3dGraphicsPipeline->d3dPipelineState.Get()));
  D3D_TRACE(d3dCommandList->IASetPrimitiveTopology(
      d3dGraphicsPipeline->d3dPrimitiveTopology));
  D3D_TRACE(d3dCommandList->SetGraphicsRootSignature(
      d3dGraphicsPipeline->d3dRootSignature.Get()));
  currentPipeline = graphicsPipeline;
}

void D3DGraphics::bindUniformBuffer(CommandBuffer *commandBuffer,
                                    Buffer *buffer, uint32_t binding,
                                    ShaderStageFlags shaderStageFlags) {
  auto d3dCommandList = d3d(commandBuffer)->v.Get();
  auto d3dBuffer = d3d(buffer);
  if (D3DGraphicsPipeline *graphicsPipeline =
          dynamic_cast<D3DGraphicsPipeline *>(currentPipeline)) {
    D3D_TRACE(d3dCommandList->SetGraphicsRootConstantBufferView(
        binding, d3dBuffer->v->GetGPUVirtualAddress()));
  } else if (D3DComputePipeline *computePipeline =
                 dynamic_cast<D3DComputePipeline *>(currentPipeline)) {
    D3D_TRACE(d3dCommandList->SetComputeRootConstantBufferView(
        binding, d3dBuffer->v->GetGPUVirtualAddress()));
  }
}

void D3DGraphics::bindIndexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                  IndexFormat indexFormat) {
  auto d3dBuffer = d3d(buffer);
  D3D12_INDEX_BUFFER_VIEW ib;
  ib.BufferLocation = d3dBuffer->v->GetGPUVirtualAddress();
  ib.Format = DXGI_FORMAT(indexFormat);
  ib.SizeInBytes = d3dBuffer->size;
  d3d(commandBuffer)->v->IASetIndexBuffer(&ib);
}

void D3DGraphics::bindVertexBuffer(CommandBuffer *commandBuffer, Buffer *buffer,
                                   uint32_t location, uint32_t stride) {
  auto d3dBuffer = d3d(buffer);
  D3D12_VERTEX_BUFFER_VIEW vb;
  vb.BufferLocation = d3dBuffer->v->GetGPUVirtualAddress();
  vb.StrideInBytes = stride;
  vb.SizeInBytes = d3dBuffer->size;
  d3d(commandBuffer)->v->IASetVertexBuffers(location, 1, &vb);
}

void D3DGraphics::bindStorageBuffer(CommandBuffer *commandBuffer,
                                    Buffer *buffer, uint32_t binding,
                                    ShaderStageFlags shaderStageFlags) {
  auto d3dCommandList = d3d(commandBuffer)->v.Get();
  auto d3dBuffer = d3d(buffer);
  // TODO encode access flags as read-only or read-write
  if (D3DGraphicsPipeline *graphicsPipeline =
          dynamic_cast<D3DGraphicsPipeline *>(currentPipeline)) {
    D3D_TRACE(d3dCommandList->SetGraphicsRootShaderResourceView(
        binding, d3dBuffer->v->GetGPUVirtualAddress()));
  } else if (D3DComputePipeline *computePipeline =
                 dynamic_cast<D3DComputePipeline *>(currentPipeline)) {
    D3D_TRACE(d3dCommandList->SetComputeRootUnorderedAccessView(
        binding, d3dBuffer->v->GetGPUVirtualAddress()));
  }
}

void D3DGraphics::bindTexture(CommandBuffer *commandBuffer, Texture *texture,
                              uint32_t set) {
  auto d3dCommandList = d3d(commandBuffer)->v.Get();
  auto d3dTexture = d3d(texture);
  if (D3DGraphicsPipeline *graphicsPipeline =
          dynamic_cast<D3DGraphicsPipeline *>(currentPipeline)) {
    D3D_TRACE(d3dCommandList->SetGraphicsRootDescriptorTable(
        set, d3dTexture->defaultSrvDescriptor.gpuHandle));
    D3D_TRACE(d3dCommandList->SetGraphicsRootDescriptorTable(
        set + 1, d3dTexture->defaultSamplerDescriptor.gpuHandle));
  } else if (D3DComputePipeline *computePipeline =
                 dynamic_cast<D3DComputePipeline *>(currentPipeline)) {
    D3D_TRACE(d3dCommandList->SetComputeRootDescriptorTable(
        set, d3dTexture->defaultSrvDescriptor.gpuHandle));
    D3D_TRACE(d3dCommandList->SetComputeRootDescriptorTable(
        set + 1, d3dTexture->defaultSamplerDescriptor.gpuHandle));
  }
}

static void resourceBarrier(
    D3DCommandList *cmdList, D3DFramebuffer::D3DAttachment *p,
    D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES newState,
    UINT subresourceIndex = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
  if (p->texture)
    p->texture->resourceBarrier(cmdList, newState, subresourceIndex);
  else {
    CD3DX12_RESOURCE_BARRIER resourceBarrier =
        CD3DX12_RESOURCE_BARRIER::Transition(p->resource, currentState,
                                             newState, p->subresourceIndex);
    D3D_TRACE(cmdList->v->ResourceBarrier(1, &resourceBarrier));
  }
}
void D3DGraphics::beginRenderPass(CommandBuffer *commandBuffer,
                                  RenderPass *renderPass,
                                  Framebuffer *framebuffer,
                                  glm::vec4 clearColor, float clearDepth,
                                  uint32_t clearStencil) {
  auto d3dCtx = d3d(ctx);
  auto d3dRenderPass = d3d(renderPass);
  auto d3dCommandList = d3d(commandBuffer);
  auto d3dFramebuffer = d3d(framebuffer);
  auto &colorAttachments = d3dFramebuffer->colorAttachments;
  auto &resolveAttachments = d3dFramebuffer->resolveAttachments;
  auto depthStencilAttachment = d3dFramebuffer->depthStencilAttachment;
  if (!resolveAttachments.empty()) {
    for (auto &colorAttachment : colorAttachments) {
      resourceBarrier(d3dCommandList, colorAttachment,
                      D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
                      D3D12_RESOURCE_STATE_RENDER_TARGET,
                      colorAttachment->subresourceIndex);
    }
  } else {
    for (auto &colorAttachment : colorAttachments) {
      resourceBarrier(d3dCommandList, colorAttachment,
                      d3dRenderPass->finalResourceState,
                      d3dRenderPass->initialResourceState,
                      colorAttachment->subresourceIndex);
    }
  }
  auto cbvSrvUavHeap = d3dCtx->d3dCbvSrvUavDescriptorHeap.v.Get();
  auto samplerDescriptorHeap = d3dCtx->d3dSamplerDescriptorHeap.v.Get();
  std::vector<ID3D12DescriptorHeap *> descriptorHeaps = {cbvSrvUavHeap,
                                                         samplerDescriptorHeap};
  D3D_TRACE(d3dCommandList->v->SetDescriptorHeaps(UINT(descriptorHeaps.size()),
                                                  descriptorHeaps.data()));
  std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> colorAttachmentHandles(
      colorAttachments.size());
  for (uint32_t j = 0; j < colorAttachments.size(); j++)
    colorAttachmentHandles[j] = colorAttachments[j]->cpuDescriptor;
  D3D_TRACE(d3dCommandList->v->OMSetRenderTargets(
      UINT(colorAttachments.size()), colorAttachmentHandles.data(), FALSE,
      depthStencilAttachment ? &depthStencilAttachment->cpuDescriptor
                             : nullptr));
  for (auto &colorAttachment : colorAttachments) {
    D3D_TRACE(d3dCommandList->v->ClearRenderTargetView(
        colorAttachment->cpuDescriptor, glm::value_ptr(clearColor), 0,
        nullptr));
  }
  if (depthStencilAttachment) {
    D3D_TRACE(d3dCommandList->v->ClearDepthStencilView(
        depthStencilAttachment->cpuDescriptor,
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clearDepth,
        clearStencil, 0, nullptr));
  }
  currentRenderPass = renderPass;
  currentFramebuffer = framebuffer;
}

void D3DGraphics::endRenderPass(CommandBuffer *commandBuffer) {
  auto d3dCommandList = d3d(commandBuffer);
  auto d3dFramebuffer = d3d(currentFramebuffer);
  auto &colorAttachments = d3dFramebuffer->colorAttachments;
  auto &resolveAttachments = d3dFramebuffer->resolveAttachments;
  auto d3dRenderPass = d3d(currentRenderPass);
  if (!resolveAttachments.empty()) {
    for (uint32_t j = 0; j < colorAttachments.size(); j++) {
      auto &colorAttachment = colorAttachments[j];
      auto &resolveAttachment = resolveAttachments[j];
      resourceBarrier(d3dCommandList, colorAttachment,
                      D3D12_RESOURCE_STATE_RENDER_TARGET,
                      D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
      resourceBarrier(d3dCommandList, resolveAttachment,
                      D3D12_RESOURCE_STATE_PRESENT,
                      D3D12_RESOURCE_STATE_RESOLVE_DEST);
      D3D_TRACE(d3dCommandList->v->ResolveSubresource(
          resolveAttachment->resource, resolveAttachment->subresourceIndex,
          colorAttachment->resource, colorAttachment->subresourceIndex,
          colorAttachment->format));
      resourceBarrier(d3dCommandList, resolveAttachment,
                      D3D12_RESOURCE_STATE_RESOLVE_DEST,
                      d3dRenderPass->finalResourceState,
                      resolveAttachment->subresourceIndex);
    }
  } else {
    for (auto &colorAttachment : colorAttachments) {
      resourceBarrier(
          d3dCommandList, colorAttachment, d3dRenderPass->initialResourceState,
          d3dRenderPass->finalResourceState, colorAttachment->subresourceIndex);
    }
  }
  currentRenderPass = nullptr;
  currentFramebuffer = nullptr;
}

void D3DGraphics::beginProfile(CommandBuffer *commandBuffer) {
    d3d(commandBuffer)->v->EndQuery(d3d(ctx)->d3dQueryTimestampHeap.v.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0);
}

uint64_t D3DGraphics::endProfile(CommandBuffer *commandBuffer) {
    auto d3dCtx = d3d(ctx);
    auto d3dCommandList = d3d(commandBuffer)->v;
    auto queryHeap = d3dCtx->d3dQueryTimestampHeap.v.Get();
    auto timestampResultBuffer = d3dCtx->d3dTimestampResultBuffer;
    d3dCommandList->EndQuery(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 1);
    d3dCommandList->ResolveQueryData(queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, timestampResultBuffer.v.Get(), 0);
    uint64_t* t = (uint64_t*)timestampResultBuffer.map();
    const UINT64 r = t[1] - t[0];
    timestampResultBuffer.unmap();
    return r;
}

void D3DGraphics::dispatch(CommandBuffer *commandBuffer, uint32_t groupCountX,
                           uint32_t groupCountY, uint32_t groupCountZ,
                           uint32_t threadsPerGroupX, uint32_t threadsPerGroupY,
                           uint32_t threadsPerGroupZ) {
  D3D_TRACE(
      d3d(commandBuffer)->v->Dispatch(groupCountX, groupCountY, groupCountZ));
}

void D3DGraphics::draw(CommandBuffer *cmdBuffer, uint32_t vertexCount,
                       uint32_t instanceCount, uint32_t firstVertex,
                       uint32_t firstInstance) {
  D3D_TRACE(d3d(cmdBuffer)->v->DrawInstanced(vertexCount, instanceCount,
                                             firstVertex, firstInstance));
}
void D3DGraphics::drawIndexed(CommandBuffer *cmdBuffer, uint32_t indexCount,
                              uint32_t instanceCount, uint32_t firstIndex,
                              int32_t vertexOffset, uint32_t firstInstance) {
  D3D_TRACE(d3d(cmdBuffer)->v->DrawIndexedInstanced(
      indexCount, instanceCount, firstIndex, vertexOffset, firstInstance));
}
void D3DGraphics::setViewport(CommandBuffer *cmdBuffer, Rect2D r) {
  viewport = r;
  D3D12_VIEWPORT d3dViewport = {float(r.x), float(r.y), float(r.w),
                                float(r.h), 0.0f,       1.0f};
  D3D_TRACE(d3d(cmdBuffer)->v->RSSetViewports(1, &d3dViewport));
}
void D3DGraphics::setScissor(CommandBuffer *cmdBuffer, Rect2D r) {
  scissorRect = r;
#ifdef ORIGIN_BOTTOM_LEFT
  auto &v = viewport;
  D3D12_RECT d3dScissorRect = {long(r.x), long(v.h - r.y - r.h),
                               long(r.x + r.w), long(v.h - r.y)};
#else
  D3D12_RECT d3dScissorRect = {long(r.x), long(r.y), long(r.x + r.w),
                               long(r.y + r.h)};
#endif
  D3D_TRACE(d3d(cmdBuffer)->v->RSSetScissorRects(1, &d3dScissorRect));
}

Graphics *Graphics::create(GraphicsContext *ctx) {
  D3DGraphics *d3dGraphics = new D3DGraphics();
  d3dGraphics->ctx = ctx;
  d3dGraphics->create();
  return d3dGraphics;
}
