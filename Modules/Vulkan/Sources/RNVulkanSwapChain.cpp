//
//  RNVulkanSwapChain.cpp
//  Rayne
//
//  Copyright 2018 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNVulkanSwapChain.h"
#include "RNVulkanRenderer.h"
#include "RNVulkanFramebuffer.h"
#include "RNVulkanInternals.h"

namespace RN
{
	RNDefineMeta(VulkanSwapChain, Object)

	static VkFormat SwapChainFormatFromTextureFormat(Texture::Format format)
	{
		switch(format)
		{
		case Texture::Format::RGBA8888SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case Texture::Format::BGRA8888SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
		case Texture::Format::RGBA8888:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case Texture::Format::BGRA8888:
			return VK_FORMAT_B8G8R8A8_UNORM;
//		case Texture::Format::RGB10A2:
//			return DXGI_FORMAT_R10G10B10A2_UNORM;
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}


VulkanSwapChain::VulkanSwapChain(const Vector2& size, HWND hwnd, VulkanRenderer* renderer, const Window::SwapChainDescriptor& descriptor) :
		_renderer(renderer),
		_frameIndex(0),
		_semaphoreIndex(0),
		_size(size),
		_descriptor(descriptor),
		_hwnd(hwnd),
		_surface(VK_NULL_HANDLE),
		_swapchain(VK_NULL_HANDLE),
		_framebuffer(nullptr)
		//_fenceValues(nullptr)
	{
		_device = _renderer->GetVulkanDevice()->GetDevice();

		CreateSurface();
		CreateSwapChain();
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		//delete[] _fenceValues;

		SafeRelease(_framebuffer);
	}

	void VulkanSwapChain::CreateSurface()
	{
		VulkanDevice *device = _renderer->GetVulkanDevice();
		VulkanInstance *instance = _renderer->GetVulkanInstance();

#if RN_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = ::GetModuleHandle(nullptr);
		surfaceInfo.hwnd = _hwnd;

		RNVulkanValidate(vk::CreateWin32SurfaceKHR(instance->GetInstance(), &surfaceInfo, nullptr, &_surface));
#endif
#if RN_PLATFORM_LINUX
		VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext = nullptr;
		surfaceInfo.flags = 0;
		surfaceInfo.connection = Kernel::GetSharedInstance()->GetXCBConnection();
		surfaceInfo.window = _window;

		RNVulkanValidate(vk::CreateXcbSurfaceKHR(instance->GetInstance(), &surfaceInfo, nullptr, &_surface));
#endif

		VkBool32 surfaceSupported;
		vk::GetPhysicalDeviceSurfaceSupportKHR(device->GetPhysicalDevice(), 0, _surface, &surfaceSupported);

		RN_ASSERT(surfaceSupported == VK_TRUE, "VkSurface unsupported!");

		std::vector<VkSurfaceFormatKHR> formats;
		device->GetSurfaceFormats(_surface, formats);

		_format = formats.at(0);
		VkFormat requestedFormat = SwapChainFormatFromTextureFormat(_descriptor.colorFormat);
		for(const VkSurfaceFormatKHR &format : formats)
		{
			if(format.format == requestedFormat)
			{
				_format = format;
				break;
			}
		}

		_extents.width = static_cast<uint32_t>(-1);
		_extents.height = static_cast<uint32_t>(-1);
	}

	void VulkanSwapChain::CreateSwapChain()
	{
		if(_framebuffer)
		{
			_framebuffer->WillUpdateSwapChain();
		}

		VulkanDevice *device = _renderer->GetVulkanDevice();

		VkSurfaceCapabilitiesKHR caps;
		RNVulkanValidate(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(device->GetPhysicalDevice(), _surface, &caps));

		VkExtent2D extent = caps.currentExtent;

		if(extent.width == static_cast<uint32_t>(-1))
		{
			extent.width = static_cast<uint32_t>(_size.x);
			extent.height = static_cast<uint32_t>(_size.y);
		}

		extent.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, extent.width));
		extent.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, extent.height));

		if(_extents.width == extent.width && _extents.height == extent.height)
			return;

		uint32_t imageCount = std::max(caps.minImageCount, std::min(caps.maxImageCount, static_cast<uint32_t>(_descriptor.bufferCount)));

		assert(caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
		assert(caps.supportedTransforms & caps.currentTransform);
		assert(caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));

		VkCompositeAlphaFlagBitsKHR compositeAlpha = (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ? VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		std::vector<VkPresentModeKHR> modes;
		device->GetPresentModes(_surface, modes);

		const bool useVSync = true;

		VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR; // FIFO is the only mode universally supported
/*		for(auto m : modes)
		{
			if((useVSync && m == VK_PRESENT_MODE_MAILBOX_KHR) || (!useVSync && m == VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				mode = m;
				break;
			}
		}*/

		VkSwapchainCreateInfoKHR swapchainInfo = {};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = _surface;
		swapchainInfo.minImageCount = imageCount;
		swapchainInfo.imageFormat = _format.format;
		swapchainInfo.imageColorSpace = _format.colorSpace;
		swapchainInfo.imageExtent = extent;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.preTransform = caps.currentTransform;
		swapchainInfo.compositeAlpha = compositeAlpha;
		swapchainInfo.presentMode = mode;
		swapchainInfo.clipped = VK_TRUE;
		swapchainInfo.oldSwapchain = _swapchain;

		RNVulkanValidate(vk::CreateSwapchainKHR(device->GetDevice(), &swapchainInfo, nullptr, &_swapchain));
		_extents = extent;

		uint32_t count = _descriptor.bufferCount;
		vk::GetSwapchainImagesKHR(device->GetDevice(), _swapchain, &count, nullptr);
		vk::GetSwapchainImagesKHR(device->GetDevice(), _swapchain, &count, _colorBuffers);
		_descriptor.bufferCount = static_cast<uint8>(count);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		for(size_t i = _presentSemaphores.size(); i < count; i++)
		{
			VkSemaphore presentSemaphore;
			VkSemaphore renderSemaphore;
			RNVulkanValidate(vk::CreateSemaphore(_device, &semaphoreInfo, nullptr, &presentSemaphore));
			RNVulkanValidate(vk::CreateSemaphore(_device, &semaphoreInfo, nullptr, &renderSemaphore));
			_presentSemaphores.push_back(presentSemaphore);
			_renderSemaphores.push_back(renderSemaphore);
		}

		// Destroy the old swapchain
		if(swapchainInfo.oldSwapchain != VK_NULL_HANDLE)
		{
			RNVulkanValidate(vk::DeviceWaitIdle(device->GetDevice()));
			vk::DestroySwapchainKHR(device->GetDevice(), swapchainInfo.oldSwapchain, nullptr);
		}

		if(!_framebuffer)
		{
			_framebuffer = new VulkanFramebuffer(_size, this, _renderer, _descriptor.colorFormat, _descriptor.depthStencilFormat);
		}
		else
		{
			_framebuffer->DidUpdateSwapChain(_size, _descriptor.colorFormat, _descriptor.depthStencilFormat);
		}
	};

	void VulkanSwapChain::ResizeSwapchain(const Vector2& size)
	{
		_newSize = size;
	}

	void VulkanSwapChain::SetFullscreen(bool fullscreen) const
	{
/*		if(FAILED(_swapChain->SetFullscreenState(!fullscreen, nullptr)))
		{
			// Transitions to fullscreen mode can fail when running apps over
			// terminal services or for some other unexpected reason.  Consider
			// notifying the user in some way when this happens.
			RN_ASSERT(false, "Failed changing from/to fullscreen.");
		}*/
	}

	Vector2 VulkanSwapChain::GetSize() const
	{
		return _size;
	}

	VkImage VulkanSwapChain::GetVulkanColorBuffer(int i) const
	{
		return _colorBuffers[i];
	}

	VkImage VulkanSwapChain::GetVulkanDepthBuffer(int i) const
	{
		return nullptr;
	}

	void VulkanSwapChain::AcquireBackBuffer()
	{
		_semaphoreIndex += 1;
		_semaphoreIndex %= _descriptor.bufferCount;
		RNVulkanValidate(vk::AcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _presentSemaphores[_semaphoreIndex], VK_NULL_HANDLE, &_frameIndex));

		//Update the swap chain and frame buffer size if the window size changed
