//
//  TGWorld.h
//  Game
//
//  Created by Sidney Just on 27.01.13.
//  Copyright (c) 2013 Sidney Just. All rights reserved.
//

#ifndef __Game__TGWorld__
#define __Game__TGWorld__

#include <Rayne.h>

namespace TG
{
	class World : RN::World
	{
	public:
		World();
		~World();
		
		virtual void Update(float delta);
		
	private:
		void CreateWorld();
		
		RN::Camera *_camera;
		
		RN::RigidBodyEntity *_block1;
		RN::RigidBodyEntity *_block2;
		RN::RigidBodyEntity *_block3;
		
		RN::RigidBodyEntity *_floor;
	};
}

#endif /* defined(__Game__TGWorld__) */