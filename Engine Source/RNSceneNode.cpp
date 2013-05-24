//
//  RNSceneNode.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNSceneNode.h"
#include "RNWorld.h"

namespace RN
{
	RNDeclareMeta(SceneNode)
	
	SceneNode::SceneNode() :
		_scale(Vector3(1.0f))
	{
		Initialize();
	}
	
	SceneNode::SceneNode(const Vector3& position) :
		_position(position),
		_scale(Vector3(1.0f))
	{
		Initialize();
	}
	
	SceneNode::SceneNode(const Vector3& position, const Quaternion& rotation) :
		_position(position),
		_scale(Vector3(1.0f)),
		_rotation(rotation),
		_euler(rotation.EulerAngle())
	{
		Initialize();
	}
	
	SceneNode::~SceneNode()
	{
		DetachAllChilds();
		
		if(_world)
			_world->RemoveSceneNode(this);
	}
	
	void SceneNode::Initialize()
	{
		_parent = 0;
		_world  = 0;
		_lastFrame = 0;
		
		group = 0;
		
		DidUpdate();
		
		if(World::SharedInstance())
			World::SharedInstance()->AddSceneNode(this);
	}
	
	
	bool SceneNode::IsVisibleInCamera(Camera *camera)
	{
		return camera->InFrustum(_boundingSphere*_worldScale);
	}
	
	void SceneNode::Render(Renderer *renderer, Camera *camera)
	{
	}
	
	
	void SceneNode::SetBoundingBox(const AABB& boundingBox, bool calculateBoundingSphere)
	{
		_boundingBox = boundingBox;
		_boundingSphere = Sphere(_boundingBox);
	}
	
	void SceneNode::SetBoundingSphere(const Sphere& boundingSphere)
	{
		_boundingSphere = boundingSphere;
	}
	
	
	void SceneNode::AttachChild(SceneNode *child)
	{
		child->Retain();
		child->DetachFromParent();
		
		_childs.AddObject(child);
		child->_parent = this;
		child->DidUpdate();
		
		DidAddChild(child);
	}
	
	void SceneNode::DetachChild(SceneNode *child)
	{
		if(child->_parent == this)
		{
			WillRemoveChild(child);
			_childs.RemoveObject(child);
			
			child->_parent = 0;
			child->DidUpdate();
			child->Release();
		}
	}
	
	void SceneNode::DetachAllChilds()
	{
		machine_uint count = _childs.Count();
		
		for(machine_uint i=0; i<count; i++)
		{
			SceneNode *child = _childs.ObjectAtIndex(i);
			WillRemoveChild(child);
			
			child->_parent = 0;
			child->DidUpdate();
			child->Release();
		}
		
		_childs.RemoveAllObjects();
	}
	
	void SceneNode::DetachFromParent()
	{
		if(_parent)
			_parent->DetachChild(this);
	}
	
	void SceneNode::DidUpdate()
	{
		_updated = true;
		
		if(_world)
			_world->SceneNodeUpdated(this);
		
		if(_parent)
			_parent->ChildDidUpdate(this);
	}
	
	void SceneNode::SetAction(const std::function<void (SceneNode *, float)>& action)
	{
		_action = action;
	}
}