//
//  RNSceneNodeAttachment.h
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_SCENENODEATTACHMENT_H__
#define __RAYNE_SCENENODEATTACHMENT_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNVector.h"
#include "RNQuaternion.h"

namespace RN
{
	class World;
	class SceneNode;
	
	class SceneNodeAttachment : public Object
	{
	public:
		friend class SceneNode;
		
		RNAPI SceneNodeAttachment();
		RNAPI ~SceneNodeAttachment() override;
		
		RNAPI void SetPosition(const Vector3 &position);
		RNAPI void SetScale(const Vector3 &scale);
		RNAPI void SetRotation(const Quaternion &rotation);
		
		RNAPI Vector3 GetPosition() const;
		RNAPI Vector3 GetScale() const;
		RNAPI Quaternion GetRotation() const;
		
		RNAPI World *GetWorld() const;
		RNAPI SceneNode *GetParent() const;
		
		RNAPI virtual void Update(float delta);
		
	protected:
		RNAPI virtual void WillUpdate(uint32 changeSet) {}
		RNAPI virtual void DidUpdate(uint32 changeSet) {}
		
		RNAPI virtual void WillRemoveFromParent() {}
		RNAPI virtual void DidAddToParent() {}
		
	private:
		void __WillUpdate(uint32 changeSet);
		void __DidUpdate(uint32 changeSet);
		
		SceneNode *_node;
		uint32 _consumeChangeSets;
		
		RNDefineMeta(SceneNodeAttachment, Object);
	};
}

#endif /* __RAYNE_SCENENODEATTACHMENT_H__ */