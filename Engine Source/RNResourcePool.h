//
//  RNResourcePool.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_RESOURCEPOOL_H__
#define __RAYNE_RESOURCEPOOL_H__

#include "RNBase.h"
#include "RNObject.h"

#define kRNResourceKeyParticleShader "kRNResourceKeyParticleShader"

namespace RN
{
	class Kernel;
	class ResourcePool : public Singleton<ResourcePool>
	{
	friend class Kernel;
	public:
		void AddResource(Object *resource, const std::string& name);
		void RemoveResource(const std::string& name);
		
		template<typename T>
		T *ResourceWithName(const std::string& name)
		{
			return static_cast<T *>(ObjectWithName(name));
		}
		
	private:
		Object *ObjectWithName(const std::string& name);
		void __RemoveResource(const std::string& name);
		
		void LoadDefaultResources();
		
		SpinLock _lock;
		std::unordered_map<std::string, Object *> _objects;
	};
}

#endif /* __RAYNE_RESOURCEPOOL_H__ */
