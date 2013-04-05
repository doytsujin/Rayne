//
//  TGWorld.cpp
//  Game
//
//  Created by Sidney Just on 27.01.13.
//  Copyright (c) 2013 Sidney Just. All rights reserved.
//

#include "TGWorld.h"

#define TGWorldFeatureLights        1
#define TGWorldFeatureNormalMapping 0
#define TGWorldFeatureInstancing    0
#define TGWorldFeatureAnimations    1
#define TGWorldFeatureLOD           0

#define TGWorldRandom (float)(rand())/RAND_MAX
#define TGWorldSpotLightRange 95.0f

namespace TG
{
	World::World()
	{
		_physicsAttachment = new RN::bullet::PhysicsWorld();
		AddAttachment(_physicsAttachment->Autorelease());
		
		CreateCameras();
		CreateWorld();
		
		RN::Input::SharedInstance()->Activate();
	}
	
	World::~World()
	{
		RN::Input::SharedInstance()->Deactivate();
		_camera->Release();
	}
	
	
	void World::Update(float delta)
	{
		RN::Input *input = RN::Input::SharedInstance();
		RN::Vector3 translation;
		RN::Vector3 rotation;
		
#if RN_PLATFORM_MAC_OS || RN_PLATFORM_WINDOWS || RN_PLATFORM_LINUX
		const RN::Vector3& mouseDelta = input->MouseDelta() * -0.2f;
		
		rotation.x = mouseDelta.x;
		rotation.z = mouseDelta.y;
		
		translation.x = (input->KeyPressed('a') - input->KeyPressed('d')) * 18.0f;
		translation.z = (input->KeyPressed('w') - input->KeyPressed('s')) * 18.0f;
		
#if TGWorldFeatureLights
		static bool fpressed = false;
		
		if(input->KeyPressed('f'))
		{
			if(!fpressed)
			{
				fpressed = true;
				
				//RN::Light *child = _camera->ChildAtIndex<RN::Light>(0);
				//child->SetRange((child->Range() < 1.0f) ? TGWorldSpotLightRange : 0.0f);
				
				RN::bullet::RigidBody *block;
				
				block = new RN::bullet::RigidBody(RN::bullet::BoxShape::WithHalfExtents(RN::Vector3(0.5f)), _blockMaterial, 5.0f);
				block->SetModel(_blockModel);
				block->SetPosition(_camera->WorldPosition());
				block->ApplyImpulse(_camera->WorldRotation().RotateVector(RN::Vector3(0.0, 0.0, -90.0f)));
			}
		}
		else
		{
			fpressed = false;
		}
#endif
#endif
		
#if RN_PLATFORM_IOS
		const std::vector<RN::Touch>& touches = input->Touches();
		
		for(auto i=touches.begin(); i!=touches.end(); i++)
		{
			if(i->phase == RN::Touch::TouchPhaseBegan)
			{
				if(i->location.x > _camera->Frame().width*0.5f)
				{
					_touchRight = i->uniqueID;
				}
				else
				{
					_touchLeft = i->uniqueID;
				}
			}
			
			if(i->phase == RN::Touch::TouchPhaseMoved)
			{
				if(i->uniqueID == _touchRight)
				{
					rotation.x = i->initialLocation.x - i->location.x;
					rotation.z = i->initialLocation.y - i->location.y;
				}
				
				if(i->uniqueID == _touchLeft)
				{
					translation.x = i->initialLocation.x - i->location.x;
					translation.z = i->initialLocation.y - i->location.y;
				}
			}
		}
		
		rotation *= delta;
		translation *= 0.2f;
#endif
		
#if TGWorldFeatureAnimations
		_girlskeleton->Update(delta*24.0f);
		_zombieskeleton->Update(delta*24.0f);
#endif
		
		_camera->Rotate(rotation);
		_camera->TranslateLocal(translation * -delta);
		
		_finalcam->SetRotation(_camera->Rotation());
		_finalcam->SetPosition(_camera->Position());
	}
	
