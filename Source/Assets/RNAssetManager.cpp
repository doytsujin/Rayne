//
//  RNAssetManager.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "../Debug/RNLogger.h"
#include "RNAssetManager.h"
#include "RNAssetManagerInternals.h"

#include "RNPNGAssetLoader.h"
#include "RNSGMAssetLoader.h"

namespace RN
{
	static AssetManager *__sharedInstance = nullptr;

	AssetManager::AssetManager() :
		_loaders(new Array()),
		_requests(new Dictionary()),
		_resources(new Dictionary())
	{
		__sharedInstance = this;

		PNGAssetLoader::Register();
		SGMAssetLoader::Register();
	}
	AssetManager::~AssetManager()
	{
		SafeRelease(_loaders);
		SafeRelease(_requests);
		SafeRelease(_resources);

		__sharedInstance = nullptr;
	}

	AssetManager *AssetManager::GetSharedInstance()
	{
		return __sharedInstance;
	}


	void AssetManager::RegisterAssetLoader(AssetLoader *loader)
	{
		std::lock_guard<std::mutex> lock(_lock);

		_loaders->AddObject(loader);
		_loaders->Sort<AssetLoader>([] (const AssetLoader *loader1, const AssetLoader *loader2) -> bool {

			uint32 priority1 = loader1->GetPriority();
			uint32 priority2 = loader2->GetPriority();

			return priority1 < priority2;
		});

		UpdateMagicSize();
	}

	void AssetManager::UnregisterAssetLoader(AssetLoader *loader)
	{
		std::lock_guard<std::mutex> lock(_lock);
		_loaders->RemoveObject(loader);

		UpdateMagicSize();
	}

	void AssetManager::UpdateMagicSize()
	{
		_maxMagicSize = 0;
		_loaders->Enumerate<AssetLoader>([&](AssetLoader *loader, size_t index, bool &stop) {

			if(loader->_magicBytes)
				_maxMagicSize = std::max(loader->_magicBytes->GetLength(), _maxMagicSize);

		});
	}



	Asset *AssetManager::ValidateAsset(MetaClass *base, Asset *asset)
	{
		if(!asset->IsKindOfClass(base))
			throw InconsistencyException(RNSTR("Failed to validate asset for class " << base->GetName()));

		return asset;
	}
	void AssetManager::PrepareAsset(Asset *asset, String *name, MetaClass *meta, Dictionary *settings)
	{
		asset->__AwakeWithCoordinator(this, name, meta);

		Array *resources = _resources->GetObjectForKey<Array>(name);

		if(!resources)
		{
			resources = new Array();
			_resources->SetObjectForKey(resources, name);
			resources->Release();
		}

		LoadedAsset *wrapper = new LoadedAsset(asset, meta);
		resources->AddObject(wrapper);
		wrapper->Release();

		RNDebug("Loaded asset " << asset);
	}
	void AssetManager::__RemoveAsset(Asset *asset, String *name)
	{
		{
			std::unique_lock<std::mutex> lock(_lock);

			Array *resources = _resources->GetObjectForKey<Array>(name);
			size_t count = resources->GetCount();

			for(size_t i = 0; i < count; i ++)
			{
				LoadedAsset *wrapper = static_cast<LoadedAsset *>(resources->GetObjectAtIndex(i));

				if(wrapper->GetMeta() == asset->_meta)
				{
					resources->RemoveObjectAtIndex(i);
					break;
				}
			}

			if(resources->GetCount() == 0)
				_resources->RemoveObjectForKey(name);
		}

		RNDebug("Unloaded asset " << asset);
	}


