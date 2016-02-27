//
//  RNVulkan.h
//  Rayne
//
//  Copyright 2016 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_VULKAN_H_
#define __RAYNE_VULKAN_H_

#include <Rayne.h>
#include <queue>
#include "RNVulkanDispatchTable.h"

#if defined(RN_BUILD_VULKAN)
	#define VKAPI RN_EXPORT
#else
	#define VKAPI RN_IMPORT
#endif

namespace RN
{
	void VulkanErrorBreak(VkResult result);
}

#define kRNVulkanRenderStages 3

#define RNVulkanValidate(expression) \
    do { \
        VkResult result = (expression); \
        if(result != VK_SUCCESS) \
        { \
            RN::VulkanErrorBreak(result); \
        } \
    } while(0)

#define RN_VULKAN_ENABLE_VALIDATION 1

#endif /* __RAYNE_VULKAN_H_ */
