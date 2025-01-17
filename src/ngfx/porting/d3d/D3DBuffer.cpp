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
#include "ngfx/porting/d3d/D3DBuffer.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
#include "ngfx/porting/d3d/D3DReadbackBuffer.h"
#include <cassert>
#include <d3dx12.h>
using namespace ngfx;

void D3DBuffer::create(D3DGraphicsContext *ctx, const void *data, uint32_t size,
                       BufferUsageFlags bufferUsageFlags) {
  if (bufferUsageFlags & BUFFER_USAGE_TRANSFER_SRC_BIT)
    heapType = D3D12_HEAP_TYPE_UPLOAD;
  else
    heapType = D3D12_HEAP_TYPE_DEFAULT;
  D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
  if (bufferUsageFlags & BUFFER_USAGE_STORAGE_BUFFER_BIT)
    resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  create(ctx, data, size, heapType, resourceFlags);
}

void D3DBuffer::create(D3DGraphicsContext *ctx, const void *data, uint32_t size,
                       D3D12_HEAP_TYPE heapType,
                       D3D12_RESOURCE_FLAGS resourceFlags,
                       D3D12_RESOURCE_STATES initialResourceState) {
  HRESULT hResult;
  this->ctx = ctx;
  auto &device = ctx->d3dDevice.v;
  this->size = size;
  this->heapType = heapType;
  this->initialResourceState = initialResourceState;
  CD3DX12_HEAP_PROPERTIES heapProperties(heapType);
  CD3DX12_RESOURCE_DESC resourceDesc =
      CD3DX12_RESOURCE_DESC::Buffer(size, resourceFlags);
  V(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
                                    &resourceDesc, initialResourceState,
                                    nullptr, IID_PPV_ARGS(&v)));
  currentResourceState = initialResourceState;
  if (data)
    upload(data, size, 0);
}

D3DBuffer::~D3DBuffer() {}

// TODO: add read/write flags for map
void *D3DBuffer::map() {
  if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
    assert(d3dReadbackBuffer == nullptr);
    auto &copyCommandList = ctx->d3dCopyCommandList;
    copyCommandList.begin();
    d3dReadbackBuffer = new D3DReadbackBuffer();
    d3dReadbackBuffer->create(ctx, size);
    if (currentResourceState != D3D12_RESOURCE_STATE_COPY_SOURCE) {
      CD3DX12_RESOURCE_BARRIER resourceBarrier =
          CD3DX12_RESOURCE_BARRIER::Transition(
              v.Get(), currentResourceState, D3D12_RESOURCE_STATE_COPY_SOURCE);
      D3D_TRACE(copyCommandList.v.Get()->ResourceBarrier(1, &resourceBarrier));
    }
    D3D_TRACE(copyCommandList.v.Get()->CopyBufferRegion(
        d3dReadbackBuffer->v.Get(), 0, v.Get(), 0, size));
    CD3DX12_RESOURCE_BARRIER resourceBarrier =
        CD3DX12_RESOURCE_BARRIER::Transition(v.Get(),
                                             D3D12_RESOURCE_STATE_COPY_SOURCE,
                                             D3D12_RESOURCE_STATE_GENERIC_READ);
    D3D_TRACE(copyCommandList.v.Get()->ResourceBarrier(1, &resourceBarrier));
    copyCommandList.end();
    ctx->d3dCommandQueue.submit(&copyCommandList);
    ctx->d3dCommandQueue.waitIdle();
    d3dReadBackBufferPtr = d3dReadbackBuffer->map();
    return d3dReadBackBufferPtr;
  } else {
    UINT8 *ptr;
    HRESULT hResult;
    V(v->Map(0, nullptr, reinterpret_cast<void **>(&ptr)));
    return ptr;
  }
}

void D3DBuffer::unmap() {
  if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
    assert(d3dReadbackBuffer);
    upload(d3dReadBackBufferPtr, size, 0);
    d3dReadbackBuffer->unmap();
    delete d3dReadbackBuffer;
    d3dReadbackBuffer = nullptr;
    d3dReadBackBufferPtr = nullptr;
  } else {
    D3D_TRACE(v->Unmap(0, nullptr));
  }
}

void D3DBuffer::upload(const void *data, uint32_t size, uint32_t offset) {
  if (heapType == D3D12_HEAP_TYPE_DEFAULT) {
    auto &copyCommandList = ctx->d3dCopyCommandList;
    copyCommandList.begin();
    D3DBuffer stagingBuffer;
    if (data) {
      stagingBuffer.create(ctx, data, size, D3D12_HEAP_TYPE_UPLOAD);
      if (currentResourceState != D3D12_RESOURCE_STATE_COPY_DEST) {
        CD3DX12_RESOURCE_BARRIER resourceBarrier =
            CD3DX12_RESOURCE_BARRIER::Transition(
                v.Get(), currentResourceState, D3D12_RESOURCE_STATE_COPY_DEST);
        D3D_TRACE(
            copyCommandList.v.Get()->ResourceBarrier(1, &resourceBarrier));
      }
      D3D_TRACE(copyCommandList.v.Get()->CopyBufferRegion(
          v.Get(), 0, stagingBuffer.v.Get(), 0, size));
    }
    CD3DX12_RESOURCE_BARRIER resourceBarrier =
        CD3DX12_RESOURCE_BARRIER::Transition(v.Get(),
                                             D3D12_RESOURCE_STATE_COPY_DEST,
                                             D3D12_RESOURCE_STATE_GENERIC_READ);
    D3D_TRACE(copyCommandList.v.Get()->ResourceBarrier(1, &resourceBarrier));
    copyCommandList.end();
    ctx->d3dCommandQueue.submit(&copyCommandList);
    ctx->d3dCommandQueue.waitIdle();
    currentResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
  } else {
    uint8_t *dst = (uint8_t *)map();
    memcpy(dst + offset, data, size);
    unmap();
  }
}

Buffer *Buffer::create(GraphicsContext *ctx, const void *data, uint32_t size,
                       BufferUsageFlags usageFlags) {
  D3DBuffer *d3dBuffer = new D3DBuffer();
  d3dBuffer->create(d3d(ctx), data, size, usageFlags);
  return d3dBuffer;
}
