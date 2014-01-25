//
//  RNLight.cpp
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNLight.h"
#include "RNWorld.h"
#include "RNCamera.h"
#include "RNResourceCoordinator.h"
#include "RNLightManager.h"
#include "RNCameraInternal.h"
#include "RNOpenGLQueue.h"

namespace RN
{
	RNDeclareMeta(Light)
	
	Light::Light(Type lighttype) :
		_lightType(lighttype),
		_color("color", Color(1.0f), std::bind(&Light::GetColor, this), std::bind(&Light::SetColor, this, std::placeholders::_1)),
		_intensity("intensity", 10.0f, std::bind(&Light::GetIntensity, this), std::bind(&Light::SetIntensity, this, std::placeholders::_1)),
		_range("range", 10.0f, std::bind(&Light::GetRange, this), std::bind(&Light::SetRange, this, std::placeholders::_1)),
		_angle("angle", 45.0f, std::bind(&Light::GetAngle, this), std::bind(&Light::SetAngle, this, std::placeholders::_1))
	{
		_shadow = false;
		_shadowcam = nullptr;
		_lightcam  = nullptr;
		
		SetPriority(SceneNode::Priority::UpdateLate);
		
		SetCollisionGroup(25);
		_angleCos = 0.707f;
		
		AddObservable(&_color);
		AddObservable(&_intensity);
		AddObservable(&_range);
		AddObservable(&_angle);
		ReCalculateColor();
	}
	
	Light::~Light()
	{
		if(_shadowcam)
			_shadowcam->Release();
		
		if(_lightcam)
			_lightcam->Release();
	}
	
	bool Light::IsVisibleInCamera(Camera *camera)
	{
		if(_lightType == Type::DirectionalLight)
			return true;
		
		return SceneNode::IsVisibleInCamera(camera);
	}
	
	void Light::Render(Renderer *renderer, Camera *camera)
	{
		LightManager *manager = camera->GetLightManager();
		
		if(manager)
			manager->AddLight(this);
	}
	
	void Light::SetRange(float range)
	{
		_range = range;
		
		SetBoundingSphere(Sphere(Vector3(), range));
		SetBoundingBox(AABB(Vector3(), range), false);
	}
	
	void Light::SetColor(const Color& color)
	{
		_color = color;
		ReCalculateColor();
	}
	
	void Light::SetIntensity(float intensity)
	{
		_intensity = intensity;
		ReCalculateColor();
	}
	
	void Light::SetAngle(float angle)
	{
		_angle = angle;
		_angleCos = cosf(angle*k::DegToRad);
	}
	
	void Light::SetLightCamera(Camera *lightCamera)
	{
		if(_lightcam)
		{
			RemoveDependency(_lightcam);
			_lightcam->Release();
		}
		
		_lightcam = lightCamera ? lightCamera->Retain() : nullptr;
		AddDependency(_lightcam);
	}
	
	void Light::RemoveShadowCameras()
	{
		_shadowcams.Enumerate<Camera>([&](Camera *camera, size_t index, bool &stop) {
			RemoveDependency(camera);
		});
		
		_shadowcams.RemoveAllObjects();
		
		if(_shadowcam)
		{
			RemoveDependency(_shadowcam);
			_shadowcam->Release();
			_shadowcam = nullptr;
		}
	}
	
	
	
	bool Light::ActivateShadows(const ShadowParameter &parameter)
	{
		switch(_lightType)
		{
			case Type::PointLight:
				return ActivatePointShadows(parameter);
				
			case Type::SpotLight:
				return ActivateSpotShadows(parameter);
				
			case Type::DirectionalLight:
				return ActivateDirectionalShadows(parameter);

			default:
				return false;
		}
	}
	
