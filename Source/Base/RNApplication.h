//
//  RNApplication.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_APPLICATION_H__
#define __RAYNE_APPLICATION_H__

#include "RNBase.h"
#include "../Objects/RNString.h"

namespace RN
{
	class Kernel;
	class Application
	{
	public:
		friend class Kernel;

		RNAPI Application();
		RNAPI virtual ~Application();

		RNAPI virtual void WillFinishLaunching(Kernel *kernel);
		RNAPI virtual void DidFinishLaunching(Kernel *kernel);

		RNAPI virtual void WillExit();

		RNAPI virtual void WillStep(float delta);
		RNAPI virtual void DidStep(float delta);

		RNAPI virtual void WillBecomeActive();
		RNAPI virtual void DidBecomeActive();
		RNAPI virtual void WillResignActive();
		RNAPI virtual void DidResignActive();

		const String *GetTitle() const { return _title; }

	private:
		void __PrepareForWillFinishLaunching(Kernel *kernel);

		String *_title;
	};
}

#endif /* __RAYNE_APPLICATION_H__ */
