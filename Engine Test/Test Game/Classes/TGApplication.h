//
//  TGApplication.h
//  Game
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __Game__TGApplication__
#define __Game__TGApplication__

#include <Rayne.h>
#include "TGWorld.h"

namespace TG
{
	class Application : public RN::Application
	{
	public:
		Application();
		~Application();
		
		void Start() override;
		void WillExit() override;
		
		void GameUpdate(float delta) override;
		void WorldUpdate(float delta) override;
	};
}

#endif /* defined(__Game__TGApplication__) */
