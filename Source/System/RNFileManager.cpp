//
//  RNFileManager.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "../Base/RNBaseInternal.h"
#include "../Base/RNKernel.h"
#include "../Base/RNApplication.h"
#include "RNFileManager.h"

#if RN_PLATFORM_POSIX
	#include <dirent.h>
	#include <unistd.h>
#endif

namespace RN
{
	RNDefineScopedMeta(FileManager, Node, Object)
	RNDefineScopedMeta(FileManager, Directory, FileManager::Node)
	RNDefineScopedMeta(FileManager, File, FileManager::Node)

	FileManager::Node::Node(String *name, Node *parent, Type type) :
		_type(type),
		_name(name->Retain()),
		_path(nullptr),
		_parent(parent)
	{
		if(_parent)
			SetPath(_parent->GetPath()->StringByAppendingPathComponent(name));
	}
	FileManager::Node::~Node()
	{
		SafeRelease(_path);
		_name->Release();
	}

	void FileManager::Node::SetPath(String *path)
	{
		SafeRelease(_path);
		_path = SafeRetain(path);

		std::cout << _path->GetUTF8String() << std::endl;
	}


	FileManager::Directory::Directory(String *name, Node *parent) :
		Node(name, parent, Type::Directory),
		_children(new Array()),
		_childMap(new Dictionary())
	{
		ParseDirectory();
	}

	FileManager::Directory::Directory(const String *path) :
		Node(path->GetLastPathComponent(), nullptr, Type::Directory),
		_children(new Array()),
		_childMap(new Dictionary())
	{
		SetPath(path->Copy()->Autorelease());
		ParseDirectory();
	}

	FileManager::Directory::~Directory()
	{
		_children->Release();
		_childMap->Release();
	}

	FileManager::Node *FileManager::Directory::GetChildWithName(const String *name) const
	{
		return _childMap->GetObjectForKey<FileManager::Node>(const_cast<String *>(name));
	}

	void FileManager::Directory::ParseDirectory()
	{
		int error = errno;
		DIR *dir = opendir(GetPath()->GetUTF8String());

		if(!dir)
		{
			errno = error;
			throw InconsistencyException("Couldn't open directory");
		}

		struct dirent *ent;
		while((ent = readdir(dir)))
		{
			if(ent->d_name[0] != '\0' && ent->d_name[0] != '.')
			{
				Node *node = nullptr;

				switch(ent->d_type)
				{
					case DT_DIR:
						node = new Directory(RNSTR(ent->d_name), this);
						break;

					case DT_REG:
						node = new File(RNSTR(ent->d_name), this);
						break;

					default:
						break;
				}

				if(node)
				{
					_children->AddObject(node);
					_childMap->SetObjectForKey(node, const_cast<String *>(node->GetName()));

					node->Release();

				}
			}
		}

		closedir(dir);
	}

	FileManager::File::File(String *name, Node *parent) :
		Node(name, parent, Type::File)
	{}

	// ---------------------
	// MARK: -
	// MARK: FileManager
	// ---------------------

#if RN_PLATFORM_POSIX
	char *realpath_expand(const char *path, char *buffer)
	{
#if RN_PLATFORM_LINUX
		char *home;

		if(path[0] == '~' && (home = getenv("HOME")))
		{
			char temp[PATH_MAX];
			return realpath(strcat(strcpy(temp, home), path + 1), buffer);
		}
		else
		{
			return realpath(path, buffer);
		}
#endif
#if RN_PLATFORM_MAC_OS
		if(path[0] == '~')
		{
			NSString *string = [[NSString stringWithCString:path encoding:NSASCIIStringEncoding] stringByExpandingTildeInPath];
			return realpath([string UTF8String], buffer);
		}
		else
		{
			return realpath(path, buffer);
		}
#endif
	}
#endif

	static FileManager *__sharedInstance = nullptr;

	FileManager::FileManager() :
		_nodes(new Array())
	{
		__sharedInstance = this;
	}
	FileManager::~FileManager()
	{
		__sharedInstance = nullptr;
	}
	FileManager *FileManager::GetSharedInstance()
	{
		return __sharedInstance;
	}

	void FileManager::__PrepareWithManifest()
	{
		Array *paths = Kernel::GetSharedInstance()->GetManifestEntryForKey<Array>(kRNManifestSearchPathsKey);
		if(paths)
		{
			String *delimiter = RNCSTR("./");
			String *base = GetPathForLocation(Location::ApplicationDirectory);

			paths->Enumerate<String>([&](String *path, size_t index, bool &stop) {

				Range range = path->GetRangeOfString(delimiter, 0, Range(0, 2));
				if(range.origin == 0)
				{
					path = path->GetSubstring(Range(1, path->GetLength() - 1));
					path->Insert(base, 0);
				}

				AddSearchPath(path);

			});
		}
	}

	String *FileManager::__ExpandPath(const String *tpath)
	{
#if RN_PLATFORM_POSIX
		char buffer[PATH_MAX];

		int error = errno;
		char *cpath = tpath->GetUTF8String();

		char *result = realpath_expand(cpath, buffer);
		String *path = String::WithString(result ? buffer : cpath);

		if(!result)
			errno = error;
#else
		char buffer[MAX_PATH];

		DWORD result = ::GetFullPathNameA(tpath.c_str(), MAX_PATH, buffer, nullptr);
		std::string path((result != 0) ? buffer : tpath.c_str());
#endif

		return path;
	}

