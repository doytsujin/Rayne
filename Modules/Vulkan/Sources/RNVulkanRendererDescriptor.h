//
//  RNVulkanRendererDescriptor.h
//  Rayne
//
//  Copyright 2016 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_VULKANRENDERERDESCRIPTOR_H_
#define __RAYNE_VULKANRENDERERDESCRIPTOR_H_

#include <Rayne.h>
#include "RNVulkanInstance.h"

namespace RN
{
	class VulkanRendererDescriptor : public RendererDescriptor
	{
	public:
		static void InitialWakeUp(MetaClass *meta);

		Renderer *CreateRenderer(RenderingDevice *device) override;
		bool CanCreateRenderer() const override;

		const Array *GetDevices() const { return _instance->GetDevices(); }
		void PrepareWithSettings(const Dictionary *settings);

	private:
		VulkanRendererDescriptor();

		VulkanInstance *_instance;

		RNDeclareMeta(VulkanRendererDescriptor)
	};
}


#endif /* __RAYNE_VULKANRENDERERDESCRIPTOR_H_ */
