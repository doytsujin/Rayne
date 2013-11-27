//
//  RNSceneNode.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_SCENENODE_H__
#define __RAYNE_SCENENODE_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNMatrix.h"
#include "RNQuaternion.h"
#include "RNVector.h"
#include "RNSignal.h"
#include "RNArray.h"
#include "RNAABB.h"
#include "RNSphere.h"
#include "RNKVOImplementation.h"

namespace RN
{
	class Renderer;
	class Camera;
	class World;
	
	class RenderingObject;
	class SceneNode : public Object
	{
	public:
		friend class Renderer;
		friend class World;
		
		enum class Priority
		{
			UpdateEarly,
			UpdateDontCare,
			UpdateLate
		};
		
		enum
		{
			FlagDrawLate = (1 << 0),
			FlagStatic   = (1 << 1),
			FlagHidden   = (1 << 2)
		};
		
		enum
		{
			ChangedFlags = (1 << 0),
			ChangedPosition = (1 << 1),
			ChangedDependencies = (1 << 2),
			ChangedPriority = (1 << 3),
			ChangedParent = (1 << 5)
		};
		
		typedef uint32 Flags;
		
		RNAPI SceneNode();
		RNAPI SceneNode(const Vector3& position);
		RNAPI SceneNode(const Vector3& position, const Quaternion& rotation);
		RNAPI ~SceneNode() override;
		
		RNAPI void FillRenderingObject(RenderingObject& object) const;
		
		RNAPI void Translate(const Vector3& trans);
		RNAPI void Scale(const Vector3& scal);
		RNAPI void Rotate(const Vector3& rot);
		
		RNAPI void TranslateLocal(const Vector3& trans);
		RNAPI void ScaleLocal(const Vector3& scal);
		
		RNAPI void SetFlags(Flags flags);
		
		RNAPI virtual void SetPosition(const Vector3& pos);
		RNAPI virtual void SetScale(const Vector3& scal);
		RNAPI virtual void SetRotation(const Quaternion& rot);
		
		RNAPI virtual void SetWorldPosition(const Vector3& pos);
		RNAPI virtual void SetWorldScale(const Vector3& scal);
		RNAPI virtual void SetWorldRotation(const Quaternion& rot);
		
		RNAPI void SetBoundingBox(const AABB& boundingBox, bool calculateBoundingSphere=true);
		RNAPI void SetBoundingSphere(const Sphere& boundingSphere);
		
		RNAPI void SetPriority(Priority priority);
		RNAPI void SetDebugName(const std::string& name);
		
		RNAPI virtual bool IsVisibleInCamera(Camera *camera);
		RNAPI virtual void Render(Renderer *renderer, Camera *camera);
		
		const Vector3& GetPosition() const { return static_cast<const Vector3&>(_position); }
		const Vector3& GetScale() const { return static_cast<const Vector3&>(_scale); }
		const Vector3& GetEulerAngle() const { return _euler; }
		const Quaternion& GetRotation() const { return _rotation; }
		
		RNAPI const Vector3 Forward() const;
		RNAPI const Vector3 Up() const;
		RNAPI const Vector3 Right() const;
		
		RNAPI const Vector3& GetWorldPosition() const;
		RNAPI const Vector3& GetWorldScale() const;
		RNAPI const Vector3& GetWorldEulerAngle() const;
		RNAPI const Quaternion& GetWorldRotation() const;
		
		RNAPI const AABB& GetBoundingBox() const;
		RNAPI const Sphere& GetBoundingSphere() const;
		
		const std::string& GetDebugName() { return _debugName; }
		
		RNAPI void LookAt(SceneNode *other);
		
		RNAPI void AttachChild(SceneNode *child);
		RNAPI void DetachChild(SceneNode *child);
		RNAPI void DetachFromParent();
		RNAPI void LockChildren() const;
		RNAPI void UnlockChildren() const;
		
		RNAPI void SetAction(const std::function<void (SceneNode *, float)>& action);
		RNAPI void AddDependency(SceneNode *dependency);
		RNAPI void RemoveDependency(SceneNode *dependency);
		