	std::shared_future<StrongRef<Asset>> AssetManager::__GetFutureMatching(MetaClass *base, String *name)
	{
		// Check if the resource is already loaded
		Asset *asset = nullptr;

		{
			Array *resources = _resources->GetObjectForKey<Array>(name);
			if(resources)
			{
				size_t count = resources->GetCount();

				for(size_t i = 0; i < count; i ++)
				{
					LoadedAsset *wrapper = static_cast<LoadedAsset *>(resources->GetObjectAtIndex(i));

					if(base->InheritsFromClass(wrapper->GetMeta()))
					{
						asset = wrapper->GetAsset();
						if(!asset)
							resources->RemoveObjectAtIndex(i);

						break;
					}
				}
			}
		}

		if(asset)
		{
			std::promise<StrongRef<Asset>> promise;
			std::shared_future<StrongRef<Asset>> future = promise.get_future().share();

			try
			{
				asset = ValidateAsset(base, asset);
				promise.set_value(asset);
			}
			catch(Exception &)
			{
				promise.set_exception(std::current_exception());
			}

			return future;
		}

		// Check if there is a pending request for the resource
		Array *requests = _requests->GetObjectForKey<Array>(name);
		if(requests)
		{
			size_t count = requests->GetCount();

			for(size_t i = 0; i < count; i ++)
			{
				PendingAsset *wrapper = static_cast<PendingAsset *>(requests->GetObjectAtIndex(i));

				if(base->InheritsFromClass(wrapper->GetMeta()))
					return wrapper->GetFuture();
			}
		}

		throw InconsistencyException("No matching future found");
	}

	Asset *AssetManager::__GetAssetWithName(MetaClass *base, const String *tname, const Dictionary *tsettings)
	{
		String *name = tname->GetNormalizedPath()->Retain();
		std::unique_lock<std::mutex> lock(_lock);

		try
		{
			std::shared_future<StrongRef<Asset>> future = __GetFutureMatching(base, name);
			name->Release();

			lock.unlock();
			future.wait();
			return future.get();
		}
		catch(InconsistencyException &)
		{}

		// Load the resource
		File *file;
		Dictionary *settings = SafeCopy(tsettings);

		try
		{
			file = File::WithName(name, File::Mode::Read);
		}
		catch(Exception &)
		{
			file = nullptr;
		}

		AssetLoader *loader = PickAssetLoader(base, file, name, true);

		std::promise<StrongRef<Asset>> promise;

		Array *requests = _requests->GetObjectForKey<Array>(name);
		if(!requests)
		{
			requests = new Array();
			_requests->SetObjectForKey(requests, name);
			requests->Release();
		}

		PendingAsset *wrapper = new PendingAsset(std::move(promise.get_future().share()), base);
		requests->AddObject(wrapper);
		wrapper->Release();

		lock.unlock();


		if(!settings)
			settings = new Dictionary();

		Object *fileOrName = file ? static_cast<Object *>(file) : static_cast<Object *>(name);
		Expected<Asset *> asset = loader->__Load(fileOrName, base, settings);

		settings->Release();

		lock.lock();

		requests->RemoveObject(wrapper);

		if(requests->GetCount() == 0)
			_requests->RemoveObjectForKey(name);

		if(!asset.IsValid())
		{
			promise.set_exception(asset.GetException());
			std::rethrow_exception(asset.GetException());
		}

		PrepareAsset(asset.Get(), name, base, settings);
		lock.unlock();

		name->Release();

		return asset.Get();
	}

