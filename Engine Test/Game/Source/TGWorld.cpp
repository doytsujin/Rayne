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
#define TGWorldFeatureFreeCamera    1
#define TGWorldFeatureZPrePass		0

#define TGWorldFeatureParticles     0
#define TGForestFeatureTrees 500
#define TGForestFeatureGras  10000

#define TGWorldRandom (float)(rand())/RAND_MAX
#define TGWorldSpotLightRange 95.0f

namespace TG
{
	World::World() :
		RN::World("GenericSceneManager")
	{
		_sunLight = 0;
		_finalcam = 0;
		_camera = 0;
		_spotLight = 0;
		_player = 0;
		
		_physicsAttachment = new RN::bullet::PhysicsWorld();
		AddAttachment(_physicsAttachment->Autorelease());
		
		CreateCameras();
//		CreateWorld();
		CreateForest();
		
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

		static bool fpressed = false;
		if(input->KeyPressed('f'))
		{
			if(!fpressed)
			{
				if(_spotLight)
					_spotLight->SetRange(_spotLight->Range() > 1.0f ? 0.0f : TGWorldSpotLightRange);
				fpressed = true;
			}
		}
		else
		{
			fpressed = false;
		}
		
#if TGWorldFeatureFreeCamera
		RN::Vector3 translation;
		RN::Vector3 rotation;
		
		const RN::Vector3& mouseDelta = input->MouseDelta() * -0.2f;
		
		rotation.x = mouseDelta.x;
		rotation.z = mouseDelta.y;
		
		translation.x = (input->KeyPressed('d') - input->KeyPressed('a')) * 16.0f;
		translation.z = (input->KeyPressed('s') - input->KeyPressed('w')) * 16.0f;
		
		_camera->Rotate(rotation);
		_camera->TranslateLocal(translation * delta);
#endif
		
		if(_sunLight != 0)
		{
			RN::Vector3 sunrot;
			sunrot.x = (input->KeyPressed('e') - input->KeyPressed('q')) * 5.0f;
			sunrot.z = (input->KeyPressed('r') - input->KeyPressed('f')) * 2.0f;
			_sunLight->Rotate(sunrot);
		}
	}
	
	
	void World::CreateCameras()
	{
#if TGWorldFeatureZPrePass
		RN::RenderStorage *storage = new RN::RenderStorage(RN::RenderStorage::BufferFormatDepth|RN::RenderStorage::BufferFormatStencil);
		RN::Texture *depthtex = new RN::Texture(RN::TextureParameter::Format::DepthStencil);
		storage->SetDepthTarget(depthtex);
		
		RN::Shader *depthShader = RN::ResourcePool::SharedInstance()->ResourceWithName<RN::Shader>(kRNResourceKeyLightDepthShader);
		RN::Material *depthMaterial = new RN::Material(depthShader);
		
		_camera = new ThirdPersonCamera(storage);
		_camera->SetMaterial(depthMaterial);
		
		RN::Shader *downsampleShader = RN::ResourcePool::SharedInstance()->ResourceWithName<RN::Shader>(kRNResourceKeyLightTileSampleShader);
		RN::Shader *downsampleFirstShader = RN::ResourcePool::SharedInstance()->ResourceWithName<RN::Shader>(kRNResourceKeyLightTileSampleFirstShader);
		
		RN::Material *downsampleMaterial2x = new RN::Material(downsampleFirstShader);
		downsampleMaterial2x->AddTexture(depthtex);
		
		RN::Camera *downsample2x = new RN::Camera(RN::Vector2(_camera->Frame().width/2, _camera->Frame().height/2), RN::TextureParameter::Format::RG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatComplete);
		_camera->AddStage(downsample2x);
		downsample2x->SetMaterial(downsampleMaterial2x);
		
/*		RN::Material *downsampleMaterial4x = new RN::Material(downsampleShader);
		downsampleMaterial4x->AddTexture(downsample2x->Storage()->RenderTarget());
		
		RN::Camera *downsample4x = new RN::Camera(RN::Vector2(_camera->Frame().width/4, _camera->Frame().height/4), RN::TextureParameter::Format::RG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
		_camera->AddStage(downsample4x);
		downsample4x->SetMaterial(downsampleMaterial4x);
		
		RN::Material *downsampleMaterial8x = new RN::Material(downsampleShader);
		downsampleMaterial8x->AddTexture(downsample4x->Storage()->RenderTarget());
		
		RN::Camera *downsample8x = new RN::Camera(RN::Vector2(_camera->Frame().width/8, _camera->Frame().height/8), RN::TextureParameter::Format::RG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
		_camera->AddStage(downsample8x);
		downsample8x->SetMaterial(downsampleMaterial8x);
		
		RN::Material *downsampleMaterial16x = new RN::Material(downsampleShader);
		downsampleMaterial16x->AddTexture(downsample8x->Storage()->RenderTarget());
		
		RN::Camera *downsample16x = new RN::Camera(RN::Vector2(_camera->Frame().width/16, _camera->Frame().height/16), RN::TextureParameter::Format::RG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
		_camera->AddStage(downsample16x);
		downsample16x->SetMaterial(downsampleMaterial16x);
		
		RN::Material *downsampleMaterial32x = new RN::Material(downsampleShader);
		downsampleMaterial32x->AddTexture(downsample16x->Storage()->RenderTarget());
		
		RN::Camera *downsample32x = new RN::Camera(RN::Vector2(_camera->Frame().width/32, _camera->Frame().height/32), RN::TextureParameter::Format::RG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
		_camera->AddStage(downsample32x);
		downsample32x->SetMaterial(downsampleMaterial32x);
		
		RN::Camera *downsample64x;
		if(RN::Kernel::SharedInstance()->ScaleFactor() == 2.0f)
		{
			RN::Material *downsampleMaterial64x = new RN::Material(downsampleShader);
			downsampleMaterial64x->AddTexture(downsample32x->Storage()->RenderTarget());
			
			downsample64x = new RN::Camera(RN::Vector2(_camera->Frame().width/64, _camera->Frame().height/64), RN::TextureParameter::Format::RG32F, RN::Camera::FlagUpdateStorageFrame|RN::Camera::FlagDrawTarget|RN::Camera::FlagInheritProjection, RN::RenderStorage::BufferFormatColor);
			_camera->AddStage(downsample64x);
			downsample64x->SetMaterial(downsampleMaterial64x);
		}
*/
/*		_finalcam = new RN::Camera(RN::Vector2(), RN::TextureParameter::Format::RGBA32F, RN::Camera::FlagDefaults);
		_finalcam->SetClearMask(RN::Camera::ClearFlagColor);
		_finalcam->Storage()->SetDepthTarget(depthtex);
		_finalcam->SetSkyCube(RN::Model::WithSkyCube("textures/sky_up.png", "textures/sky_down.png", "textures/sky_left.png", "textures/sky_right.png", "textures/sky_front.png", "textures/sky_back.png"));
		_finalcam->renderGroup |= RN::Camera::RenderGroup1;
		
		if(RN::Kernel::SharedInstance()->ScaleFactor() == 2.0f)
		{
			_finalcam->ActivateTiledLightLists(downsample64x->Storage()->RenderTarget());
		}
		else
		{
			_finalcam->ActivateTiledLightLists(downsample32x->Storage()->RenderTarget());
		}
		
		_camera->AttachChild(_finalcam);*/
		
#else
		RN::RenderStorage *storage = new RN::RenderStorage(RN::RenderStorage::BufferFormatComplete);
		storage->AddRenderTarget(RN::TextureParameter::Format::RGBA32F);
		_camera = new ThirdPersonCamera(storage);
#endif
	}
	