/*		if(_newSize.GetLength() > 0.001f)
		{
			if(_size.GetSquaredDistance(_newSize) > 0.001f)
			{
				_size = _newSize;

				//Wait for rendering to all currently bound buffers is finished
				UINT completedFenceValue = _fence->GetCompletedValue();
				if(completedFenceValue < _fenceValues[_frameIndex])
				{
					_fence->SetEventOnCompletion(_fenceValues[_frameIndex], _fenceEvent);
					WaitForSingleObjectEx(_fenceEvent, INFINITE, false);
				}

				//Do the actual resize and notify the framebuffer about it
				CreateSwapChain();

				_frameIndex = _swapChain->GetCurrentBackBufferIndex();
			}

			_newSize = Vector2();
		}*/
	}

	void VulkanSwapChain::Prepare(VkCommandBuffer commandBuffer)
	{
		VkImageMemoryBarrier postPresentBarrier = {};
		postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		postPresentBarrier.pNext = NULL;
		postPresentBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		postPresentBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		postPresentBarrier.image = static_cast<VulkanTexture *>(_framebuffer->GetColorTexture(_frameIndex))->GetVulkanImage();
		vk::CmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &postPresentBarrier);
	}

	void VulkanSwapChain::Finalize(VkCommandBuffer commandBuffer)
	{
		// Add a present memory barrier to the end of the command buffer
		// This will transform the frame buffer color attachment to a
		// new layout for presenting it to the windowing system integration
		VkImageMemoryBarrier prePresentBarrier = {};
		prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		prePresentBarrier.pNext = NULL;
		prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		prePresentBarrier.image = static_cast<VulkanTexture *>(_framebuffer->GetColorTexture(_frameIndex))->GetVulkanImage();

		vk::CmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &prePresentBarrier);
	}

	void VulkanSwapChain::PresentBackBuffer(VkQueue queue)
	{
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &_renderSemaphores[_semaphoreIndex];
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &_swapchain;
		present_info.pImageIndices = &_frameIndex;

		RNVulkanValidate(vk::QueuePresentKHR(queue, &present_info));
	}
}
