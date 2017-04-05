//
//  RNMetalSwapChain.cpp
//  Rayne
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNMetalSwapChain.h"
#include "RNMetalInternals.h"

namespace RN
{
	RNDefineMeta(MetalSwapChain, Object)

	MetalSwapChain::MetalSwapChain(const Vector2 size, id<MTLDevice> device) : _drawable(nullptr)
	{
		_metalView = [[RNMetalView alloc] initWithFrame:NSMakeRect(0, 0, size.x, size.y) andDevice:device];
		CGSize realSize = [_metalView getSize];

		Framebuffer::Descriptor descriptor;
		descriptor.options = Framebuffer::Options::PrivateStorage;
		descriptor.colorFormat = Texture::Format::RGBA8888;
		descriptor.depthFormat = Texture::Format::Depth24Stencil8;
		_framebuffer = new MetalFramebuffer(Vector2(realSize.width, realSize.height), descriptor, this);
	}

	MetalSwapChain::~MetalSwapChain()
	{

	}

	Vector2 MetalSwapChain::GetSize() const
	{
		return Vector2([_metalView frame].size.width, [_metalView frame].size.height);
	}

	void MetalSwapChain::AcquireBackBuffer()
	{
		_drawable = [_metalView nextDrawable];
	}
	void MetalSwapChain::Prepare()
	{

	}
	void MetalSwapChain::Finalize()
	{

	}
	void MetalSwapChain::PresentBackBuffer(id<MTLCommandBuffer> commandBuffer)
	{
		[commandBuffer presentDrawable:_drawable];
	}

	id MetalSwapChain::GetMetalDrawable() const
	{
		return _drawable;
	}
}