	void Light::DeactivateShadows()
	{
		RemoveShadowCameras();
		_shadow = false;
	}
	
	
	bool Light::ActivateDirectionalShadows(const ShadowParameter &parameter)
	{
		if(_shadow)
			DeactivateShadows();
			
		_shadowSplits  = static_cast<int>(parameter.splits);
		_shadowDistFac = parameter.distanceFactor;
		
		Texture::Parameter textureParameter;
		textureParameter.wrapMode = Texture::WrapMode::Clamp;
		textureParameter.filter = Texture::Filter::Linear;
		textureParameter.format = Texture::Format::Depth24I;
		textureParameter.depthCompare = true;
		textureParameter.maxMipMaps = 0;
		
		Texture2DArray *depthtex = new Texture2DArray(textureParameter);
		depthtex->SetSize(32, 32, parameter.splits);
		depthtex->Autorelease();
		
		Shader   *depthShader = ResourceCoordinator::GetSharedInstance()->GetResourceWithName<Shader>(kRNResourceKeyDirectionalShadowDepthShader, nullptr);
		Material *depthMaterial = new Material(depthShader);
		depthMaterial->polygonOffset = true;
		depthMaterial->polygonOffsetFactor = parameter.biasFactor;
		depthMaterial->polygonOffsetUnits  = parameter.biasUnits;
		
		for(int i = 0; i < _shadowSplits; i++)
		{
			RenderStorage *storage = new RenderStorage(RenderStorage::BufferFormatDepth, 0, 1.0f);
			storage->SetDepthTarget(depthtex, i);
			
			Camera *tempcam = new Camera(Vector2(parameter.resolution), storage, Camera::Flags::UpdateAspect | Camera::Flags::UpdateStorageFrame | Camera::Flags::Orthogonal | Camera::Flags::NoFlush, 1.0f);
			tempcam->SetMaterial(depthMaterial);
			tempcam->SetLODCamera(_lightcam);
			tempcam->SetLightManager(nullptr);
			tempcam->SetPriority(kRNShadowCameraPriority);
			tempcam->SetClipNear(1.0f);

			_shadowcams.AddObject(tempcam);
			AddDependency(tempcam);
			
			tempcam->Release();
			storage->Release();
			
			try
			{
				OpenGLQueue::GetSharedInstance()->SubmitCommand([&] {
					storage->BindAndUpdateBuffer();
				}, true);
			}
			catch(Exception e)
			{
				RemoveShadowCameras();
				return false;
			}
		}
		
		_shadow = true;
		return true;
	}
	
	bool Light::ActivatePointShadows(const ShadowParameter &parameter)
	{
		if(_shadow)
			DeactivateShadows();
		
		Texture::Parameter textureParameter;
		textureParameter.wrapMode = Texture::WrapMode::Repeat;
		textureParameter.filter = Texture::Filter::Nearest;
		textureParameter.format = Texture::Format::Depth24I;
		textureParameter.depthCompare = false;
		textureParameter.maxMipMaps = 0;
		
		Texture *depthtex = (new TextureCubeMap(textureParameter))->Autorelease();
		
		Shader   *depthShader = ResourceCoordinator::GetSharedInstance()->GetResourceWithName<Shader>(kRNResourceKeyPointShadowDepthShader, nullptr);
		Material *depthMaterial = (new Material(depthShader))->Autorelease();
		
		RenderStorage *storage = new RenderStorage(RenderStorage::BufferFormatDepth, 0, 1.0f);
		storage->SetDepthTarget(depthtex, -1);
		
		_shadowcam = new CubemapCamera(Vector2(parameter.resolution), storage, Camera::Flags::UpdateAspect | Camera::Flags::UpdateStorageFrame | Camera::Flags::NoFlush, 1.0f);
		_shadowcam->Retain();
		_shadowcam->Autorelease();
		_shadowcam->SetMaterial(depthMaterial);
		_shadowcam->SetPriority(kRNShadowCameraPriority);
		_shadowcam->SetClipNear(0.01f);
		_shadowcam->SetClipFar(_range);
		_shadowcam->SetFOV(90.0f);
		_shadowcam->SetLightManager(nullptr);
		_shadowcam->UpdateProjection();
		_shadowcam->SetWorldRotation(Vector3(0.0f, 0.0f, 0.0f));
		
		AddDependency(_shadowcam);
		storage->Release();
		
		try
		{
			OpenGLQueue::GetSharedInstance()->SubmitCommand([&] {
				storage->BindAndUpdateBuffer();
			}, true);
		}
		catch(Exception e)
		{
			RemoveShadowCameras();
			return false;
		}
		
		_shadow = true;
		return true;
	}
	
