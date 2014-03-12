//
//  TGSponzaWorld.cpp
//  Test Game
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "TGSponzaWorld.h"

namespace TG
{
	SponzaWorld::SponzaWorld()
	{
		RN::MessageCenter::GetSharedInstance()->AddObserver(kRNInputEventMessage, [&](RN::Message *message) {
			
			RN::Event *event = static_cast<RN::Event *>(message);
			HandleInputEvent(event);
			
		}, this);
	}
	
	void SponzaWorld::LoadOnThread(RN::Thread *thread)
	{
		CreateCameras();
		
		_camera->SetPosition(RN::Vector3(15.0, 0.0, 0.0));
		_camera->SetClipFar(100.0f);
		
		// Sponza
		RN::Model *model = RN::Model::WithFile("models/sponza/sponza.sgm");
		model->GetMaterialAtIndex(0, 12)->SetDiscard(true);
		model->GetMaterialAtIndex(0, 12)->SetCullMode(RN::Material::CullMode::None);
		
		model->GetMaterialAtIndex(0, 17)->SetDiscard(true);
		model->GetMaterialAtIndex(0, 17)->SetCullMode(RN::Material::CullMode::None);
		
		model->GetMaterialAtIndex(0, 22)->SetDiscard(true);
		model->GetMaterialAtIndex(0, 22)->SetCullMode(RN::Material::CullMode::None);
		
		RN::Entity *sponza = new RN::Entity();
		sponza->SetModel(model);
		sponza->SetScale(RN::Vector3(0.2f));
		sponza->SetPosition(RN::Vector3(0.0f, -5.0f, 0.0f));
		

		// Lights
		RN::Random::MersenneTwister random;
		random.Seed(0x1024);
		
		for(int i = 0; i < 200; i ++)
		{
			RN::Vector3 pos(random.RandomFloatRange(-21.0f, 21.0f), random.RandomFloatRange(-7.0f, 14.0f), random.RandomFloatRange(-21.0f, 21.0f));
			
			RN::Light *light = new RN::Light();
			light->SetPosition(pos);
			light->SetRange(random.RandomFloatRange(2.0f, 5.0f));
			light->SetColor(RN::Color(random.RandomFloat(), random.RandomFloat(), random.RandomFloat()));
			
			float offset = random.RandomFloatRange(0.0f, 10.0f);
			
			light->SetAction([offset](RN::SceneNode *light, float delta) {
				
				RN::Vector3 pos = light->GetWorldPosition();
				float time = RN::Kernel::GetSharedInstance()->GetTime() + offset;
				
				pos.x += 0.05f * cos(time * 2.0f);
				pos.z += 0.05f * sin(time * 2.0f);
				
				light->SetWorldPosition(pos);
				
			});
		}
		
		_sunLight = new Sun();
		//_sunLight->SetRenderGroup(3);
		_sunLight->ActivateShadows(RN::ShadowParameter(_camera));
		
		RN::Texture *billboardtex = RN::Texture::WithFile("textures/billboard.png");
		RN::Texture::Parameter param = billboardtex->GetParameter();
		param.wrapMode = RN::Texture::WrapMode::Clamp;
		billboardtex->SetParameter(param);
		RN::Decal *billboard = new RN::Decal(billboardtex);
		
		printf("(%zu, %zu), %f\n", billboardtex->GetWidth(), billboardtex->GetHeight(), billboardtex->GetScaleFactor());
		
		//billboard->SetScale(RN::Vector3(0.04f));
		billboard->SetRenderGroup(1);
		billboard->SetRotation(RN::Quaternion(RN::Vector3(90.0f, 0.0f, 0.0f)));
		billboard->Translate(RN::Vector3(-17.35f, 12.0f, 0.7f));
	}
}
