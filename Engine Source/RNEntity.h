//
//  RNEntity.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_ENTITY_H__
#define __RAYNE_ENTITY_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNSceneNode.h"
#include "RNCamera.h"
#include "RNMaterial.h"
#include "RNModel.h"
#include "RNSkeleton.h"
#include "RNMesh.h"

namespace RN
{
	class World;
	class Entity : public SceneNode
	{
	friend class World;
	public:
		RNAPI Entity();
		RNAPI virtual ~Entity();
		
		RNAPI void SetModel(Model *model);
		RNAPI void SetSkeleton(class Skeleton *skeleton);
		
		Model *Model() const { return _model; }
		Skeleton *Skeleton() const { return _skeleton; }

	private:
		class Model *_model;
		class Skeleton *_skeleton;
		
		RNDefineMeta(Entity, SceneNode)
	};
}

#endif /* __RAYNE_ENTITY_H__ */
