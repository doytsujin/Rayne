//
//  RNSceneManager.h
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_SCENEMANAGER_H__
#define __RAYNE_SCENEMANAGER_H__

#include "RNBase.h"
#include "RNSceneNode.h"
#include "RNCamera.h"
#include "RNEntity.h"
#include "RNRenderer.h"
#include "RNHit.h"

namespace RN
{
	class SceneManager : public Object
	{
	public:
		RNAPI virtual void AddSceneNode(SceneNode *node) = 0;
		RNAPI virtual void RemoveSceneNode(SceneNode *node) = 0;
		RNAPI virtual void UpdateSceneNode(SceneNode *node, SceneNode::ChangeSet changes) = 0;
		
		RNAPI virtual void RenderScene(Camera *camera) = 0;
		RNAPI virtual Hit CastRay(const Vector3 &position, const Vector3 &direction, uint32 mask = 0x00ff, Hit::HitMode mode = Hit::HitMode::IgnoreNone) = 0;
		
		RNAPI virtual std::vector<SceneNode *> GetSceneNodes(const AABB &box) = 0;
		
	protected:
		RNAPI SceneManager();
		RNAPI ~SceneManager() override;
		
		Renderer *_renderer;
		
		RNDeclareMeta(SceneManager)
	};
	
	class GenericSceneManager : public SceneManager
	{
	public:
		RNAPI GenericSceneManager();
		RNAPI ~GenericSceneManager() override;
		
		RNAPI void AddSceneNode(SceneNode *node) override;
		RNAPI void RemoveSceneNode(SceneNode *node) override;
		RNAPI void UpdateSceneNode(SceneNode *node, SceneNode::ChangeSet changes) override;
		
		RNAPI void RenderScene(Camera *camera) override;
		
		RNAPI Hit CastRay(const Vector3 &position, const Vector3 &direction, uint32 mask = 0xffff, Hit::HitMode mode = Hit::HitMode::IgnoreNone) override;
		
		RNAPI std::vector<SceneNode *> GetSceneNodes(const AABB &box) override;
		
	private:
		RNAPI void RenderSceneNode(Camera *camera, SceneNode *node);
		
		std::vector<SceneNode *> _nodes;
		
		RNDeclareMeta(GenericSceneManager);
	};
}

#endif /* __RAYNE_SCENEMANAGER_H__ */
