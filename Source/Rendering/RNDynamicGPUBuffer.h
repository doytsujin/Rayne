//
//  RNDynamicGPUBuffer.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_DYNAMICGPUBUFFER_H_
#define __RAYNE_DYNAMICGPUBUFFER_H_

#include "../Base/RNBase.h"
#include "RNGPUBuffer.h"

namespace RN
{
	class DynamicGPUBuffer : public Object
	{
	public:
		RNAPI DynamicGPUBuffer(size_t initial, GPUResource::UsageOptions usageOption);
		RNAPI ~DynamicGPUBuffer();

		RNAPI void Resize(size_t length);

		RNAPI void Advance();

		RNAPI void *GetBuffer();
		RNAPI void Invalidate();
		RNAPI void InvalidateRange(const Range &range);
		size_t GetLength() const { return _length; }
		GPUBuffer *GetGPUBuffer() const { return _buffers[_index]; }

	private:
		GPUBuffer *_buffers[3];
		size_t _index;
		size_t _length;

		GPUResource::UsageOptions _usageOption;

		__RNDeclareMetaInternal(DynamicGPUBuffer)
	};
}


#endif /* __RAYNE_DYNAMICGPUBUFFER_H_ */