	void World::CreateWorld()
	{
		// Sponza
		RN::Model *model = RN::Model::WithFile("models/sponza/sponza.sgm");
		model->MaterialAtIndex(0, 5)->discard = true;
		model->MaterialAtIndex(0, 5)->culling = false;
		model->MaterialAtIndex(0, 5)->override = RN::Material::OverrideGroupDiscard;
		
		model->MaterialAtIndex(0, 6)->discard = true;
		model->MaterialAtIndex(0, 6)->culling = false;
		model->MaterialAtIndex(0, 6)->override = RN::Material::OverrideGroupDiscard;
		
		model->MaterialAtIndex(0, 17)->discard = true;
		model->MaterialAtIndex(0, 17)->culling = false;
		model->MaterialAtIndex(0, 17)->override = RN::Material::OverrideGroupDiscard;
		
#if TGWorldFeatureNormalMapping && TGWorldFeatureLights
		RN::Shader *normalshader = RN::Shader::WithFile("shader/rn_Texture1Normal");
		RN::Texture *normalmap = RN::Texture::WithFile("models/sponza/spnza_bricks_a_ddn.png", RN::Texture::FormatRGBA8888);
		
		model->MaterialAtIndex(0, 3)->AddTexture(normalmap);
		model->MaterialAtIndex(0, 3)->SetShader(normalshader);
#endif
		
		RN::bullet::Shape *sponzaShape = new RN::bullet::TriangelMeshShape(model);
		RN::bullet::PhysicsMaterial *sponzaMaterial = new RN::bullet::PhysicsMaterial();
		
		sponzaShape->SetScale(RN::Vector3(0.2f));
		
		RN::bullet::RigidBody *sponza = new RN::bullet::RigidBody(sponzaShape, 0.0f);
		sponza->SetModel(model);
		sponza->SetScale(RN::Vector3(0.2f));
		sponza->SetMaterial(sponzaMaterial);
		sponza->SetRotation(RN::Quaternion(RN::Vector3(0.0, 0.0, -90.0)));
		sponza->SetPosition(RN::Vector3(0.0f, -5.0f, 0.0f));
		
#if !TGWorldFeatureFreeCamera
		RN::Model *playerModel = RN::Model::WithFile("models/TiZeta/simplegirl.sgm");
		RN::Skeleton *playerSkeleton = RN::Skeleton::WithFile("models/TiZeta/simplegirl.sga");
		playerSkeleton->SetAnimation("cammina");
		
		_player = new Player(playerModel);
		_player->SetSkeleton(playerSkeleton);
		_player->SetPosition(RN::Vector3(1.0f, -1.0f, 0.0f));
		_player->SetScale(RN::Vector3(0.4f));
		_player->SetCamera(_camera);
		
		_camera->SetTarget(_player);
#endif
				
#if TGWorldFeatureParticles
		RN::Texture *texture = RN::Texture::WithFile("textures/particle.png");
		
		RN::ParticleMaterial *material = new RN::ParticleMaterial();
		material->AddTexture(texture);
		
		material->lifespan = 10.0f;
		material->minVelocity = RN::Vector3(0.0f, 0.5f, 0.0f);
		material->maxVelocity = RN::Vector3(0.0f, 0.15f, 0.0f);
		
		material->discard = true;
		material->blending = true;
		material->SetBlendMode(RN::Material::BlendMode::Interpolative);
		
		DustEmitter *emitter = new DustEmitter();
		emitter->SetMaterial(material);
		emitter->Cook(100.0f, 10);
		emitter->group = 1;
#endif
		
#if TGWorldFeatureLights
		RN::Light *light;
		//srand(time(0));
		
		light = new RN::Light();
		light->SetPosition(RN::Vector3(-15.0f, 0.0f, 0.0f));
		light->SetRange(80.0f);
		light->SetColor(RN::Color(TGWorldRandom, TGWorldRandom, TGWorldRandom));
		
		light = new RN::Light();
		light->SetPosition(RN::Vector3(15.0f, 0.0f, 0.0f));
		light->SetRange(80.0f);
		light->SetColor(RN::Color(TGWorldRandom, TGWorldRandom, TGWorldRandom));
		
		_sunLight = new RN::Light(RN::Light::TypeDirectionalLight);
		_sunLight->SetRotation(RN::Quaternion(RN::Vector3(0.0f, 0.0f, -90.0f)));
		_sunLight->_lightcam = _camera;
//		_sunLight->ActivateSunShadows(true);
		
		_spotLight = new RN::Light(RN::Light::TypeSpotLight);
		_spotLight->SetPosition(RN::Vector3(0.75f, -0.5f, 0.0f));
		_spotLight->SetRange(TGWorldSpotLightRange);
		_spotLight->SetAngle(0.9f);
		_spotLight->SetColor(RN::Color(0.5f));
		
#if TGWorldFeatureFreeCamera
		_camera->AttachChild(_spotLight);
#else
		_player->AttachChild(_spotLight);
#endif
		
		for(int i=0; i<200; i++)
		{
			light = new RN::Light();
			light->SetPosition(RN::Vector3(TGWorldRandom * 70.0f - 35.0f, TGWorldRandom * 50.0f-10.0f, TGWorldRandom * 40.0f - 20.0f));
			light->SetRange((TGWorldRandom * 10.0f) + 5.0f);
			light->SetColor(RN::Color(TGWorldRandom, TGWorldRandom, TGWorldRandom));
			
			/*light->SetAction([](RN::Transform *transform, float delta) {
				transform->Translate(RN::Vector3(0.5f * delta, 0.0f, 0.0));
			});*/
		}
#endif
		
		RN::Billboard *billboard = new RN::Billboard();
		
		billboard->SetTexture(RN::Texture::WithFile("textures/billboard.png"));
		billboard->SetScale(RN::Vector3(0.2f));
		billboard->SetRotation(RN::Quaternion(RN::Vector3(90.0f, 0.0f, 0.0f)));
		billboard->TranslateLocal(RN::Vector3(0.0f, 9.0f, 57.0f));
	}
	
	
	void World::CreateForest()
	{
		// Ground
		RN::Shader *terrainShader = new RN::Shader();
		terrainShader->SetFragmentShader("shader/rn_Terrain.fsh");
		terrainShader->SetVertexShader("shader/rn_Texture1.vsh");
		
		RN::Model *ground = RN::Model::WithFile("models/UberPixel/ground.sgm");
		ground->MaterialAtIndex(0, 0)->SetShader(terrainShader);
		
		RN::bullet::Shape *groundShape = new RN::bullet::TriangelMeshShape(ground);
		RN::bullet::PhysicsMaterial *groundMaterial = new RN::bullet::PhysicsMaterial();
		
		groundShape->SetScale(RN::Vector3(20.0f));
		
		RN::bullet::RigidBody *groundbody = new RN::bullet::RigidBody(groundShape, 0.0f);
		groundbody->SetModel(ground);
		groundbody->SetScale(RN::Vector3(20.0f));
		groundbody->SetMaterial(groundMaterial);
		
		
		RN::Entity *ent;
		
		RN::Model *building = RN::Model::WithFile("models/Sebastian/Old_Buildings.sgm");
		ent = new RN::Entity();
		ent->SetModel(building);
		ent->SetPosition(RN::Vector3(0.0f, 0.6f, 0.0f));
		
#if TGWorldFeatureNormalMapping && TGWorldFeatureLights
		RN::Shader *normalshader = RN::Shader::WithFile("shader/rn_Texture1Normal");
		
		building->MaterialAtIndex(0, 0)->AddTexture(RN::Texture::WithFile("models/Sebastian/brick2-NM.png", RN::Texture::FormatRGBA8888));
		building->MaterialAtIndex(0, 0)->SetShader(normalshader);
		
		building->MaterialAtIndex(0, 1)->AddTexture(RN::Texture::WithFile("models/Sebastian/Concrete_A-NM.png", RN::Texture::FormatRGBA8888));
		building->MaterialAtIndex(0, 1)->SetShader(normalshader);
		
		building->MaterialAtIndex(0, 2)->AddTexture(RN::Texture::WithFile("models/Sebastian/Concrete_B-NM.png", RN::Texture::FormatRGBA8888));
		building->MaterialAtIndex(0, 2)->SetShader(normalshader);
		
		building->MaterialAtIndex(0, 3)->AddTexture(RN::Texture::WithFile("models/Sebastian/Concrete_C-NM.png", RN::Texture::FormatRGBA8888));
		building->MaterialAtIndex(0, 3)->SetShader(normalshader);
#endif
		
		building = RN::Model::WithFile("models/Sebastian/Old_BuildingsDecals.sgm");
		building->MaterialAtIndex(0, 0)->culling = false;
		building->MaterialAtIndex(0, 0)->discard = true;
		building->MaterialAtIndex(0, 0)->override = RN::Material::OverrideGroupDiscard;
		
		building->MaterialAtIndex(0, 1)->culling = false;
		building->MaterialAtIndex(0, 1)->discard = true;
		building->MaterialAtIndex(0, 1)->override = RN::Material::OverrideGroupDiscard;
		
		ent = new RN::Entity();
		ent->SetModel(building);
		ent->SetPosition(RN::Vector3(0.0f, 0.6f, 0.0f));
		
		building = RN::Model::WithFile("models/Sebastian/Old_BuildingsPlants.sgm");
		building->MaterialAtIndex(0, 0)->culling = false;
		building->MaterialAtIndex(0, 0)->discard = true;
		building->MaterialAtIndex(0, 0)->override = RN::Material::OverrideGroupDiscard;
		
		ent = new RN::Entity();
		ent->SetModel(building);
		ent->SetPosition(RN::Vector3(0.0f, 0.6f, 0.0f));
		
		RN::Model *tree = RN::Model::WithFile("models/dexfuck/spruce2.sgm");
		tree->MaterialAtIndex(0, 0)->culling = false;
		tree->MaterialAtIndex(0, 0)->discard = true;
		tree->MaterialAtIndex(0, 0)->override = RN::Material::OverrideGroupDiscard|RN::Material::OverrideCulling;
		
		RN::InstancingNode *node;
		RN::Random::DualPhaseLCG dualPhaseLCG;
		dualPhaseLCG.Seed(0x1024);
		
		node = new RN::InstancingNode(tree);
		
		for(int i = 0; i < TGForestFeatureTrees; i += 1)
		{
			RN::Vector3 pos = RN::Vector3(dualPhaseLCG.RandomFloatRange(-100.0f, 100.0f), 0.0f, dualPhaseLCG.RandomFloatRange(-100.0f, 100.0f));
			if(pos.Length() < 10.0f)
				continue;
			
			ent = new RN::Entity();
			ent->SetModel(tree);
			ent->SetPosition(pos);
			ent->SetScale(RN::Vector3(dualPhaseLCG.RandomFloatRange(0.89f, 1.12f)));
			ent->SetRotation(RN::Vector3(dualPhaseLCG.RandomFloatRange(0.0f, 365.0f), 0.0f, 0.0f));
			
			node->AttachChild(ent);
			
			if(i == 10)
			{
				ent->SetAction([](RN::SceneNode *node, float delta) {
					node->Translate(RN::Vector3(0.0f, 1.0f * delta, 0.0f));
				});
			}
		}
		
		RN::Model *grass = RN::Model::WithFile("models/dexfuck/grass01.sgm");
		grass->MaterialAtIndex(0, 0)->culling = false;
		grass->MaterialAtIndex(0, 0)->discard = true;
		grass->MaterialAtIndex(0, 0)->override = RN::Material::OverrideGroupDiscard|RN::Material::OverrideCulling;
		
		node = new RN::InstancingNode(grass);
		
		for(int i=0; i<TGForestFeatureGras; i++)
		{
			RN::Vector3 pos = RN::Vector3(dualPhaseLCG.RandomFloatRange(-50.0f, 50.0f), 0.2f, dualPhaseLCG.RandomFloatRange(-50.0f, 50.0f));
			if(pos.Length() < 5.0f)
				continue;
			
			ent = new RN::Entity();
			ent->SetModel(grass);
			ent->SetPosition(pos);
			ent->SetScale(RN::Vector3(2.5f));
			ent->SetRotation(RN::Vector3(dualPhaseLCG.RandomFloatRange(0, 365.0f), 0.0f, 0.0f));
			
			node->AttachChild(ent);
		}
		
		
/*		RN::Model *farm = RN::Model::WithFile("models/arteria/Farm/farmbase.sgm");
		RN::Entity *ent = new RN::Entity();
		ent->SetModel(farm);
		ent->SetPosition(RN::Vector3(0.0f, 20.0f, 0.0f));
		ent->SetScale(RN::Vector3(0.1f, 0.1f, 0.1f));*/
		
#if !TGWorldFeatureFreeCamera
		RN::Model *playerModel = RN::Model::WithFile("models/TiZeta/simplegirl.sgm");
		RN::Skeleton *playerSkeleton = RN::Skeleton::WithFile("models/TiZeta/simplegirl.sga");
		playerSkeleton->SetAnimation("cammina");
		
		_player = new Player(playerModel);
		_player->SetSkeleton(playerSkeleton);
		_player->SetPosition(RN::Vector3(5.0f, 10.0f, 0.0f));
		_player->SetScale(RN::Vector3(0.4f));
		_player->SetCamera(_camera);
		
		_camera->SetTarget(_player);
#endif
		
#if TGWorldFeatureLights
		//RN::Light *light;
		//srand(time(0));
		
/*		light = new RN::Light();
		light->SetPosition(RN::Vector3(-30.0f, 0.0f, 0.0f));
		light->SetRange(80.0f);
		light->SetColor(RN::Color(TGWorldRandom, TGWorldRandom, TGWorldRandom));
		
		light = new RN::Light();
		light->SetPosition(RN::Vector3(30.0f, 0.0f, 0.0f));
		light->SetRange(80.0f);
		light->SetColor(RN::Color(TGWorldRandom, TGWorldRandom, TGWorldRandom));*/
		
		_sunLight = new RN::Light(RN::Light::TypeDirectionalLight);
		_sunLight->SetRotation(RN::Quaternion(RN::Vector3(0.0f, 0.0f, -90.0f)));
		_sunLight->_lightcam = _camera;
		_sunLight->ActivateSunShadows(true);
		
/*		_spotLight = new RN::Light(RN::Light::TypeSpotLight);
		_spotLight->SetPosition(RN::Vector3(0.75f, -0.5f, 0.0f));
		_spotLight->SetRange(TGWorldSpotLightRange);
		_spotLight->SetAngle(0.9f);
		_spotLight->SetColor(RN::Color(0.5f, 0.5f, 0.5f));*/
		
/*#if TGWorldFeatureFreeCamera
		_camera->AttachChild(_spotLight);
#else
		_player->AttachChild(_spotLight);
#endif*/
		
/*		for(int i=0; i<200; i++)
		{
			light = new RN::Light();
			light->SetPosition(RN::Vector3(TGWorldRandom * 140.0f - 70.0f, TGWorldRandom * 100.0f-20.0f, TGWorldRandom * 80.0f - 40.0f));
			light->SetRange((TGWorldRandom * 20.0f) + 10.0f);
			light->SetColor(RN::Color(TGWorldRandom, TGWorldRandom, TGWorldRandom));
			
			light->SetAction([](RN::Transform *transform, float delta) {
			 transform->Translate(RN::Vector3(0.5f * delta, 0.0f, 0.0));
			 });
		}*/
#endif
	}

}
