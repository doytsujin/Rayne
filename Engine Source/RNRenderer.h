//
//  RNRenderer.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_RENDERER_H__
#define __RAYNE_RENDERER_H__

#include "RNBase.h"
#include "RNMatrixQuaternion.h"
#include "RNTexture.h"
#include "RNShader.h"
#include "RNMaterial.h"
#include "RNMesh.h"
#include "RNSkeleton.h"
#include "RNLight.h"

namespace RN
{
	class RenderingObject
	{
	public:
		enum class Type
		{
			Object,
			Instanced,
			Custom
		};
		
		RenderingObject(Type ttype=Type::Object) :
			type(ttype)
		{
			offset = 0;
			count  = 0;
			
			instancingData = 0;
			
			mesh      = 0;
			material  = 0;
			transform = 0;
			skeleton  = 0;
		}
		
		Type type;
		uint32 offset;
		uint32 count;
		
		Mesh     *mesh;
		Material *material;
		Matrix   *transform;
		Skeleton *skeleton;
		
		GLuint instancingData;
		std::function<void (const RenderingObject&)> callback;
	};
	
	class Renderer : public Singleton<Renderer>
	{
	public:
		RNAPI Renderer();
		RNAPI ~Renderer();
		
		RNAPI void BeginFrame(float delta);
		RNAPI void FinishFrame();
		
		RNAPI void BeginCamera(Camera *camera);
		RNAPI void FinishCamera();
		
		RNAPI void RenderObject(RenderingObject object);
		RNAPI void RenderLight(Light *light);
		
		RNAPI void SetDefaultFBO(GLuint fbo);
		RNAPI void SetDefaultFrame(uint32 width, uint32 height);
		
		RNAPI void BindMaterial(Material *material, ShaderProgram *program);
		
		RNAPI uint32 BindTexture(Texture *texture);
		RNAPI uint32 BindTexture(GLenum type, GLuint texture);
		RNAPI void BindVAO(GLuint vao);
		RNAPI void UseShader(ShaderProgram *shader);
		
		RNAPI void SetCullingEnabled(bool enabled);
		RNAPI void SetDepthTestEnabled(bool enabled);
		RNAPI void SetDepthWriteEnabled(bool enabled);
		RNAPI void SetBlendingEnabled(bool enabled);
		RNAPI void SetPolygonOffsetEnabled(bool enabled);
		
		RNAPI void SetCullMode(GLenum cullMode);
		RNAPI void SetDepthFunction(GLenum depthFunction);
		RNAPI void SetBlendFunction(GLenum blendSource, GLenum blendDestination);
		RNAPI void SetPolygonOffset(float factor, float units);
		
	protected:
		RNAPI void UpdateShaderData();
		RNAPI void DrawCamera(Camera *camera, Camera *source, uint32 skyCubeMeshes);
		RNAPI void DrawMesh(Mesh *mesh, uint32 offset, uint32 count);
		RNAPI void DrawMeshInstanced(const RenderingObject& object);
		RNAPI void BindVAO(const std::tuple<ShaderProgram *, Mesh *>& tuple);
		
		RNAPI void CullLights(Camera *camera, Light **lights, machine_uint lightCount, GLuint indicesBuffer, GLuint offsetBuffer);
		
		RNAPI int CreatePointLightList(Camera *camera);
		RNAPI int CreateSpotLightList(Camera *camera);
		RNAPI int CreateDirectionalLightList(Camera *camera);
		
		bool _hasValidFramebuffer;
		
		float _scaleFactor;
		float _time;
		
		std::map<std::tuple<ShaderProgram *, Mesh *>, std::tuple<GLuint, uint32>> _autoVAOs;
		std::vector<Camera *> _flushCameras;
		
		GLuint _defaultFBO;
		uint32 _defaultWidth;
		uint32 _defaultHeight;
		
		uint32 _textureUnit;
		uint32 _maxTextureUnits;
		
		Camera        *_currentCamera;
		Material      *_currentMaterial;
		ShaderProgram *_currentProgram;
		GLuint         _currentVAO;
		
		bool _cullingEnabled;
		bool _depthTestEnabled;
		bool _blendingEnabled;
		bool _depthWrite;
		bool _polygonOffsetEnabled;
		
		GLenum _cullMode;
		GLenum _depthFunc;
		
		GLenum _blendSource;
		GLenum _blendDestination;
		
		float _polygonOffsetFactor;
		float _polygonOffsetUnits;
		
		Camera *_frameCamera;
		std::vector<RenderingObject> _frame;
		std::vector<Light *> _pointLights;
		std::vector<Light *> _spotLights;
		std::vector<Light *> _directionalLights;
		
	private:
		void Initialize();
		void FlushCamera(Camera *camera);
		void DrawCameraStage(Camera *camera, Camera *stage);
		void AllocateLightBufferStorage(size_t indicesSize, size_t offsetSize);
		
		Shader *_copyShader;
		GLuint _copyVAO;
		GLuint _copyVBO;
		GLuint _copyIBO;
		
