//
//  RNResourcePool.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNResourcePool.h"
#include "RNShader.h"

namespace RN
{
	void ResourcePool::LoadDefaultResources()
	{
		AddResource(Shader::WithFile("shader/rn_Texture1"), kRNResourceKeyTexture1Shader);
		AddResource(Shader::WithFile("shader/rn_Texture2"), kRNResourceKeyTexture2Shader);
		
		AddResource(Shader::WithFile("shader/rn_Texture1Normal"), kRNResourceKeyTexture1NormalShader);
		
		AddResource(Shader::WithFile("shader/rn_Color1"), kRNResourceKeyColor1Shader);
		AddResource(Shader::WithFile("shader/rn_Color2"), kRNResourceKeyColor2Shader);
		
		AddResource(Shader::WithFile("shader/rn_Particle"), kRNResourceKeyParticleShader);
		
		AddResource(Shader::WithFile("shader/rn_LightTileSampleFirst"), kRNResourceKeyLightTileSampleFirstShader);
		AddResource(Shader::WithFile("shader/rn_LightTileSample"), kRNResourceKeyLightTileSampleShader);
		AddResource(Shader::WithFile("shader/rn_LightDepth"), kRNResourceKeyLightDepthShader);
	}
	
	
	
	void ResourcePool::AddResource(Object *resource, const std::string& name)
	{
		_lock.Lock();
		
		__RemoveResource(name);
		if(resource)
		{
			_objects.insert(std::unordered_map<std::string, Object *>::value_type(name, resource));
			resource->Retain();
		}
		
		_lock.Unlock();
	}
	
	void ResourcePool::RemoveResource(const std::string& name)
	{
		_lock.Lock();
		__RemoveResource(name);
		_lock.Unlock();
	}
	
	void ResourcePool::__RemoveResource(const std::string& name)
	{
		auto iterator = _objects.find(name);
		if(iterator != _objects.end())
		{
			Object *object = iterator->second;
			object->Release();
			
			_objects.erase(iterator);
		}
	}
	
	Object *ResourcePool::ObjectWithName(const std::string& name)
	{
		_lock.Lock();
		
		auto iterator = _objects.find(name);
		Object *object = (iterator != _objects.end()) ? iterator->second : 0;
		
		_lock.Unlock();
		
		return object;
	}
}
