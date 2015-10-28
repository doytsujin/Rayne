//
//  RNAssetLoader.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "../Objects/RNAutoreleasePool.h"
#include "../Threads/RNWorkQueue.h"
#include "RNAssetLoader.h"

namespace RN
{
	RNDefineMeta(AssetLoader, Object)

	AssetLoader::AssetLoader(const Config &config) :
		_magicBytes(nullptr),
		_magicBytesOffset(0),
		_fileExtensions(nullptr),
		_resourceClass(config.resourceClass),
		_supportsBackgroundLoading(config.supportsBackgroundLoading),
		_supportsVirtualFiles(config.supportsVirtualFiles),
		_priority(config.priority)
	{
		_magicBytes = SafeRetain(config._magicBytes);
		_magicBytesOffset = config._magicBytesOffset;
		_fileExtensions = SafeRetain(config._extensions);
	}

	AssetLoader::~AssetLoader()
	{
		SafeRelease(_magicBytes);
		SafeRelease(_fileExtensions);
	}


	Asset *AssetLoader::Load(File *file, Dictionary *settings)
	{
		throw NotImplementedException("Load() not implemented");
	}

	Asset *AssetLoader::Load(const String *name, Dictionary *settings)
	{
		throw NotImplementedException("Load() not implemented");
	}

	Expected<Asset *> AssetLoader::__Load(Object *fileOrName, Dictionary *settings) RN_NOEXCEPT
	{
		try
		{
			if(fileOrName->IsKindOfClass(File::GetMetaClass()))
			{
				File *file = static_cast<File *>(fileOrName);
				return Load(file, settings);
			}
			else
			{
				String *name = static_cast<String *>(fileOrName);
				return Load(name, settings);
			}
		}
		catch(...)
		{
			return std::current_exception();
		}
	}

	std::future<Asset *> AssetLoader::LoadInBackground(Object *fileOrName, Dictionary *settings, Tag tag, Callback &&callback)
	{
		WorkQueue *queue = WorkQueue::GetGlobalQueue(WorkQueue::Priority::Background);

		fileOrName->Retain();
		settings->Retain();

		return queue->PerformWithFuture([=]() -> Asset * {

			Asset *result;

			try
			{
				AutoreleasePool::PerformBlock([&] {

					if(fileOrName->IsKindOfClass(File::GetMetaClass()))
					{
						File *file = static_cast<File *>(fileOrName);
						result = Load(file, settings);
					}
					else
					{
						String *name = static_cast<String *>(fileOrName);
						result = Load(name, settings);
					}
				});
			}
			catch(Exception &e)
			{
				callback(nullptr, tag);
				fileOrName->Release();
				settings->Release();

				std::rethrow_exception(std::current_exception());
			}

			callback(result, tag);

			fileOrName->Release();
			settings->Release();

			return result;

		});
	}

	bool AssetLoader::SupportsLoadingFile(File *file) const
	{
		return true;
	}

	bool AssetLoader::SupportsLoadingName(const String *name) const
	{
		return _supportsVirtualFiles;
	}
}
