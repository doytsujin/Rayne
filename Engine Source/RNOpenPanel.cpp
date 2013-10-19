//
//  RNOpenPanel.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNOpenPanel.h"
#include "RNBaseInternal.h"

namespace RN
{
	OpenPanel::OpenPanel()
	{
		_allowsFolders = false;
		_allowsFiles = true;
		_allowsMultipleSelection = false;
		_canCreateDirectories = true;
	}
	
	
	void OpenPanel::SetAllowsFolders(bool allowsFolders)
	{
		_allowsFolders = allowsFolders;
	}
	
	void OpenPanel::SetAllowsFiles(bool allowsFiles)
	{
		_allowsFiles = allowsFiles;
	}
	
	void OpenPanel::SetAllowsMultipleSelection(bool multipleSelection)
	{
		_allowsMultipleSelection = multipleSelection;
	}
	
	void OpenPanel::SetCanCreateDirectories(bool canCreateDirectories)
	{
		_canCreateDirectories = canCreateDirectories;
	}
	
	
	void OpenPanel::SetTitle(const std::string& title)
	{
		_title = title;
	}
	
	void OpenPanel::SetMessage(const std::string& message)
	{
		_message = message;
	}
	
	
	void OpenPanel::SetAllowedFileTypes(const std::vector<std::string>& fileTypes)
	{
		_allowedFileTypes = fileTypes;
	}
	
	
	
	void OpenPanel::Show(std::function<void (bool result, const std::vector<std::string>& paths)> callback)
	{
#if RN_PLATFORM_MAC_OS
		NSOpenPanel *panel = [NSOpenPanel openPanel];
		
		[panel setAllowsMultipleSelection:_allowsMultipleSelection];
		[panel setCanChooseDirectories:_allowsFolders];
		[panel setCanChooseFiles:_allowsFiles];
		[panel setCanCreateDirectories:_canCreateDirectories];
		
		[panel setTitle:[NSString stringWithUTF8String:_title.c_str()]];
		[panel setMessage:[NSString stringWithUTF8String:_message.c_str()]];
		
		if(!_allowedFileTypes.empty())
		{
			NSMutableArray *fileTypes = [NSMutableArray array];
			
			for(const std::string& type : _allowedFileTypes)
				[fileTypes addObject:[NSString stringWithUTF8String:type.c_str()]];
			
			[panel setAllowedFileTypes:fileTypes];
		}
		
		[panel beginSheetModalForWindow:[NSApp keyWindow] completionHandler:^(NSInteger tresult) {
			bool result = (tresult == NSFileHandlingPanelOKButton);
			NSArray *urls = [panel URLs];
		 
			std::vector<std::string> files;
			for(NSURL *url in urls)
			{
				files.push_back([[url path] UTF8String]);
			}
		 
			callback(result, files);
		}];
#endif
		
#if RN_PLATFORM_WINDOWS
		TCHAR szFile[MAX_PATH];
		OPENFILENAME ofn;
		
		ZeroMemory(szFile ,MAX_PATH);
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner   = hWnd;
		ofn.lpstrTitle  = _title.c_str();
		ofn.lpstrFile   = szFile;
		ofn.lpstrFilter = "All Files (*.*)\0*.*\0\0";
		ofn.nMaxFile    = MAX_PATH;
		ofn.Flags       = OFN_PATHMUSTEXIST | OFN_EXPLORER;
		
		if(_allowsMultipleSelection)
			ofn.Flags |= OFN_ALLOWMULTISELECT;
		
		if(_allowsFiles)
			ofn.Flags |= OFN_FILEMUSTEXIST;
		
			
		bool result = ::GetOpenFileName(&ofn);
		std::vector<std::string> files;
		
		if(result)
		{
			if(_allowsMultipleSelection)
			{
				char *base = szFile;
				char *temp = base + strlen(base) + 1;
				
				while(*temp)
				{
					std::stringstream stream;
					stream << base << temp;
					
					files.push_back(stream.str());
					
					temp += strlen(temp) + 1;
				}
			}
			else
			{
				files.push_back(szFile);
			}
		}
		
		callback(result, files);
#endif
	}
}
