//
//  RNCamera.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_CAMERA_H__
#define __RAYNE_CAMERA_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNSceneNode.h"
#include "RNRenderStorage.h"
#include "RNTexture.h"
#include "RNMaterial.h"
#include "RNRect.h"
#include "RNPlane.h"
#include "RNColor.h"
#include "RNModel.h"
#include "RNSphere.h"

namespace RN
{
	class Camera : public SceneNode
	{
	public:
		enum
		{
			FlagDrawTarget = (1 << 0),
			FlagUpdateAspect = (1 << 1),
			FlagFullscreen = (1 << 2),
			FlagNoClear = (1 << 3),
			FlagInheritPosition = (1 << 4),
			FlagInheritFrame = (1 << 5),
			FlagInheritProjection = (1 << 6),
			FlagUpdateStorageFrame = (1 << 7),
			
			
			FlagDefaults = (FlagFullscreen | FlagUpdateAspect | FlagUpdateStorageFrame),
			FlagInherit = (FlagInheritFrame | FlagInheritPosition | FlagInheritProjection)
		};
		typedef uint32 Flags;
		
		enum
		{
			ClearFlagColor = (1 << 0),
			ClearFlagDepth = (1 << 1),
			ClearFlagStencil = (1 << 2)
		};
		typedef uint32 ClearFlags;
		
		enum
		{
			ColorFlagRed = (1 << 0),
			ColorFlagGreen = (1 << 1),
			ColorFlagBlue = (1 << 2),
			ColorFlagAlpha = (1 << 3)
		};
		typedef uint32 ColorFlags;
		
		enum
		{
			RenderGroup0 = (1 << 0),
			RenderGroup1 = (1 << 1),
			RenderGroup2 = (1 << 2),
			RenderGroup3 = (1 << 3),
			RenderGroup4 = (1 << 4),
			RenderGroup5 = (1 << 5),
			RenderGroup7 = (1 << 7),
			RenderGroup8 = (1 << 8),
			RenderGroup9 = (1 << 9)
		};
		typedef uint32 RenderGroups;
		
		RNAPI Camera(const Vector2& size);
		
		RNAPI Camera(const Vector2& size, Texture *target);
		RNAPI Camera(const Vector2& size, Texture *target, Flags flags);
		RNAPI Camera(const Vector2& size, Texture *target, Flags flags, RenderStorage::BufferFormat format);
		
		RNAPI Camera(const Vector2& size, Texture::Format targetFormat);
		RNAPI Camera(const Vector2& size, Texture::Format targetFormat, Flags flags);
		RNAPI Camera(const Vector2& size, Texture::Format targetFormat, Flags flags, RenderStorage::BufferFormat format);
		
		RNAPI Camera(const Vector2& size, RenderStorage *storage, Flags flags);
		
		RNAPI virtual ~Camera();
		
		RNAPI void Bind();
		RNAPI void Unbind();
		RNAPI void PrepareForRendering();
		
		RNAPI void SetFrame(const Rect& frame);
		RNAPI void SetClearColor(const Color& color);
		RNAPI void SetMaterial(Material *material);
		RNAPI void SetRenderStorage(RenderStorage *storage);
		RNAPI void SetClearMask(ClearFlags mask);
		RNAPI void SetColorMask(ColorFlags mask);
		RNAPI void SetAllowsDepthWrite(bool flag);
		RNAPI void SetSkyCube(Model *skycube);
		RNAPI void SetMaxLightsPerTile(machine_uint lights);
		
		RNAPI void AddStage(Camera *stage);
		RNAPI void InsertStage(Camera *stage);
		RNAPI void ReplaceStage(Camera *stage);
		RNAPI void RemoveStage(Camera *stage);
		
		RNAPI void ActivateTiledLightLists(Texture *depthTiles);
		Texture *DepthTiles() { return _depthTiles; }
		
		RNAPI void Update(float delta);
		RNAPI void PostUpdate();
		RNAPI void UpdateProjection();
		
		RNAPI Vector3 ToWorld(const Vector3& dir);
		RNAPI void UpdateFrustum();
		RNAPI bool InFrustum(const Vector3& position, float radius);
		RNAPI bool InFrustum(const Sphere& sphere);
		RNAPI bool InFrustum(const AABB& aabb);
		
		RNAPI virtual bool IsVisibleInCamera(Camera *camera);
		
		const Vector3& FrustumCenter() const { return _frustumCenter; }
		float const FrustumRadius() const { return _frustumRadius; }
		
		RenderStorage *Storage() const { return _storage; }
		const Color& ClearColor() const { return _clearColor; }
		const Rect& Frame();
		Material *Material() const { return _material; }
		Camera *Stage() const { return _stage; }
		Flags CameraFlags() const { return _flags; }
		
		Vector2 LightTiles() const { return _lightTiles; }
		Model *SkyCube() const { return _skycube; }
		
		uint32 RenderTargets() const { return (uint32)_storage->_renderTargets->Count(); }
		Texture *RenderTarget(uint32 index=0) const { return _storage->_renderTargets->ObjectAtIndex(index); }
		
		bool HasDepthbuffer() const { return _storage->HasDepthbuffer(); }
		bool HasStencilbuffer() const { return _storage->HasStencilbuffer(); }
		
		bool AllowsDepthWrite() const { return _allowDepthWrite; }
		
		float *DepthArray();
		machine_uint MaxLightsPerTile() const { return _maxLights; }
		
		float fov;
		float aspect;
		float clipnear;
		float clipfar;
		
		Matrix projectionMatrix;
		Matrix inverseProjectionMatrix;
		Matrix viewMatrix;
		Matrix inverseViewMatrix;
		
		RenderGroups renderGroup;
		
	protected:
		void Initialize();
		
	private:
		Rect _frame;
		Color _clearColor;
		Flags _flags;
		ColorFlags _colorMask;
		GLuint _clearMask;
		float _scaleFactor;
		
		Vector3 _frustumCenter;
		float _frustumRadius;
		
		struct
		{
			Plane _frustumLeft;
			Plane _frustumRight;
			Plane _frustumTop;
			Plane _frustumBottom;
			Plane _frustumFar;
			Plane _frustumNear;
			
			Plane _absFrustumLeft;
			Plane _absFrustumRight;
			Plane _absFrustumTop;
			Plane _absFrustumBottom;
			Plane _absFrustumFar;
			Plane _absFrustumNear;
		};
		
		Vector2 _lightTiles;
		
		bool _allowDepthWrite;
		
		class Material *_material;
		RenderStorage *_storage;
		Camera *_stage;
		
		Texture *_depthTiles;
		Model *_skycube;
		
		uint32 _frameID;
		
		float *_depthArray;
		uint32 _depthFrame;
		size_t _depthSize;
		
		machine_uint _maxLights;
		
		RNDefineConstructorlessMeta(Camera, SceneNode)
	};
}

#endif /* __RAYNE_CAMERA_H__ */
