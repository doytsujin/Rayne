//
//  RNMetalUniformBuffer.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNMetalUniformBuffer.h"
#include "RNMetalInternals.h"
#include "RNMetalRenderer.h"

namespace RN
{
	RNDefineMeta(MetalUniformBuffer, Object)

	MetalUniformBuffer::MetalUniformBuffer(Renderer *renderer, size_t size, uint32 index) :
		_bufferIndex(0),
		_index(index)
	{
		for(size_t i = 0; i < kRNMetalUniformBufferCount; i++)
			_buffers[i] = renderer->CreateBufferWithLength(size, GPUResource::UsageOptions::Uniform , GPUResource::AccessOptions::WriteOnly);
	}

	MetalUniformBuffer::~MetalUniformBuffer()
	{
		for(size_t i = 0; i < kRNMetalUniformBufferCount; i++)
			_buffers[i]->Release();
	}

	GPUBuffer *MetalUniformBuffer::Advance()
	{
		_bufferIndex = (_bufferIndex + 1) % kRNMetalUniformBufferCount;
		return _buffers[_bufferIndex];
	}
}