		SceneNode *GetParent() const;
		FrameID GetLastFrame() const { return _lastFrame; }
		World *GetWorld() const { return _world; }
		Priority GetPriority() const { return _priority; }
		Flags GetFlags() const { return _flags; }
		
		const Array *GetChildren() const { return &_children; }
		
		RNAPI virtual class Hit CastRay(const Vector3 &position, const Vector3 &direction, Hit::HitMode mode = Hit::HitMode::IgnoreNone);
		
		RNAPI const Matrix& GetWorldTransform() const;
		RNAPI const Matrix& GetLocalTransform() const;
		
		virtual void Update(float delta)
		{
			if(_action)
				_action(this, delta);
		}
		
		virtual bool CanUpdate(FrameID frame);
		
		int8 renderGroup;
		int8 collisionGroup;
		
	protected:
		RNAPI void DidUpdate(uint32 changeSet);
		RNAPI void UpdateInternalData() const;
		RNAPI void CleanUp() override;
		
		RNAPI virtual void ChildDidUpdate(SceneNode *child, uint32 changes) {}
		RNAPI virtual void DidAddChild(SceneNode *child)  {}
		RNAPI virtual void DidRemoveChild(SceneNode *child) {}
		
		void UpdatedToFrame(FrameID frame) { _lastFrame.store(frame); }
		
		Observable<Vector3> _position;
		Observable<Vector3> _scale;
		Observable<Quaternion> _rotation;
		Vector3 _euler;
		
		AABB   _boundingBox;
		Sphere _boundingSphere;
		
		Signal<void (SceneNode *)> _cleanUpSignal;
		
	private:
		void Initialize();
		bool Compare(const SceneNode *other) const;
		void __BreakDependency(SceneNode *dependency);
		
		World *_world;
		
		mutable SpinLock _parentChildLock;
		SceneNode *_parent;
		Array _children;
		
		Flags _flags;
		Priority _priority;
		std::atomic<FrameID> _lastFrame;
		
		std::function<void (SceneNode *, float)> _action;
		std::string _debugName;
		
		SpinLock _dependenciesLock;
		std::unordered_map<SceneNode *, Connection *> _dependencyMap;
		std::vector<SceneNode *> _dependencies;
		
		mutable bool _updated;
		mutable Vector3 _worldPosition;
		mutable Quaternion _worldRotation;
		mutable Vector3 _worldScale;
		mutable Vector3 _worldEuler;
		
		mutable AABB _transformedBoundingBox;
		mutable Sphere _transformedBoundingSphere;
		
		mutable Matrix _worldTransform;
		mutable Matrix _localTransform;
		
		RNDefineMetaWithTraits(SceneNode, Object, MetaClassTraitCronstructable)
	};
	
	
	
