//
//  RNRigidBodyEntity.cpp
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNEntity.h"
#include "RNWorld.h"
#include "RNKernel.h"
#include <btBulletDynamicsCommon.h>

namespace RN
{
	RigidBodyEntity::RigidBodyEntity()
	{
		_mass = 1.0;
		_size.x = 1.0;
		_size.y = 1.0;
		_size.z = 1.0;
		
		_shapeType = Shape_BOX;
		_shape = 0;
		_rigidbody = 0;
		_triangleMesh = 0;
		
		World::SharedInstance()->Physics()->AddRigidBody(this);
	}
	
	RigidBodyEntity::~RigidBodyEntity()
	{
		World::SharedInstance()->Physics()->RemoveRigidBody(this);
	}
	
	void RigidBodyEntity::Update(float delta)
	{
		
	}
	
	void RigidBodyEntity::PostUpdate()
	{
		SetRotation(_tempRotation);
		SetPosition(_tempPosition);
	}
	
	void RigidBodyEntity::InitializeRigidBody(btDynamicsWorld *world)
	{
		switch(_shapeType)
		{
			case Shape_BOX:
				_shape = new btBoxShape(btVector3(_size.x, _size.y, _size.z));
				break;
			
			case Shape_SPHERE:
				_shape = new btSphereShape(_size.x);
				break;
				
			case Shape_MESH:
				_shape = GenerateMeshShape();
				break;
		}
		btVector3 inertia(0, 0, 0);
		if(_triangleMesh == 0)
			_shape->calculateLocalInertia(_mass, inertia);
		btRigidBody::btRigidBodyConstructionInfo bodyci(_mass, this, _shape, inertia);
		_rigidbody = new btRigidBody(bodyci);
		world->addRigidBody(_rigidbody);
		btTransform trans;
		getWorldTransform(trans);
		setWorldTransform(trans);
	}
	
	void RigidBodyEntity::DestroyRigidBody(btDynamicsWorld *world)
	{
		world->removeRigidBody(_rigidbody);
		delete _rigidbody;
		delete _shape;
		if(_triangleMesh != 0)
			delete _triangleMesh;
	}
	
	void RigidBodyEntity::getWorldTransform(btTransform &worldTrans) const
	{
		const Quaternion& rot = Rotation();
		worldTrans.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
		const Vector3& pos = Position();
		worldTrans.setOrigin(btVector3(pos.x, pos.y, pos.z));
	}
	
	void RigidBodyEntity::setWorldTransform(const btTransform &worldTrans)
	{
		btQuaternion rot = worldTrans.getRotation();
		_tempRotation.x = rot.x();
		_tempRotation.y = rot.y();
		_tempRotation.z = rot.z();
		_tempRotation.w = rot.w();
		btVector3 pos = worldTrans.getOrigin();
		_tempPosition = Vector3(pos.x(), pos.y(), pos.z());
	}
	
	btCollisionShape *RigidBodyEntity::GenerateMeshShape()
	{
		_triangleMesh = new btTriangleMesh();
//		_triangleMesh->addTriangle(btVector3(vert1.x, vert1.y, vert1.z), btVector3(vert2.x, vert2.y, vert2.z), btVector3(vert3.x, vert3.y, vert3.z));
		
		return new btBvhTriangleMeshShape(_triangleMesh, true);
	}
}