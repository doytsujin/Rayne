//
//  RNVulkanInstance.cpp
//  Rayne
//
//  Copyright 2016 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNVulkanInstance.h"
#include "RNVulkanDevice.h"

namespace RN
{
	VulkanInstance::VulkanInstance() :
		_instance(nullptr),
		_module(nullptr),
		_allocationCallbacks(nullptr),
		_devices(nullptr)
	{
		_requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		_requiredDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	VulkanInstance::~VulkanInstance()
	{
		if(_instance)
			vk::DestroyInstance(_instance, _allocationCallbacks);

		if(_module)
			::FreeModule(_module);

		SafeRelease(_devices);
	}

	bool VulkanInstance::LoadVulkan()
	{
		if(_instance)
			return true;

		if(!_module)
		{
			// Load the module and verify that we have vkGetInstanceProcAddr() available
			_module = ::LoadLibrary("vulkan-1.dll");

			PFN_vkGetInstanceProcAddr procAddr = nullptr;

			if(_module)
				procAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(::GetProcAddress(_module, "vkGetInstanceProcAddr"));

			if(!_module || !procAddr)
			{
				RNError("Couldn't load Vulkan library");

				if(_module)
				{
					::FreeLibrary(_module);
					_module = nullptr;
				}

				return false;
			}
		}

		// Create a Vulkan Instance
		PFN_vkGetInstanceProcAddr procAddr = nullptr;

		if(_module)
			procAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(::GetProcAddress(_module, "vkGetInstanceProcAddr"));

		vk::init_dispatch_table_top(procAddr);

		// TODO: Verify extensions

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = Kernel::GetSharedInstance()->GetApplication()->GetTitle()->GetUTF8String();
		appInfo.applicationVersion = 0;
		appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledExtensionCount = static_cast<uint32_t>(_requiredExtensions.size());
		instanceInfo.ppEnabledExtensionNames = _requiredExtensions.data();

		VkResult result = vk::CreateInstance(&instanceInfo, _allocationCallbacks, &_instance);
		if(result != VK_SUCCESS)
		{
			RNDumpVulkanResult(result);
			_instance = nullptr;

			return false;
		}

		vk::init_dispatch_table_middle(_instance, false);

		// Load the list of devices
		{
			_devices = new Array();

			std::vector<VkPhysicalDevice> devices;
			if(EnumerateDevicesWithExtensions(devices, _requiredDeviceExtensions) == VK_SUCCESS)
			{
				for(auto device : devices)
				{
					VulkanDevice *temp = new VulkanDevice(this, device);
					if(temp->IsValidDevice())
						_devices->AddObject(temp);

					temp->Release();
				}
			}
		}

		return true;
	}

	VkResult VulkanInstance::EnumerateExtensions(const char *layer, std::vector<VkExtensionProperties> &extensions) const
	{
		uint32_t count = 0;
		vk::EnumerateInstanceExtensionProperties(layer, &count, nullptr);

		extensions.resize(count);
		return vk::EnumerateInstanceExtensionProperties(layer, &count, extensions.data());
	}

	VkResult VulkanInstance::EnumerateDeviceExtensions(VkPhysicalDevice device, const char *layer, std::vector<VkExtensionProperties> &extensions) const
	{
		uint32_t count = 0;
		vk::EnumerateDeviceExtensionProperties(device, layer, &count, nullptr);

		extensions.resize(count);
		return vk::EnumerateDeviceExtensionProperties(device, layer, &count, extensions.data());
	}



	VkResult VulkanInstance::EnumerateDevices(std::vector<VkPhysicalDevice> &devices) const
	{
		uint32_t count = 0;
		vk::EnumeratePhysicalDevices(_instance, &count, nullptr);

		devices.resize(count);
		return vk::EnumeratePhysicalDevices(_instance, &count, devices.data());
	}

	VkResult VulkanInstance::EnumerateDevicesWithExtensions(std::vector<VkPhysicalDevice> &devices, const std::vector<const char *> &extensions) const
	{
		VkResult result = EnumerateDevices(devices);
		if(result == VK_SUCCESS)
		{
			for(auto iterator = devices.begin(); iterator != devices.end();)
			{
				if(!DeviceSupportsExtensions(*iterator, extensions))
				{
					iterator = devices.erase(iterator);
					continue;
				}

				iterator ++;
			}
		}

		return result;
	}

	bool VulkanInstance::DeviceSupportsExtensions(VkPhysicalDevice device, const std::vector<const char *> &extensions) const
	{
		std::vector<VkExtensionProperties> rawDeviceExtensions;
		EnumerateDeviceExtensions(device, nullptr, rawDeviceExtensions);

		std::unordered_set<std::string> deviceExtensions;
		for (const auto &extension : rawDeviceExtensions)
			deviceExtensions.insert(extension.extensionName);

		for(const auto &name : extensions)
		{
			if(deviceExtensions.find(name) == deviceExtensions.end())
				return false;
		}

		return true;
	}


	String *GetStringForVulkanResult(VkResult result)
	{
		switch(result)
		{
			case VK_SUCCESS:
				return RNCSTR("Success");
			case VK_NOT_READY:
				return RNCSTR("Not Ready");
			case VK_TIMEOUT:
				return RNCSTR("Timeout");
			case VK_EVENT_SET:
				return RNCSTR("Event Set");
			case VK_EVENT_RESET:
				return RNCSTR("Event Reset");
			case VK_INCOMPLETE:
				return RNCSTR("Incomplete");

			default:
				return RNSTR("Unknown (" << result << ")");
		}
	}
}
