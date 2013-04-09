//
//  RBCollisionObject.cpp
//  rayne-bullet
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
//  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
//  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "RBCollisionObject.h"

namespace RN
{
	namespace bullet
	{
		RNDeclareMeta(CollisionObject)
		
		CollisionObject::CollisionObject()
		{
			_material = 0;
			_object = 0;
		}
		
		CollisionObject::~CollisionObject()
		{
			if(_material)
			{
				_material->RemoveListener(this);
				_material->Release();
			}
		}
		
		void CollisionObject::SetMaterial(PhysicsMaterial *material)
		{
			if(_material)
			{
				_material->RemoveListener(this);
				_material->Release();
			}
			
			_material = material ? material->Retain() : 0;
			
			if(_material)
			{
				_material->AddListener(this, [this](PhysicsMaterial *material) {
					RN_ASSERT0(material == _material);
					ApplyPhysicsMaterial(_material);
				});
				
				ApplyPhysicsMaterial(_material);
			}
		}
		
		void CollisionObject::ApplyPhysicsMaterial(PhysicsMaterial *material)
		{
			bulletCollisionObject();
			
			_object->setFriction(_material->Friction());
			_object->setRestitution(_material->Restitution());
		}
	}
}