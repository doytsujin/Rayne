//
//  RNCamera.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//


#ifndef __RAYNE_CAMERA_H__
#define __RAYNE_CAMERA_H__

#include "../Base/RNBase.h"
#include "../Math/RNPlane.h"
#include "../Math/RNRect.h"
#include "../Rendering/RNFramebuffer.h"
#include "RNSceneNode.h"

namespace RN
{
	class Window;
	class Camera : public SceneNode
	{
	public:
		friend class Scene;

		RN_OPTIONS(Flags, uint32,
				   NoSky        = (1 << 5),
				   NoSorting    = (1 << 6),
				   NoRender     = (1 << 7),
				   NoFlush      = (1 << 8),
				   NoDepthWrite = (1 << 9),
				   ForceFlush   = (1 << 10),

				   InheritPosition   = (1 << 12),
				   InheritFrame      = (1 << 13),
				   InheritProjection = (1 << 14),

				   Fullscreen = (1 << 20),
				   Orthogonal = (1 << 21),
				   Hidden     = (1 << 22),

				   UseFog          = (1 << 24),
				   UseClipPlanes   = (1 << 25),

				   Defaults = (Fullscreen | UseFog),
				   Inherit  = (InheritFrame | InheritPosition | InheritProjection));



		RNAPI Camera();
		RNAPI Camera(const Vector2 &size);

/*		RNAPI Camera(const Vector2 &size, Texture *target);
		RNAPI Camera(const Vector2 &size, Texture *target, Flags flags);
		RNAPI Camera(const Vector2 &size, Texture *target, Flags flags, RenderStorage::BufferFormat format);

		RNAPI Camera(const Vector2 &size, Texture::Format targetFormat);
		RNAPI Camera(const Vector2 &size, Texture::Format targetFormat, Flags flags);
		RNAPI Camera(const Vector2 &size, Texture::Format targetFormat, Flags flags, RenderStorage::BufferFormat format, float scaleFactor=0.0f);

		RNAPI Camera(const Vector2 &size, RenderStorage *storage, Flags flags, float scaleFactor=0.0f);*/
		RNAPI ~Camera();

		RNAPI void DidUpdate(ChangeSet changeSet) override;

		RNAPI void SetFrame(const Rect &frame);
		RNAPI void SetFlags(Flags flags);
		RNAPI void SetClearColor(const Color &color);
//		RNAPI void SetMaterial(Material *material);
//		RNAPI void SetLightManager(LightManager *lightManager);
//		RNAPI void SetSky(Model *sky);
		RNAPI void SetLODCamera(Camera *camera);
		RNAPI void SetPriority(int32 priority);
		RNAPI void SetFOV(float fov);
		RNAPI void SetAspectRatio(float ratio);
		RNAPI void SetClipNear(float near);
		RNAPI void SetClipFar(float far);
		RNAPI void SetFogColor(Color color);
		RNAPI void SetFogNear(float near);
		RNAPI void SetFogFar(float far);
		RNAPI void SetAmbientColor(Color color);
		RNAPI void SetClipPlane(const Plane &clipPlane);
		RNAPI void SetOrthogonalFrustum(float top, float bottom, float left, float right);
		RNAPI void SetProjectionMatrix(const Matrix &projectionMatrix);

		RNAPI void Update(float delta) override;
		RNAPI void UpdateEditMode(float delta) override;

		RNAPI Vector3 ToWorld(const Vector3 &dir);

		RNAPI virtual bool InFrustum(const Vector3 &position, float radius);
		RNAPI virtual bool InFrustum(const Sphere &sphere);
		RNAPI virtual bool InFrustum(const AABB &aabb);

		RNAPI const Vector3 &GetFrustumCenter();
		RNAPI float GetFrustumRadius();

		Framebuffer *GetFramebuffer() const { return _framebuffer; }
		const Color &GetClearColor() const { return _clearColor; }
		RNAPI const Rect &GetFrame();
//		Material *GetMaterial() const { return _material; }
		Flags GetFlags() const { return _flags; }
		Camera *GetLODCamera() const { return _lodCamera ? _lodCamera : const_cast<Camera *>(this); }
//		Model *GetSky() const { return _sky; }
//		LightManager *GetLightManager();
		int32 GetPriority() const { return _priority; }
		float GetFOV() const { return _fov; }
		float GetAspectRatio() const { return _aspect; }
		float GetClipNear() const { return _clipNear; }
		float GetClipFar() const { return _clipFar; }
		const Color &GetFogColor() const { return _fogColor; }
		float GetFogNear() const { return _fogNear; }
		float GetFogFar() const { return _fogFar; }
		const Color &GetAmbientColor() const { return _ambient; }
		const Plane &GetClipPlane() const { return _clipPlane; }
		const Matrix &GetProjectionMatrix() const { return _projectionMatrix; }
		const Matrix &GetInverseProjectionMatrix() const { return _inverseProjectionMatrix; }
		const Matrix &GetViewMatrix() const { return _viewMatrix; }
		const Matrix &GetInverseViewMatrix() const { return _inverseViewMatrix; }

/*		RNAPI PostProcessingPipeline *AddPostProcessingPipeline(const std::string &name, int32 priority);
		RNAPI PostProcessingPipeline *GetPostProcessingPipeline(const std::string &name);
		RNAPI void AddPostProcessingPipeline(PostProcessingPipeline *pipeline);
		RNAPI void RemovePostProcessingPipeline(PostProcessingPipeline *pipeline);

		const std::vector<PostProcessingPipeline *>& GetPostProcessingPipelines() const { return _PPPipelines; }*/

	private:
		void PostUpdate(Renderer *renderer);
		void UpdateProjection(Renderer *renderer);
		void UpdateFrustum();

		Vector3 __ToWorld(const Vector3 &dir);
//		Matrix MakeShadowSplit(Camera *camera, Light *light, float near, float far);
		void Initialize();

		IntrusiveList<Camera>::Member _cameraSceneEntry;
		Framebuffer *_framebuffer;

		Rect _frame;
		Color _clearColor;
		Flags _flags;
		float _scaleFactor;
		bool _fixedScaleFactor;
		bool _dirtyProjection;
		bool _dirtyFrustum;
		int32 _priority;

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
		} frustums;

		float _fov;
		float _aspect;
		float _clipNear;
		float _clipFar;

		float _fogNear;
		float _fogFar;

		Color _fogColor;
		Color _ambient;

		Plane _clipPlane;
//		LightManager *_lightManager;

		Matrix _projectionMatrix;
		Matrix _inverseProjectionMatrix;
		Matrix _viewMatrix;
		Matrix _inverseViewMatrix;

		float _orthoLeft;
		float _orthoRight;
		float _orthoTop;
		float _orthoBottom;

		bool _prefersLightManager;

//		Material *_material;
		Camera *_lodCamera;
//		Model *_sky;

//		std::vector<PostProcessingPipeline *> _PPPipelines;
//		std::map<std::string, PostProcessingPipeline *> _namedPPPipelines;

		__RNDeclareMetaInternal(Camera);
	};
}


#endif /* __RAYNE_CAMERA_H__ */