	std::shared_future<StrongRef<Asset>> AssetManager::__GetFutureAssetWithName(MetaClass *base, const String *tname, const Dictionary *tsettings)
	{
		String *name = tname->GetNormalizedPath()->Retain();
		std::unique_lock<std::mutex> lock(_lock);

		try
		{
			std::shared_future<StrongRef<Asset>> future = __GetFutureMatching(base, name);
			name->Release();

			return future;
		}
		catch(InconsistencyException &)
		{}

		// Load the resource
		File *file;
		Dictionary *settings = SafeCopy(tsettings);

		try
		{
			file = File::WithName(name, File::Mode::Read);
		}
		catch(Exception &)
		{
			file = nullptr;
		}

		AssetLoader *loader = PickAssetLoader(base, file, name, true);

		if(!settings)
			settings = new Dictionary();

		Object *fileOrName = file ? static_cast<Object *>(file) : static_cast<Object *>(name);

		std::future<StrongRef<Asset>> future = std::move(loader->LoadInBackground(fileOrName, base, settings, [this, name, settings, base](Asset *result) {

			std::unique_lock<std::mutex> lock(_lock);

			Array *requests = _requests->GetObjectForKey<Array>(name);
			if(requests)
			{
				size_t count = requests->GetCount();

				for(size_t i = 0; i < count; i ++)
				{
					PendingAsset *wrapper = static_cast<PendingAsset *>(requests->GetObjectAtIndex(i));

					if(base == wrapper->GetMeta())
					{
						requests->RemoveObjectAtIndex(i);
						break;
					}
				}
			}

			if(!result)
			{
				lock.unlock();

				name->Release();
				settings->Release();

				return;
			}

			PrepareAsset(result, name, base, settings);
			lock.unlock();

			name->Release();
			settings->Release();
			result->Autorelease();

		}));


		std::shared_future<StrongRef<Asset>> shared = future.share();

		Array *requests = _requests->GetObjectForKey<Array>(name);
		if(!requests)
		{
			requests = new Array();
			_requests->SetObjectForKey(requests, name);
			requests->Release();
		}

		std::shared_future<StrongRef<Asset>> copy(shared);

		PendingAsset *wrapper = new PendingAsset(std::move(copy), base);
		requests->AddObject(wrapper);
		wrapper->Release();

		return shared;
	}


	AssetLoader *AssetManager::PickAssetLoader(MetaClass *base, File *file, const String *name, bool requiresBackgroundSupport)
	{
		AssetLoader *assetLoader = nullptr;

		if(file)
		{
			uint8 *buffer = (_maxMagicSize > 0) ? new uint8[_maxMagicSize] : nullptr;
			uint8 *magic = (_maxMagicSize > 0) ? new uint8[_maxMagicSize] : nullptr;

			String *extension = file->GetPath()->GetPathExtension();

			size_t magicSize = 0;

			if(buffer)
			{
				magicSize = file->Read(magic, _maxMagicSize);
				file->Seek(0);
			}

			_loaders->Enumerate<AssetLoader>([&](AssetLoader *loader, size_t index, bool &stop) {

				try
				{
					if(!loader->SupportsResourceClass(base))
						return;

					if(requiresBackgroundSupport && !loader->_supportsBackgroundLoading)
						return;

					if(loader->_fileExtensions->GetCount() > 0 && !loader->_fileExtensions->ContainsObject(extension))
						return;

					if(loader->_magicBytes)
					{
						size_t offset = loader->_magicBytesOffset;
						size_t size = loader->_magicBytes->GetLength();

						if(offset > 0)
						{
							file->Seek(offset, true);
							size = file->Read(buffer, size);
							file->Seek(0, true);
						}
						else
						{
							std::copy(magic, magic + size, buffer);
						}

						if(memcmp(buffer, loader->_magicBytes->GetBytes(), size) != 0)
							return;
					}

					bool result = loader->SupportsLoadingFile(file);
					file->Seek(0, true);

					if(result)
					{
						assetLoader = loader;
						stop = true;

						return;
					}
				}
				catch(Exception &)
				{
					file->Seek(0, true);
				}

			});

			delete[] buffer;
			delete[] magic;
		}
		else if(name)
		{
			String *extension = name->GetPathExtension();

			_loaders->Enumerate<AssetLoader>([&](AssetLoader *loader, size_t index, bool &stop) {

				try
				{
					if(!loader->SupportsResourceClass(base))
						return;

					if(requiresBackgroundSupport && !loader->_supportsBackgroundLoading)
						return;

					if(!loader->_supportsVirtualFiles)
						return;

					if(extension && loader->_fileExtensions->GetCount() > 0 && !loader->_fileExtensions->ContainsObject(extension))
						return;

					if(loader->SupportsLoadingName(name))
					{
						assetLoader = loader;
						stop = true;

						return;
					}
				}
				catch(Exception &)
				{}

			});
		}

		if(!assetLoader)
		{
			if(file)
				throw InconsistencyException(RNSTR("No resource loader matching " << name << " (" << file->GetPath() << ")"));

			throw InconsistencyException(RNSTR("No resource loader matching " << name << " (couldn't resolve name to a path)"));
		}

		return assetLoader;
	}
}
