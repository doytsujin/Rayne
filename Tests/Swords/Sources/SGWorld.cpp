//
//  SGWorld.cpp
//  Sword Game
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "SGWorld.h"

namespace SG
{
	void World::WillBecomeActive()
	{
		RN::Scene::WillBecomeActive();

		//TODO: Renderer crasht bei schnell aus dem level bewegen

		_bulletWorld = new RN::BulletWorld();
		AddAttachment(_bulletWorld);

		_camera = new RN::Camera();

		_oculusCamera = new RN::OculusCamera(false);

		_player = new Player(_oculusCamera, this, _bulletWorld);
		AddNode(_player);

		_player->AddChild(_camera);
		_camera->SetPosition(RN::Vector3(0.0f, 2.0f, 2.0f));
		_camera->SetRotation(RN::Quaternion::WithEulerAngle(RN::Vector3(0.0f, -45.0f, 0.0f)));

		AddNode(_oculusCamera);

		CreateTestLevel();
	}

	void World::CreateTestLevel()
	{
		LoadGround();
	}

	void World::LoadGround()
	{
		RN::Mesh *boxMesh = RN::Mesh::WithTexturedCube(RN::Vector3(100.0f, 0.5f, 100.0f));
		RN::MaterialDescriptor boxMaterialDescriptor;
		boxMaterialDescriptor.vertexShader = RN::Renderer::GetActiveRenderer()->GetDefaultShader(RN::Shader::Type::Vertex, RN::ShaderOptions::WithMesh(boxMesh));
		boxMaterialDescriptor.fragmentShader = RN::Renderer::GetActiveRenderer()->GetDefaultShader(RN::Shader::Type::Fragment, RN::ShaderOptions::WithMesh(boxMesh));
		boxMaterialDescriptor.AddTexture(RN::Texture::WithName(RNCSTR("models/uberpixel/grass.png")));
		RN::Material *boxMaterial = RN::Material::WithDescriptor(boxMaterialDescriptor);
		boxMaterial->SetTextureTileFactor(100);
		RN::Model *boxModel = new RN::Model(boxMesh, boxMaterial);
		_ground = new RN::Entity(boxModel);
		_ground->SetPosition(RN::Vector3(0.0f, -0.5f, 0.0f));
		AddNode(_ground->Autorelease());

		//TODO: Mesh braucht eine optionale kopie im RAM, damit das trianglemeshshape korrekt funktioniert
		RN::BulletRigidBody *groundBody = RN::BulletRigidBody::WithShape(/*RN::BulletTriangleMeshShape::WithModel(groundModel)*/RN::BulletStaticPlaneShape::WithNormal(RN::Vector3(0.0f, 1.0f, 0.0f), 0.0f), 0.0f);
		groundBody->SetPositionOffset(RN::Vector3(0.0f, -0.5f, 0.0f));
		RN::BulletMaterial *groundBulletMaterial = new RN::BulletMaterial();
		groundBody->SetMaterial(groundBulletMaterial->Autorelease());
		_ground->AddAttachment(groundBody);
		_bulletWorld->InsertCollisionObject(groundBody);
	}

	void World::WillUpdate(float delta)
	{
		RN::Scene::WillUpdate(delta);

/*		RN::InputManager *manager = RN::InputManager::GetSharedInstance();

		RN::Vector3 rotation(0.0);

		rotation.x = manager->GetMouseDelta().x;
		rotation.y = manager->GetMouseDelta().y;
		rotation = -rotation;

		RN::Vector3 translation(0.0);

		translation.x = ((int)manager->IsControlToggling(RNCSTR("D")) - (int)manager->IsControlToggling(RNCSTR("A"))) * 10.0f;
		translation.z = ((int)manager->IsControlToggling(RNCSTR("S")) - (int)manager->IsControlToggling(RNCSTR("W"))) * 10.0f;

		_camera->Rotate(rotation * delta * 3.0f);
		_camera->TranslateLocal(translation * delta);

		if(manager->IsControlToggling(RNCSTR("F")))
		{
			if(!_throwKeyPressed)
			{
				ThrowBox();
			}
			_throwKeyPressed = true;
		}
		else
		{
			_throwKeyPressed = false;
		}*/
	}
}