	FileManager::Node *FileManager::ResolvePath(const String *path, ResolveHint hint)
	{
		Array *components = path->GetPathComponents();
		size_t componentCount = components->GetCount();

		if(RN_EXPECT_FALSE(componentCount == 0))
			return nullptr;

		std::lock_guard<std::mutex> lock(_lock);

		Node *result = nullptr;

		_nodes->Enumerate<Directory>([&](Directory *directory, size_t index, bool &stop) {

			Directory *node = directory;

			for(size_t i = 0; i < componentCount; i ++)
			{
				String *component = components->GetObjectAtIndex<String>(i);
				Node *subnode = node->GetChildWithName(component);

				bool lastNode = (i == componentCount - 1);

				if(!subnode)
				{
					if(lastNode && (hint & ResolveHint::CreateNode))
						result = node;

					break;
				}

				if(lastNode)
				{
					result = subnode;
					break;
				}

				if(subnode->GetType() == Node::Type::File)
					break;

				node = static_cast<Directory *>(subnode);
			}

			if(result)
				stop = true;

		});

		if(result)
			return result->Retain()->Autorelease();

		return nullptr;
	}

	String *FileManager::ResolveFullPath(const String *path, ResolveHint hint)
	{
		Node *result = ResolvePath(path, hint);
		if(result)
		{
			if(hint & ResolveHint::CreateNode)
			{
				String *last = path->GetLastPathComponent();

				if(result->GetName()->IsEqual(last))
					return result->GetPath()->Copy()->Autorelease();

				return result->GetPath()->StringByAppendingPathComponent(last);
			}

			return result->GetPath()->Copy()->Autorelease();
		}
		
		String *expanded = __ExpandPath(path);
		
		if(access(expanded->GetUTF8String(), F_OK) != -1)
			return expanded;

		if(hint & ResolveHint::CreateNode)
		{
			String *parent = expanded->StringByDeletingLastPathComponent();
			
			if(access(parent->GetUTF8String(), F_OK) != -1)
				return expanded;
		}

		return nullptr;
	}

	String *FileManager::GetPathForLocation(Location location) const
	{
#if RN_PLATFORM_MAC_OS
		static bool isAppBundle = false;
		static std::once_flag flag;

		std::call_once(flag, [&]{

			NSArray *arguments = [[NSProcessInfo processInfo] arguments];
			NSString *directory = [arguments objectAtIndex:0];

			isAppBundle = ([directory rangeOfString:@".app"].location != NSNotFound);

		});
#endif
		switch(location)
		{
			case Location::ApplicationDirectory:
			{
#if RN_PLATFORM_MAC_OS
				if(isAppBundle)
				{
					NSBundle *bundle = [NSBundle mainBundle];
					NSString *path = [bundle bundlePath];

					return RNSTR([path UTF8String]);
				}
				else
				{
					NSArray *arguments = [[NSProcessInfo processInfo] arguments];
					NSString *directory = [arguments objectAtIndex:0];

					String *result = RNSTR([directory UTF8String]);
					result = result->StringByDeletingLastPathComponent();

					return result;
				}
#endif

				break;
			}

			case Location::RootResourcesDirectory:
			{
#if RN_PLATFORM_MAC_OS
				if(isAppBundle)
				{
					NSBundle *bundle = [NSBundle mainBundle];
					NSString *path = [bundle resourcePath];

					return RNSTR([path UTF8String]);
				}
				else
				{
					NSArray *arguments = [[NSProcessInfo processInfo] arguments];
					NSString *directory = [arguments objectAtIndex:0];

					String *result = RNSTR([directory UTF8String]);
					result = result->StringByDeletingLastPathComponent();

					return result;
				}
#endif

				break;
			}

			case Location::SaveDirectory:
			{
#if RN_PLATFORM_MAC_OS
				//NSURL *url = [[[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] lastObject];


#endif
				break;
			}
		}

		return nullptr;
	}

	void FileManager::AddSearchPath(const String *path)
	{
		path = __ExpandPath(path);

		std::lock_guard<std::mutex> lock(_lock);

		bool hasPath = false;

		_nodes->Enumerate<Directory>([&](Directory *directory, size_t index, bool &stop) {
			if(directory->GetPath()->IsEqual(path))
			{
				hasPath = true;
				stop = true;
			}
		});

		if(!hasPath)
		{
			try
			{
				Directory *directory = new Directory(path);
				_nodes->AddObject(directory);
				directory->Release();
			}
			catch(Exception e)
			{
				throw e;
			}
		}
	}

	void FileManager::RemoveSearchPath(const String *path)
	{
		path = __ExpandPath(path);

		std::lock_guard<std::mutex> lock(_lock);

		size_t index = kRNNotFound;

		_nodes->Enumerate<Directory>([&](Directory *directory, size_t tindex, bool &stop) {
			if(directory->GetPath()->IsEqual(path))
			{
				index = tindex;
				stop = true;
			}
		});

		if(index != kRNNotFound)
		{
			_nodes->RemoveObjectAtIndex(index);
		}
	}
}