	bool Light::ActivateSpotShadows(const ShadowParameter &parameter)
	{
		if(_shadow)
			DeactivateShadows();
		
		Texture::Parameter textureParameter;
		textureParameter.wrapMode = Texture::WrapMode::Clamp;
		textureParameter.filter = Texture::Filter::Linear;
		textureParameter.format = Texture::Format::Depth24I;
		textureParameter.depthCompare = true;
		textureParameter.maxMipMaps = 0;
		
		Texture *depthtex = new Texture2D(textureParameter);
		depthtex->Autorelease();
		
		Shader   *depthShader = ResourceCoordinator::GetSharedInstance()->GetResourceWithName<Shader>(kRNResourceKeyDirectionalShadowDepthShader, nullptr);
		Material *depthMaterial = (new Material(depthShader))->Autorelease();
		depthMaterial->polygonOffset = true;
		depthMaterial->polygonOffsetFactor = parameter.biasFactor;
		depthMaterial->polygonOffsetUnits  = parameter.biasUnits;
		
		RenderStorage *storage = new RenderStorage(RenderStorage::BufferFormatDepth, 0, 1.0f);
		storage->SetDepthTarget(depthtex, -1);
		
		_shadowcam = new Camera(Vector2(parameter.resolution), storage, Camera::Flags::UpdateAspect | Camera::Flags::UpdateStorageFrame | Camera::Flags::NoFlush, 1.0f);
		_shadowcam->Retain();
		_shadowcam->Autorelease();
		_shadowcam->SetMaterial(depthMaterial);
		_shadowcam->SetPriority(kRNShadowCameraPriority);
		_shadowcam->SetClipNear(0.01f);
		_shadowcam->SetClipFar(_range);
		_shadowcam->SetFOV(_angle * 2.0f);
		_shadowcam->SetLightManager(nullptr);
		_shadowcam->UpdateProjection();
		_shadowcam->SetWorldRotation(Vector3(0.0f, 0.0f, 0.0f));
		
		AddDependency(_shadowcam);
		storage->Release();
		
		try
		{
			OpenGLQueue::GetSharedInstance()->SubmitCommand([&] {
				storage->BindAndUpdateBuffer();
			}, true);
		}
		catch(Exception e)
		{
			RemoveShadowCameras();
			return false;
		}
		
		_shadow = true;
		return true;
	}
	
	
	void Light::Update(float delta)
	{
		SceneNode::Update(delta);
		
		if(_lightType == Type::DirectionalLight)
		{
			if(_shadow && _lightcam)
			{
				float near = _lightcam->GetClipNear();
				float far;
				
				if(_shadowcam)
					_shadowcam->SetWorldRotation(GetWorldRotation());
				
				_shadowmats.clear();
				
				for(int i = 0; i < _shadowSplits; i++)
				{
					float linear = _lightcam->GetClipNear() + (_lightcam->GetClipFar() - _lightcam->GetClipNear())*(i+1.0f) / float(_shadowSplits);
					float log = _lightcam->GetClipNear() * powf(_lightcam->GetClipFar() / _lightcam->GetClipNear(), (i+1.0f) / float(_shadowSplits));
					far = linear*_shadowDistFac+log*(1.0f-_shadowDistFac);
					
					if(_shadowcam)
					{
						_shadowmats.push_back(std::move(_shadowcam->MakeShadowSplit(_lightcam, this, near, far)));
					}
					else
					{
						Camera *tempcam = _shadowcams.GetObjectAtIndex<Camera>(i);
						tempcam->SetWorldRotation(GetWorldRotation());
						
						_shadowmats.push_back(std::move(tempcam->MakeShadowSplit(_lightcam, this, near, far)));
					}
					
					near = far;
				}
				
				if(_shadowcam)
				{
					_shadowcam->MakeShadowSplit(_lightcam, this, _lightcam->GetClipNear(), _lightcam->GetClipFar());
				}
			}
		}
		else if(_lightType == Type::SpotLight)
		{
			if(_shadow && _shadowcam)
			{
				_shadowcam->SetWorldPosition(GetWorldPosition());
				_shadowcam->SetWorldRotation(GetWorldRotation());
				_shadowcam->SetClipFar(_range);
				_shadowcam->SetFOV(_angle * 2.0f);
				_shadowcam->UpdateProjection();
				_shadowcam->PostUpdate();
				
				_shadowmats.clear();
				_shadowmats.emplace_back(_shadowcam->GetProjectionMatrix() * _shadowcam->GetViewMatrix());
			}
		}
		else
		{
			if(_shadow && _shadowcam)
			{
				_shadowcam->SetWorldPosition(GetWorldPosition());
				_shadowcam->SetClipFar(_range);
				_shadowcam->UpdateProjection();
			}
		}
	}

	void Light::ReCalculateColor()
	{
		_resultColor = Vector3(_color->r, _color->g, _color->b);
		_resultColor *= (float)_intensity;
	}
}