	void World::CreateCameras()
	{
		RN::RenderStorage *storage = new RN::RenderStorage(RN::RenderStorage::BufferFormatDepth | RN::RenderStorage::BufferFormatStencil);
		RN::Texture *depthtex = new RN::Texture(RN::Texture::FormatDepthStencil);
		storage->SetDepthTarget(depthtex);
		
		RN::Shader *depthShader = RN::Shader::WithFile("shader/rn_LightDepth");
		RN::Material *depthMaterial = new RN::Material(depthShader);
		
		_camera = new RN::Camera(RN::Vector2(), storage, RN::Camera::FlagDefaults);
		_camera->SetMaterial(depthMaterial);
		_camera->override = RN::Camera::OverrideAll & ~(RN::Camera::OverrideDiscard | RN::Camera::OverrideDiscardThreshold | RN::Camera::OverrideTextures);
		
		RN::Shader *downsampleShader = RN::Shader::WithFile("shader/rn_LightTileSample");
		RN::Shader *downsampleFirstShader = RN::Shader::WithFile("shader/rn_LightTileSampleFirst");
		RN::Material *downsampleMaterial2x = new RN::Material(downsampleFirstShader);
		downsampleMaterial2x->AddTexture(depthtex);
		
		RN::Camera *downsample2x = new RN::Camera(RN::Vector2(_camera->Frame().width/2, _camera->Frame().height/2), RN::Texture::FormatRG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatComplete);
		_camera->AddStage(downsample2x);
		downsample2x->SetMaterial(downsampleMaterial2x);
		
		RN::Material *downsampleMaterial4x = new RN::Material(downsampleShader);
		downsampleMaterial4x->AddTexture(downsample2x->Storage()->RenderTarget());
		
		RN::Camera *downsample4x = new RN::Camera(RN::Vector2(_camera->Frame().width/4, _camera->Frame().height/4), RN::Texture::FormatRG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
		_camera->AddStage(downsample4x);
		downsample4x->SetMaterial(downsampleMaterial4x);
		
		RN::Material *downsampleMaterial8x = new RN::Material(downsampleShader);
		downsampleMaterial8x->AddTexture(downsample4x->Storage()->RenderTarget());
		
		RN::Camera *downsample8x = new RN::Camera(RN::Vector2(_camera->Frame().width/8, _camera->Frame().height/8), RN::Texture::FormatRG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
		_camera->AddStage(downsample8x);
		downsample8x->SetMaterial(downsampleMaterial8x);
		
		RN::Material *downsampleMaterial16x = new RN::Material(downsampleShader);
		downsampleMaterial16x->AddTexture(downsample8x->Storage()->RenderTarget());
		
		RN::Camera *downsample16x = new RN::Camera(RN::Vector2(_camera->Frame().width/16, _camera->Frame().height/16), RN::Texture::FormatRG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
		_camera->AddStage(downsample16x);
		downsample16x->SetMaterial(downsampleMaterial16x);
		
		RN::Material *downsampleMaterial32x = new RN::Material(downsampleShader);
		downsampleMaterial32x->AddTexture(downsample16x->Storage()->RenderTarget());
		
		RN::Camera *downsample32x = new RN::Camera(RN::Vector2(_camera->Frame().width/32, _camera->Frame().height/32), RN::Texture::FormatRG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
		_camera->AddStage(downsample32x);
		downsample32x->SetMaterial(downsampleMaterial32x);
		
		RN::Camera *downsample64x;
		if(RN::Kernel::SharedInstance()->ScaleFactor() == 2.0f)
		{
			RN::Material *downsampleMaterial64x = new RN::Material(downsampleShader);
			downsampleMaterial64x->AddTexture(downsample32x->Storage()->RenderTarget());
			
			downsample64x = new RN::Camera(RN::Vector2(_camera->Frame().width/64, _camera->Frame().height/64), RN::Texture::FormatRG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
			_camera->AddStage(downsample64x);
			downsample64x->SetMaterial(downsampleMaterial64x);
		}
		
		_finalcam = new RN::Camera(RN::Vector2(), RN::Texture::FormatRGBA32F, RN::Camera::FlagDefaults);
		_finalcam->SetClearMask(RN::Camera::ClearFlagColor);
		_finalcam->Storage()->SetDepthTarget(depthtex);
		_finalcam->SetSkyCube(RN::Model::WithSkyCube("textures/sky_up.png", "textures/sky_down.png", "textures/sky_left.png", "textures/sky_right.png", "textures/sky_front.png", "textures/sky_back.png"));
		
		if(RN::Kernel::SharedInstance()->ScaleFactor() == 2.0f)
		{
			_finalcam->ActivateTiledLightLists(downsample64x->Storage()->RenderTarget());
		}
		else
		{
			_finalcam->ActivateTiledLightLists(downsample32x->Storage()->RenderTarget());
		}
	}
	
	void World::CreateWorld()
	{
		// Sponza
		RN::Model *model = RN::Model::WithFile("models/sponza/sponza.sgm");
		model->MaterialAtIndex(0, 5)->discard = true;
		model->MaterialAtIndex(0, 5)->culling = false;
		model->MaterialAtIndex(0, 5)->alphatest = true;
		
		model->MaterialAtIndex(0, 6)->discard = true;
		model->MaterialAtIndex(0, 6)->culling = false;
		model->MaterialAtIndex(0, 6)->alphatest = true;
		
		model->MaterialAtIndex(0, 17)->discard = true;
		model->MaterialAtIndex(0, 17)->culling = false;
		model->MaterialAtIndex(0, 17)->alphatest = true;
		
#if TGWorldFeatureNormalMapping && TGWorldFeatureLights
		RN::Shader *normalshader = RN::Shader::WithFile("shader/rn_Texture1Normal");
		RN::Texture *normalmap = RN::Texture::WithFile("models/sponza/spnza_bricks_a_ddn.png", RN::Texture::FormatRGBA8888);
		
		model->MaterialAtIndex(0, 3)->AddTexture(normalmap);
		model->MaterialAtIndex(0, 3)->SetShader(normalshader);
#endif
		
		RN::bullet::Shape *sponzaShape = new RN::bullet::TriangelMeshShape(model);
		RN::bullet::PhysicsMaterial *sponzaMaterial = new RN::bullet::PhysicsMaterial();
		
		sponzaShape->SetScale(RN::Vector3(0.5));
		
		RN::bullet::RigidBody *sponza = new RN::bullet::RigidBody(sponzaShape, sponzaMaterial, 0.0f);
		sponza->SetModel(model);
		sponza->SetScale(RN::Vector3(0.5, 0.5, 0.5));
		sponza->SetRotation(RN::Quaternion(RN::Vector3(0.0, 0.0, -90.0)));
		sponza->SetPosition(RN::Vector3(0.0f, -5.0f, 0.0f));
		
		
		RN::Material *blockMaterial = new RN::Material();
		blockMaterial->AddTexture(RN::Texture::WithFile("textures/brick.png", RN::Texture::FormatRGB888));
		
		RN::Mesh *blockMesh = RN::Mesh::CubeMesh(RN::Vector3(0.5f));
		_blockModel = RN::Model::WithMesh(blockMesh, blockMaterial->Autorelease());
		_blockModel->Retain();
		
		_blockMaterial = new RN::bullet::PhysicsMaterial();
		_blockMaterial->SetRestitution(0.3f);
		_blockMaterial->SetFriction(0.6f);
		
		/*RN::bullet::PhysicsMaterial *floorMaterial = new RN::bullet::PhysicsMaterial();
		RN::bullet::Shape *floorShape = RN::bullet::StaticPlaneShape::WithNormal(RN::Vector3(0.0f, 1.0f, 0.0f), 1.0f);
		
		RN::bullet::RigidBody *floor = new RN::bullet::RigidBody(floorShape, floorMaterial, 0.0f);
		floor->SetPosition(RN::Vector3(0.0f, -13.0f, 0.0f));*/
		
#if TGWorldFeatureAnimations
		RN::Model *girlmodel = RN::Model::WithFile("models/TiZeta/simplegirl.sgm");
		_girlskeleton = RN::Skeleton::WithFile("models/TiZeta/simplegirl.sga");
		_girlskeleton->SetAnimation("cammina");
		 
		RN::Entity *girl = new RN::Entity();
		girl->SetModel(girlmodel);
		girl->SetSkeleton(_girlskeleton);
		girl->SetPosition(RN::Vector3(5.0f, -5.0f, 0.0f));
		
		
		RN::Model *zombiemodel = RN::Model::WithFile("models/RosswetMobile/new_thin_zombie.sgm");
		_zombieskeleton = RN::Skeleton::WithFile("models/RosswetMobile/new_thin_zombie.sga");
		_zombieskeleton->SetAnimation("idle");
		 
		RN::Entity *zombie = new RN::Entity();
		zombie->SetModel(zombiemodel);
		zombie->SetSkeleton(_zombieskeleton);
		zombie->SetPosition(RN::Vector3(-5.0f, -5.0f, 0.0f));
#endif
		
#if TGWorldFeatureLOD
		RN::Shader *foliageShader = new RN::Shader();
		foliageShader->SetVertexShader("shader/rn_WindFoliage.vsh");
		foliageShader->SetFragmentShader("shader/rn_Texture1.fsh");
		
		RN::Model *spruceModel = RN::Model::WithFile("models/dexfuck/tree01.sgm");

		for(int i=0; i<2; i++)
		{
			uint32 meshes = spruceModel->Meshes(i);
			for(uint32 j=0; j<meshes; j++)
			{
				RN::Material *material = spruceModel->MaterialAtIndex(i, j);
				
				material->culling = false;
				material->discard = true;
				material->alphatest = true;
				//material->SetShader(foliageShader);
			}
		}
		
		spruceModel->MaterialAtIndex(2, 0)->culling = false;
		spruceModel->MaterialAtIndex(2, 0)->discard = true;
		spruceModel->MaterialAtIndex(2, 0)->alphatest = true;
		
		_spruce = new RN::Entity();
		_spruce->SetModel(spruceModel);
#endif
		
#if TGWorldFeatureLights
		RN::Light *light;
		//srand(time(0));
		
		light = new RN::Light();
		light->SetPosition(RN::Vector3(-30.0f, 0.0f, 0.0f));
		light->SetRange(80.0f);
		light->SetColor(RN::Vector3(TGWorldRandom, TGWorldRandom, TGWorldRandom));
		
		light = new RN::Light();
		light->SetPosition(RN::Vector3(30.0f, 0.0f, 0.0f));
		light->SetRange(80.0f);
		light->SetColor(RN::Vector3(TGWorldRandom, TGWorldRandom, TGWorldRandom));
		
		light = new RN::Light(RN::Light::TypeSpotLight);
		light->SetPosition(RN::Vector3(0.75f, -0.5f, 0.0f));
		light->SetRange(TGWorldSpotLightRange);
		light->SetAngle(0.9f);
		light->SetColor(RN::Vector3(TGWorldRandom, TGWorldRandom, TGWorldRandom));
		_camera->AttachChild(light);
		
		for(int i=0; i<200; i++)
		{
			light = new RN::Light();
			light->SetPosition(RN::Vector3(TGWorldRandom * 140.0f - 70.0f, TGWorldRandom * 100.0f-20.0f, TGWorldRandom * 80.0f - 40.0f));
			light->SetRange((TGWorldRandom * 20.0f) + 10.0f);
			light->SetColor(RN::Vector3(TGWorldRandom, TGWorldRandom, TGWorldRandom));
			
			light->SetAction([](RN::Transform *transform, float delta) {
				transform->Translate(RN::Vector3(0.5f * delta, 0.0f, 0.0));
			});
		}
#endif
		
#if TGWorldFeatureInstancing
		RN::Model *foliage[4];
		
		foliage[0] = RN::Model::WithFile("models/nobiax/fern_01.sgm");
		foliage[0]->MaterialForMesh(foliage[0]->MeshAtIndex(0))->SetShader(foliageShader);
		foliage[0]->MaterialForMesh(foliage[0]->MeshAtIndex(0))->culling = false;
		foliage[0]->MaterialForMesh(foliage[0]->MeshAtIndex(0))->alphatest = true;
		
		foliage[1] = RN::Model::WithFile("models/nobiax/grass_05.sgm");
		foliage[1]->MaterialForMesh(foliage[1]->MeshAtIndex(0))->SetShader(foliageShader);
		foliage[1]->MaterialForMesh(foliage[1]->MeshAtIndex(0))->culling = false;
		foliage[1]->MaterialForMesh(foliage[1]->MeshAtIndex(0))->alphatest = true;
		
		foliage[2] = RN::Model::WithFile("models/nobiax/grass_19.sgm");
		foliage[2]->MaterialForMesh(foliage[2]->MeshAtIndex(0))->SetShader(foliageShader);
		foliage[2]->MaterialForMesh(foliage[2]->MeshAtIndex(0))->culling = false;
		foliage[2]->MaterialForMesh(foliage[2]->MeshAtIndex(0))->alphatest = true;
		
		foliage[3] = RN::Model::WithFile("models/nobiax/grass_04.sgm");
		foliage[3]->MaterialForMesh(foliage[3]->MeshAtIndex(0))->SetShader(foliageShader);
		foliage[3]->MaterialForMesh(foliage[3]->MeshAtIndex(0))->culling = false;
		foliage[3]->MaterialForMesh(foliage[3]->MeshAtIndex(0))->alphatest = true;
		
		uint32 index = 0;
		
		for(float x = -100.0f; x < 200.0f; x += 1.5f)
		{
			for(float y = -10.0f; y < 10.0f; y += 1.0f)
			{
				index = (index + 1) % 4;
				
				RN::Entity *fern = new RN::Entity();
				fern->SetModel(foliage[index]);
				fern->Rotate(RN::Vector3(0.0, 0.0, -90.0));
				fern->SetPosition(RN::Vector3(x, -5.3, y));
			}
		}
#endif
	}
}
