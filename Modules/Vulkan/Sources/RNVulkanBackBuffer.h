//
//  RNVulkanBackBuffer.h
//  Rayne
//
//  Copyright 2016 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_VULKANBACKBUFFER_H_
#define __RAYNE_VULKANBACKBUFFER_H_

#include "RNVulkan.h"
#include "RNVulkanDispatchTable.h"

namespace RN
{
	class VulkanBackBuffer
	{
	public:
		VKAPI VulkanBackBuffer(VkDevice device);
		VKAPI ~VulkanBackBuffer();

		VKAPI 	void WaitForPresentFence();
		VKAPI void AcquireNextImage(VkSwapchainKHR swapChain);

		uint32_t *GetImageIndex() const { return const_cast<uint32_t *>(&_imageIndex); }
		VkSemaphore *GetRenderSemaphore() const { return const_cast<VkSemaphore *>(&_renderSemaphore); }

		VkFence GetPresentFence() const { return _presentFence; }

	private:
		VkDevice _device;
		VkSemaphore _acquireSemaphore;
		VkSemaphore _renderSemaphore;
		VkFence _presentFence;

		uint32_t _imageIndex;
	};
}


#endif /* __RAYNE_VULKANBACKBUFFER_H_ */