	RN_INLINE void SceneNode::Translate(const Vector3& trans)
	{
		_position += trans;
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::Scale(const Vector3& scal)
	{
		_scale += scal;
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::Rotate(const Vector3& rot)
	{
		_euler += rot;
		_rotation = Quaternion(_euler);
		DidUpdate(ChangedPosition);
	}
	
	
	RN_INLINE void SceneNode::TranslateLocal(const Vector3& trans)
	{
		_position += _rotation->RotateVector(trans);
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::ScaleLocal(const Vector3& scal)
	{
		_scale += _rotation->RotateVector(scal);
		DidUpdate(ChangedPosition);
	}
	
	
	RN_INLINE void SceneNode::SetPosition(const Vector3& pos)
	{
		_position = pos;
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::SetScale(const Vector3& scal)
	{
		_scale = scal;
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::SetRotation(const Quaternion& rot)
	{
		_euler = rot.GetEulerAngle();
		_rotation = rot;
		
		DidUpdate(ChangedPosition);
	}
	
	
	RN_INLINE void SceneNode::SetWorldPosition(const Vector3& pos)
	{
		if(!_parent)
		{
			SetPosition(pos);
			return;
		}
		
		Quaternion temp;
		temp = temp / _parent->GetWorldRotation();
		
		_position = temp.RotateVector(pos) - temp.RotateVector(GetWorldPosition());
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::SetWorldScale(const Vector3& scal)
	{
		if(!_parent)
		{
			SetScale(scal);
			return;
		}
		
		_scale = scal - GetWorldScale();
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::SetWorldRotation(const Quaternion& rot)
	{
		if(!_parent)
		{
			SetRotation(rot);
			return;
		}
		
		_rotation = rot / _parent->GetWorldRotation();
		_euler = _rotation->GetEulerAngle();
		
		DidUpdate(ChangedPosition);
	}
	
	
	RN_INLINE const Vector3 SceneNode::Forward() const
	{
		Vector3 forward = GetWorldRotation().RotateVector(Vector3(0.0, 0.0, -1.0));
		return forward;
	}
	
	RN_INLINE const Vector3 SceneNode::Up() const
	{
		Vector3 up = GetWorldRotation().RotateVector(Vector3(0.0, 1.0, 0.0));
		return up;
	}
	
	RN_INLINE const Vector3 SceneNode::Right() const
	{
		Vector3 right = GetWorldRotation().RotateVector(Vector3(1.0, 0.0, 0.0));
		return right;
	}
	
	
	RN_INLINE const Vector3& SceneNode::GetWorldPosition() const
	{
		UpdateInternalData();
		return _worldPosition;
	}
	RN_INLINE const Vector3& SceneNode::GetWorldScale() const
	{
		UpdateInternalData();
		return _worldScale;
	}
	RN_INLINE const Vector3& SceneNode::GetWorldEulerAngle() const
	{
		UpdateInternalData();
		return _worldEuler;
	}
	RN_INLINE const Quaternion& SceneNode::GetWorldRotation() const
	{
		UpdateInternalData();
		return _worldRotation;
	}
	

	
	RN_INLINE const Matrix& SceneNode::GetLocalTransform() const
	{
		UpdateInternalData();
		return _localTransform;
	}
	
	RN_INLINE const Matrix& SceneNode::GetWorldTransform() const
	{
		UpdateInternalData();
		return _worldTransform;
	}
	
	RN_INLINE const AABB& SceneNode::GetBoundingBox() const
	{
		UpdateInternalData();
		return _transformedBoundingBox;
	}
	
	RN_INLINE const Sphere& SceneNode::GetBoundingSphere() const
	{
		UpdateInternalData();
		return _transformedBoundingSphere;
	}
	
	
	RN_INLINE void SceneNode::UpdateInternalData() const
	{
		if(_updated)
		{
			_localTransform.MakeTranslate(_position);
			_localTransform.Rotate(_rotation);
			_localTransform.Scale(_scale);
			
			
			LockChildren();
			
			if(_parent)
			{
				_parent->UpdateInternalData();
				
				_worldPosition = _parent->_worldPosition + _parent->_worldRotation.RotateVector(_position);
				_worldRotation = _parent->_worldRotation * _rotation;
				_worldScale = _parent->_worldScale + _scale;
				_worldEuler = _parent->_worldEuler + _euler;
				
				_worldTransform = _parent->_localTransform * _localTransform;
			}
			else
			{
				_worldPosition = _position;
				_worldRotation = _rotation;
				_worldScale = _scale;
				_worldEuler = _euler;
				
				_worldTransform = _localTransform;
			}
			
			_transformedBoundingBox = _boundingBox;
			
			_transformedBoundingBox.position = _worldPosition;
			_transformedBoundingBox *= _worldScale;
			_transformedBoundingBox.Rotate(_worldRotation);
			
			_transformedBoundingSphere = _boundingSphere;
			_transformedBoundingSphere.position = _worldPosition;
			_transformedBoundingSphere *= _worldScale;
			_transformedBoundingSphere.Rotate(_worldRotation);
			
			_updated = false;
			
			size_t count = _children.GetCount();
			
			for(size_t i = 0; i < count; i ++)
			{
				SceneNode *child = static_cast<SceneNode *>(_children[i]);
				child->_updated = true;
			}
			
			UnlockChildren();
			return;
		}
	}
}

#endif /* __RAYNE_SCENENODE_H__ */
