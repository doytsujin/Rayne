//
//  RNContextInternal.h
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_CONTEXTINTERNAL_H__
#define __RAYNE_CONTEXTINTERNAL_H__

#include "RNBase.h"
#include "RNBaseInternal.h"

namespace RN
{
	struct ContextInternals
	{
#if RN_PLATFORM_MAC_OS
		NSOpenGLContext *context;
		NSOpenGLPixelFormat *pixelFormat;
		CGLContextObj cglContext;
#endif
		
#if RN_PLATFORM_IOS
		EAGLContext *context;
#endif
		
#if RN_PLATFORM_WINDOWS
		HWND hWnd;
		HDC  hDC;
		HGLRC context;
		int pixelFormat;
		bool ownsWindow;
#endif
		
#if RN_PLATFORM_LINUX
		XVisualInfo vi;
		GLXContext context;
		XID win;
#endif
	};
	
#if RN_PLATFORM_WINDOWS
	static HWND CreateOffscreenWindow()
	{
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(0);
		
		HWND hWnd = ::CreateWindowExW(0, L"RNWindowClass", L"", WS_POPUP | WS_CLIPCHILDREN, 0, 0, 640, 480, 0, 0, hInstance, 0);
		return hWnd;
	}
#endif
}

#endif /* __RAYNE_CONTEXTINTERNAL_H__ */
