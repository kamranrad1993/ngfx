/*
 * Copyright 2021 GoPro Inc.
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

#include "ngfx/porting/d3d/D3DSampler.h"
#include "ngfx/porting/d3d/D3DDebugUtil.h"
#include "ngfx/porting/d3d/D3DGraphicsContext.h"
using namespace ngfx;

void D3DSampler::create(D3DGraphicsContext* ctx, const D3DSamplerDesc &samplerDesc) {
	desc = samplerDesc;
	D3D_TRACE(ctx->d3dDevice.v->CreateSampler(&samplerDesc, ctx->d3dSamplerDescriptorHeap.handle.cpuHandle));
	handle = ctx->d3dSamplerDescriptorHeap.handle;
	++ctx->d3dSamplerDescriptorHeap.handle;
}