		Vector4 _copyVertices[4];
		GLshort _copyIndices[6];
		
		int *_lightIndicesBuffer;
		int *_tempLightIndicesBuffer;
		int *_lightOffsetBuffer;
		size_t _lightIndicesBufferSize;
		size_t _lightOffsetBufferSize;
		
		size_t _lightPointDataSize;
		GLuint _lightPointTextures[3];
		GLuint _lightPointBuffers[3];
		
		size_t _lightSpotDataSize;
		GLuint _lightSpotTextures[3];
		GLuint _lightSpotBuffers[3];
		
		std::vector<Vector3> _lightDirectionalDirection;
		std::vector<Vector4> _lightDirectionalColor;
		std::vector<Matrix> _lightDirectionalMatrix;
		std::vector<Texture *> _lightDirectionalDepth;
		
		std::vector<Vector4> _lightSpotPosition;
		std::vector<Vector4> _lightSpotDirection;
		std::vector<Vector4> _lightSpotColor;
		
		std::vector<Vector4> _lightPointPosition;
		std::vector<Vector4> _lightPointColor;
		
		size_t _instancingVBOSize;
		GLuint _instancingVBO;
		SpinLock _lock;
	};
	
	RN_INLINE uint32 Renderer::BindTexture(Texture *texture)
	{
		glActiveTexture((GLenum)(GL_TEXTURE0 + _textureUnit));
		glBindTexture(texture->GLType(), texture->Name());
		
		uint32 unit = _textureUnit;
		
		_textureUnit ++;
		_textureUnit %= _maxTextureUnits;
		
		return unit;
	}
	
	RN_INLINE uint32 Renderer::BindTexture(GLenum type, GLuint texture)
	{
		glActiveTexture((GLenum)(GL_TEXTURE0 + _textureUnit));
		glBindTexture(type, texture);
		
		uint32 unit = _textureUnit;
		
		_textureUnit ++;
		_textureUnit %= _maxTextureUnits;
		
		return unit;
	}
	
	RN_INLINE void Renderer::BindVAO(GLuint vao)
	{
		if(_currentVAO != vao)
		{
			gl::BindVertexArray(vao);
			_currentVAO = vao;
		}
	}
	
	RN_INLINE void Renderer::UseShader(ShaderProgram *shader)
	{
		if(_currentProgram != shader)
		{
			glUseProgram(shader->program);
			if(shader->time != -1)
				glUniform1f(shader->time, _time);
			
			_currentProgram = shader;
		}
	}
	
	
	RN_INLINE void Renderer::SetCullingEnabled(bool enabled)
	{
		if(_cullingEnabled == enabled)
			return;
		
		_cullingEnabled = enabled;
		_cullingEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	}
	
	RN_INLINE void Renderer::SetDepthTestEnabled(bool enabled)
	{
		if(_depthTestEnabled == enabled)
			return;
		
		_depthTestEnabled = enabled;
		_depthTestEnabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
	}
	
	RN_INLINE void Renderer::SetDepthWriteEnabled(bool enabled)
	{
		if(_depthWrite == enabled)
			return;
		
		_depthWrite = enabled;
		glDepthMask(_depthWrite ? GL_TRUE : GL_FALSE);
	}
	
	RN_INLINE void Renderer::SetBlendingEnabled(bool enabled)
	{
		if(_blendingEnabled == enabled)
			return;
		
		_blendingEnabled = enabled;
		_blendingEnabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
	}
	
	RN_INLINE void Renderer::SetPolygonOffsetEnabled(bool enabled)
	{
		if(_polygonOffsetEnabled == enabled)
			return;
		
		_polygonOffsetEnabled = enabled;
		_polygonOffsetEnabled ? glEnable(GL_POLYGON_OFFSET_FILL) : glDisable(GL_POLYGON_OFFSET_FILL);
	}
	
	
	RN_INLINE void Renderer::SetCullMode(GLenum cullMode)
	{
		if(_cullMode == cullMode)
			return;
		
		glCullFace(cullMode);
		_cullMode = cullMode;
	}
	
	RN_INLINE void Renderer::SetDepthFunction(GLenum depthFunction)
	{
		if(_depthFunc == depthFunction)
			return;
		
		glDepthFunc(depthFunction);
		_depthFunc = depthFunction;
	}
	
	RN_INLINE void Renderer::SetBlendFunction(GLenum blendSource, GLenum blendDestination)
	{
		if(_blendSource != blendSource || _blendDestination != blendDestination)
		{
			glBlendFunc(blendSource, blendDestination);
			
			_blendSource = blendSource;
			_blendDestination = blendDestination;
		}
	}
	
	RN_INLINE void Renderer::SetPolygonOffset(float factor, float units)
	{
		if(_polygonOffsetFactor != factor || _polygonOffsetUnits != units)
		{
			glPolygonOffset(factor, units);
			
			_polygonOffsetFactor = factor;
			_polygonOffsetUnits = units;
		}
	}
	
}

#endif /* __RAYNE_RENDERER_H__ */
