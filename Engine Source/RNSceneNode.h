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
#include "RNSTL.h"

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
			ChangedGeneric = (1 << 0),
			ChangedFlags = (1 << 1),
			ChangedPosition = (1 << 2),
			ChangedDependencies = (1 << 3),
			ChangedPriority = (1 << 4),
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
		
		RNAPI Vector3 GetPosition() const;
		RNAPI Vector3 GetScale() const;
		RNAPI Vector3 GetEulerAngle() const;
		RNAPI Quaternion GetRotation() const;
		
		RNAPI Vector3 Forward() const;
		RNAPI Vector3 Up() const;
		RNAPI Vector3 Right() const;
		
		RNAPI Vector3 GetWorldPosition() const;
		RNAPI Vector3 GetWorldScale() const;
		RNAPI Vector3 GetWorldEulerAngle() const;
		RNAPI Quaternion GetWorldRotation() const;
		
		RNAPI AABB GetBoundingBox() const;
		RNAPI Sphere GetBoundingSphere() const;
		
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
		
		RNAPI Matrix GetWorldTransform() const;
		RNAPI Matrix GetLocalTransform() const;
		
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
		RNAPI void CleanUp() override;
		
		RNAPI virtual void ChildDidUpdate(SceneNode *child, uint32 changes) {}
		RNAPI virtual void WillAddChild(SceneNode *child) {}
		RNAPI virtual void DidAddChild(SceneNode *child)  {}
		RNAPI virtual void WillRemoveChild(SceneNode *child) {}
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
		void UpdateInternalData() const;
		void __BreakDependency(SceneNode *dependency);
		
		World *_world;
		
		mutable SpinLock _parentChildLock;
		SceneNode *_parent;
		Array _children;
		
		Priority _priority;
		std::atomic<Flags> _flags;
		std::atomic<FrameID> _lastFrame;
		
		std::function<void (SceneNode *, float)> _action;
		std::string _debugName;
		
		SpinLock _dependenciesLock;
		std::unordered_map<SceneNode *, Connection *> _dependencyMap;
		std::vector<SceneNode *> _dependencies;
		
		mutable SpinLock _transformLock;
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
		_transformLock.Lock();
		_position += trans;
		_transformLock.Unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::Scale(const Vector3& scal)
	{
		_transformLock.Lock();
		_scale += scal;
		_transformLock.Unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::Rotate(const Vector3& rot)
	{
		_transformLock.Lock();
		_euler += rot;
		_rotation = Quaternion(_euler);
		_transformLock.Unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	
	RN_INLINE void SceneNode::TranslateLocal(const Vector3& trans)
	{
		_transformLock.Lock();
		_position += _rotation->RotateVector(trans);
		_transformLock.Unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::ScaleLocal(const Vector3& scal)
	{
		_transformLock.Lock();
		_scale += _rotation->RotateVector(scal);
		_transformLock.Unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	
	RN_INLINE void SceneNode::SetPosition(const Vector3& pos)
	{
		_transformLock.Lock();
		_position = pos;
		_transformLock.Unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::SetScale(const Vector3& scal)
	{
		_transformLock.Lock();
		_scale = scal;
		_transformLock.Unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::SetRotation(const Quaternion& rot)
	{
		_transformLock.Lock();
		_euler = rot.GetEulerAngle();
		_rotation = rot;
		_transformLock.Unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	
	RN_INLINE void SceneNode::SetWorldPosition(const Vector3& pos)
	{
		if(!_parent)
		{
			SetPosition(pos);
			return;
		}
		
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		Quaternion temp;
		temp = temp / _parent->GetWorldRotation();
		
		_position = temp.RotateVector(pos) - temp.RotateVector(GetWorldPosition());
		
		lock1.unlock();
		lock2.unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::SetWorldScale(const Vector3& scal)
	{
		if(!_parent)
		{
			SetScale(scal);
			return;
		}
		
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		_scale = scal - GetWorldScale();
		
		lock1.unlock();
		lock2.unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	RN_INLINE void SceneNode::SetWorldRotation(const Quaternion& rot)
	{
		if(!_parent)
		{
			SetRotation(rot);
			return;
		}
		
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		_rotation = rot / _parent->GetWorldRotation();
		_euler = _rotation->GetEulerAngle();
		
		lock1.unlock();
		lock2.unlock();
		
		DidUpdate(ChangedPosition);
	}
	
	
	RN_INLINE Vector3 SceneNode::Forward() const
	{
		Vector3 forward = GetWorldRotation().RotateVector(Vector3(0.0, 0.0, -1.0));
		return forward;
	}
	
	RN_INLINE Vector3 SceneNode::Up() const
	{
		Vector3 up = GetWorldRotation().RotateVector(Vector3(0.0, 1.0, 0.0));
		return up;
	}
	
	RN_INLINE Vector3 SceneNode::Right() const
	{
		Vector3 right = GetWorldRotation().RotateVector(Vector3(1.0, 0.0, 0.0));
		return right;
	}
	
	
	RN_INLINE Vector3 SceneNode::GetPosition() const
	{
		_transformLock.Lock();
		Vector3 result(_position);
		_transformLock.Unlock();
		
		return result;
	}
	RN_INLINE Vector3 SceneNode::GetScale() const
	{
		_transformLock.Lock();
		Vector3 result(_scale);
		_transformLock.Unlock();
		
		return result;
	}
	RN_INLINE Quaternion SceneNode::GetRotation() const
	{
		_transformLock.Lock();
		Quaternion result(_rotation);
		_transformLock.Unlock();
		
		return result;
	}
	RN_INLINE Vector3 SceneNode::GetEulerAngle() const
	{
		_transformLock.Lock();
		Vector3 result(_euler);
		_transformLock.Unlock();
		
		return result;
	}
	
	RN_INLINE Vector3 SceneNode::GetWorldPosition() const
	{
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		UpdateInternalData();
		
		Vector3 result(_worldPosition);
		
		lock1.unlock();
		lock2.unlock();
		
		return result;
	}
	RN_INLINE Vector3 SceneNode::GetWorldScale() const
	{
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		UpdateInternalData();
		
		Vector3 result(_worldScale);
		
		lock1.unlock();
		lock2.unlock();
		
		return result;
	}
	RN_INLINE Vector3 SceneNode::GetWorldEulerAngle() const
	{
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		UpdateInternalData();
		
		Vector3 result(_worldEuler);
		
		lock1.unlock();
		lock2.unlock();
		
		return result;
	}
	RN_INLINE Quaternion SceneNode::GetWorldRotation() const
	{
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		UpdateInternalData();
		
		Quaternion result(_worldRotation);
		
		lock1.unlock();
		lock2.unlock();
		
		return result;
	}
	
	RN_INLINE Matrix SceneNode::GetLocalTransform() const
	{
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		UpdateInternalData();
		
		Matrix result(_localTransform);
		
		lock1.unlock();
		lock2.unlock();
		
		return result;
	}
	
	RN_INLINE Matrix SceneNode::GetWorldTransform() const
	{
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		UpdateInternalData();
		
		Matrix result(_worldTransform);
		
		lock1.unlock();
		lock2.unlock();
		
		return result;
	}
	
	RN_INLINE AABB SceneNode::GetBoundingBox() const
	{
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		UpdateInternalData();
		
		AABB result(_transformedBoundingBox);
		
		lock1.unlock();
		lock2.unlock();
		
		return result;
	}
	
	RN_INLINE Sphere SceneNode::GetBoundingSphere() const
	{
		stl::lockable_shim<SpinLock> lock1(_parentChildLock);
		stl::lockable_shim<SpinLock> lock2(_transformLock);
		
		std::lock(lock1, lock2);
		
		UpdateInternalData();
		
		Sphere result(_transformedBoundingSphere);
		
		lock1.unlock();
		lock2.unlock();
		
		return result;
	}
	
	
	
	RN_INLINE void SceneNode::UpdateInternalData() const
	{
		if(_updated)
		{
			_localTransform.MakeTranslate(_position);
			_localTransform.Rotate(_rotation);
			_localTransform.Scale(_scale);
			
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
		}
	}
}

#endif /* __RAYNE_SCENENODE_H__ */
