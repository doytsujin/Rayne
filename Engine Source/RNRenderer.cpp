//
//  RNRenderer.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNRenderer.h"
#include "RNMatrix.h"
#include "RNQuaternion.h"
#include "RNCamera.h"
#include "RNKernel.h"
#include "RNThreadPool.h"
#include "RNAlgorithm.h"

#define kRNRendererMaxVAOAge 300
#define kRNRendererFastPathLightCount 10

#define kRNRendererPointLightListIndicesIndex 0
#define kRNRendererPointLightListOffsetIndex  1
#define kRNRendererPointLightListDataIndex    2

#define kRNRendererSpotLightListIndicesIndex 0
#define kRNRendererSpotLightListOffsetIndex  1
#define kRNRendererSpotLightListDataIndex    2

namespace RN
{
	Renderer::Renderer()
	{
		_defaultFBO = 0;
		_defaultWidth = _defaultHeight = 0;
		
		_currentMaterial = 0;
		_currentProgram	 = 0;
		_currentCamera   = 0;
		_currentVAO      = 0;
		
		_scaleFactor = Kernel::SharedInstance()->ScaleFactor();
		_time = 0.0f;
		_mode = Mode::ModeWorld;
		
		_hdrExposure = 1.0f;
		_hdrWhitePoint = 1.0f;
		
		// Default OpenGL state
		// TODO: Those initial values are gathered from the OpenGL 4.0 man pages, not sure if they are true for all versions!
		_cullingEnabled   = false;
		_depthTestEnabled = false;
		_blendingEnabled  = false;
		_depthWrite       = false;
		
		_cullMode  = GL_CCW;
		_depthFunc = GL_LESS;
		
		_blendSource      = GL_ONE;
		_blendDestination = GL_ZERO;
		
		// Setup framebuffer copy stuff
		_copyShader = 0;
		
		_copyVertices[0] = Vector4(-1.0f, 1.0f, 0.0f, 1.0f);
		_copyVertices[1] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		_copyVertices[2] = Vector4(1.0f, -1.0f, 1.0f, 0.0f);
		_copyVertices[3] = Vector4(-1.0f, -1.0f, 0.0f, 0.0f);
		
		_copyIndices[0] = 0;
		_copyIndices[1] = 3;
		_copyIndices[2] = 1;
		_copyIndices[3] = 2;
		_copyIndices[4] = 1;
		_copyIndices[5] = 3;
		
		_lightIndicesBuffer = 0;
		_lightOffsetBuffer = 0;
		_tempLightIndicesBuffer = 0;
		
		_lightIndicesBufferSize = 0;
		_lightOffsetBufferSize = 0;
		
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint *)&_maxTextureUnits);
		_textureUnit = 0;
		
		_hasValidFramebuffer = false;
		_frameCamera = 0;
		
		Initialize();
	}
	
	Renderer::~Renderer()
	{
	}
	
	void Renderer::Initialize()
	{
		_copyShader = new Shader("shader/rn_CopyFramebuffer");
		
		gl::GenVertexArrays(1, &_copyVAO);
		gl::BindVertexArray(_copyVAO);
		
		glGenBuffers(1, &_copyVBO);
		glGenBuffers(1, &_copyIBO);
		
		glBindBuffer(GL_ARRAY_BUFFER, _copyVBO);
		glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(GLfloat), _copyVertices, GL_STATIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _copyIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLshort), _copyIndices, GL_STATIC_DRAW);
		
		gl::BindVertexArray(0);
		
#if !(RN_PLATFORM_IOS)
		// Point lights
		_lightPointDataSize = 0;
		
		glGenTextures(3, _lightPointTextures);
		glGenBuffers(3, _lightPointBuffers);
		
		// light index offsets
		glBindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListIndicesIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightPointBuffers[kRNRendererPointLightListIndicesIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, _lightPointBuffers[kRNRendererPointLightListIndicesIndex]);
		
		// Light indices
		glBindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListOffsetIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightPointBuffers[kRNRendererPointLightListOffsetIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32I, _lightPointBuffers[kRNRendererPointLightListOffsetIndex]);
		
		// Light Data
		glBindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListDataIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightPointBuffers[kRNRendererPointLightListDataIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _lightPointBuffers[kRNRendererPointLightListDataIndex]);
		
		// Spot lights
		_lightSpotDataSize = 0;
		
		glGenTextures(3, _lightSpotTextures);
		glGenBuffers(3, _lightSpotBuffers);
		
		// light index offsets
		glBindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListIndicesIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightSpotBuffers[kRNRendererSpotLightListIndicesIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, _lightSpotBuffers[kRNRendererSpotLightListIndicesIndex]);
		
		// Light indices
		glBindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListOffsetIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightSpotBuffers[kRNRendererSpotLightListOffsetIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32I, _lightSpotBuffers[kRNRendererSpotLightListOffsetIndex]);
		
		// Light Data
		glBindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListDataIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightSpotBuffers[kRNRendererSpotLightListDataIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _lightSpotBuffers[kRNRendererSpotLightListDataIndex]);
		
#endif
	}
	
	
	void Renderer::SetDefaultFBO(GLuint fbo)
	{
		_defaultFBO = fbo;
		_hasValidFramebuffer = false;
	}
	
	void Renderer::SetDefaultFrame(uint32 width, uint32 height)
	{
		_defaultWidth  = width;
		_defaultHeight = height;
	}
	
	void Renderer::SetMode(Mode mode)
	{
		_mode = mode;
	}
	
	// ---------------------
	// MARK: -
	// MARK: Additional helper
	// ---------------------
	
	void Renderer::UpdateShaderData()
	{
		const Matrix& projectionMatrix = _currentCamera->projectionMatrix;
		const Matrix& inverseProjectionMatrix = _currentCamera->inverseProjectionMatrix;
		
		const Matrix& viewMatrix = _currentCamera->viewMatrix;
		const Matrix& inverseViewMatrix = _currentCamera->inverseViewMatrix;
		
		if(_currentProgram->frameSize != -1)
		{
			const Rect& frame = _currentCamera->Frame();
			glUniform4f(_currentProgram->frameSize, 1.0f/frame.width/_scaleFactor, 1.0f/frame.height/_scaleFactor, frame.width * _scaleFactor, frame.height * _scaleFactor);
		}
		
		if(_currentProgram->hdrSettings != -1)
			glUniform4f(_currentProgram->hdrSettings, _hdrExposure, _hdrWhitePoint, 0.0f, 0.0f);
		
		if(_currentProgram->clipPlanes != -1)
			glUniform2f(_currentProgram->clipPlanes, _currentCamera->clipnear, _currentCamera->clipfar);
		
		
		if(_currentProgram->fogPlanes != -1)
			glUniform2f(_currentProgram->fogPlanes, _currentCamera->fognear, 1.0f/(_currentCamera->fogfar-_currentCamera->fognear));
		
		if(_currentProgram->fogColor != -1)
			glUniform4fv(_currentProgram->fogColor, 1, &_currentCamera->fogcolor.r);
		
		if(_currentProgram->clipPlane != -1)
			glUniform4fv(_currentProgram->clipPlane, 1, &_currentCamera->clipplane.x);
		
		if(_currentProgram->matProj != -1)
			glUniformMatrix4fv(_currentProgram->matProj, 1, GL_FALSE, projectionMatrix.m);
		
		if(_currentProgram->matProjInverse != -1)
			glUniformMatrix4fv(_currentProgram->matProjInverse, 1, GL_FALSE, inverseProjectionMatrix.m);
		
		if(_currentProgram->matView != -1)
			glUniformMatrix4fv(_currentProgram->matView, 1, GL_FALSE, viewMatrix.m);
		
		if(_currentProgram->matViewInverse != -1)
			glUniformMatrix4fv(_currentProgram->matViewInverse, 1, GL_FALSE, inverseViewMatrix.m);
		
		if(_currentProgram->matProjView != -1)
		{
			Matrix projectionViewMatrix = projectionMatrix * viewMatrix;
			glUniformMatrix4fv(_currentProgram->matProjView, 1, GL_FALSE, projectionViewMatrix.m);
		}
		
		if(_currentProgram->matProjViewInverse != -1)
		{
			Matrix inverseProjectionViewMatrix = inverseProjectionMatrix * inverseViewMatrix;
			glUniformMatrix4fv(_currentProgram->matProjViewInverse, 1, GL_FALSE, inverseProjectionViewMatrix.m);
		}
		
		if(_currentProgram->viewPosition != -1)
		{
			const Vector3& position = _currentCamera->WorldPosition();
			glUniform3fv(_currentProgram->viewPosition, 1, &position.x);
		}
	}
	
	void Renderer::AllocateLightBufferStorage(size_t indicesSize, size_t offsetSize)
	{
		if(indicesSize > _lightIndicesBufferSize)
		{
			delete _lightIndicesBuffer;
			delete _tempLightIndicesBuffer;
			
			_lightIndicesBufferSize = indicesSize;
			
			_lightIndicesBuffer = new int[_lightIndicesBufferSize];
			_tempLightIndicesBuffer = new int[_lightIndicesBufferSize];
		}
		
		if(offsetSize > _lightOffsetBufferSize)
		{
			delete _lightOffsetBuffer;
			
			_lightOffsetBufferSize = offsetSize;
			_lightOffsetBuffer = new int[_lightOffsetBufferSize];
		}
	}
	
#define Distance(plane, op, r) { \
	float dot = (position.x * plane.normal.x + position.y * plane.normal.y + position.z * plane.normal.z); \
	distance = dot - plane.d; \
	if(distance op r) \
		continue; \
	}
	
#define DistanceExpect(plane, op, r, c) { \
	float dot = (position.x * plane.normal.x + position.y * plane.normal.y + position.z * plane.normal.z); \
	distance = dot - plane.d; \
	if(__builtin_expect((distance op r), c)) \
		continue; \
	}
	
	void Renderer::CullLights(Camera *camera, Light **lights, machine_uint lightCount, GLuint indicesBuffer, GLuint offsetBuffer)
	{
		Rect rect = camera->Frame();
		int tilesWidth  = ceil(rect.width / camera->LightTiles().x);
		int tilesHeight = ceil(rect.height / camera->LightTiles().y);
		
		size_t i = 0;
		size_t tileCount = tilesWidth * tilesHeight;
		
		size_t lightindicesSize = tilesWidth * tilesHeight * lightCount;
		size_t lightindexoffsetSize = tilesWidth * tilesHeight * 2;
		
		if(lightCount == 0)
		{
			AllocateLightBufferStorage(1, lightindexoffsetSize);
			
			std::fill(_lightIndicesBuffer, _lightIndicesBuffer + 1, 0);
			std::fill(_lightOffsetBuffer, _lightOffsetBuffer + lightindexoffsetSize, 0);
			
			// Indicies
			glBindBuffer(GL_TEXTURE_BUFFER, indicesBuffer);
			glBufferData(GL_TEXTURE_BUFFER, 1 * sizeof(int), 0, GL_DYNAMIC_DRAW);
			glBufferData(GL_TEXTURE_BUFFER, 1 * sizeof(int), _lightIndicesBuffer, GL_DYNAMIC_DRAW);
			
			// Offsets
			glBindBuffer(GL_TEXTURE_BUFFER, offsetBuffer);
			glBufferData(GL_TEXTURE_BUFFER, lightindexoffsetSize * sizeof(int), 0, GL_DYNAMIC_DRAW);
			glBufferData(GL_TEXTURE_BUFFER, lightindexoffsetSize * sizeof(int), _lightOffsetBuffer, GL_DYNAMIC_DRAW);
			
			return;
		}
		
		
		Vector3 corner1 = camera->ToWorld(Vector3(-1.0f, -1.0f, 1.0f));
		Vector3 corner2 = camera->ToWorld(Vector3(1.0f, -1.0f, 1.0f));
		Vector3 corner3 = camera->ToWorld(Vector3(-1.0f, 1.0f, 1.0f));
		
		Vector3 dirx = (corner2-corner1)/rect.width*camera->LightTiles().x;
		Vector3 diry = (corner3-corner1)/rect.height*camera->LightTiles().y;
		
		const Vector3& camPosition = camera->WorldPosition();
		float *depthArray = camera->DepthArray();
		
		Vector3 camdir = camera->Forward();
		
		std::vector<size_t> indicesCount(tileCount);
		AllocateLightBufferStorage(lightindicesSize, lightindexoffsetSize);
		
		ThreadPool::Batch batch = ThreadPool::SharedInstance()->OpenBatch();
		
		for(int y=0; y<tilesHeight; y++)
		{
			for(int x=0; x<tilesWidth; x++)
			{
				size_t index = i ++;
				batch->AddTask([&, x, y, index]() {
					Plane plleft;
					Plane plright;
					Plane pltop;
					Plane plbottom;
					
					Plane plfar;
					Plane plnear;
					
					plright.SetPlane(camPosition, corner1+dirx*x+diry*(y+1.0f), corner1+dirx*x+diry*(y-1.0f));
					plleft.SetPlane(camPosition, corner1+dirx*(x+1.0f)+diry*(y+1.0f), corner1+dirx*(x+1.0f)+diry*(y-1.0f));
					plbottom.SetPlane(camPosition, corner1+dirx*(x-1.0f)+diry*(y+1.0f), corner1+dirx*(x+1.0f)+diry*(y+1.0f));
					pltop.SetPlane(camPosition, corner1+dirx*(x-1.0f)+diry*y, corner1+dirx*(x+1.0f)+diry*y);
					
					plnear.SetPlane(camPosition + camdir * depthArray[index * 2 + 0], camdir);
					plfar.SetPlane(camPosition + camdir * depthArray[index * 2 + 1], camdir);
					
					size_t lightIndicesCount = 0;
					int *lightPointIndices = _tempLightIndicesBuffer + (index * lightCount);
					
					for(size_t i=0; i<lightCount; i++)
					{
						Light *light = lights[i];
						
						const Vector3& position = light->_worldPosition;
						const float range = light->_range;
						float distance, dr, dl, dt, db;
						DistanceExpect(plright, >, range, true);
						dr = distance;
						DistanceExpect(plleft, <, -range, true);
						dl = distance;
						DistanceExpect(plbottom, <, -range, true);
						db = distance;
						DistanceExpect(pltop, >, range, true);
						dt = distance;
						
						float sqrange = range*range;
						
						if(dr > 0.0f && db < 0.0f && dr*dr+db*db > sqrange)
							continue;
						if(dr > 0.0f && dt > 0.0f && dr*dr+dt*dt > sqrange)
							continue;
						if(dl < 0.0f && db < 0.0f && dl*dl+db*db > sqrange)
							continue;
						if(dl < 0.0f && dt > 0.0f && dl*dl+dt*dt > sqrange)
							continue;
						
						Distance(plnear, >, range);
						Distance(plfar, <, -range);
						
						lightPointIndices[lightIndicesCount ++] = static_cast<int>(i);
					}
					
					indicesCount[index] = lightIndicesCount;
				});
			}
		}
		
		batch->Commit();
		batch->Wait();
		
		size_t lightIndicesCount = 0;
		size_t lightIndexOffsetCount = 0;
		
		for(i=0; i<tileCount; i++)
		{
			size_t tileIndicesCount = indicesCount[i];
			int *tileIndices = _tempLightIndicesBuffer + (i * lightCount);
			
			size_t previous = lightIndicesCount;
			_lightOffsetBuffer[lightIndexOffsetCount ++] = static_cast<int>(previous);
			
			if(tileIndicesCount > 0)
			{
				std::copy(tileIndices, tileIndices + tileIndicesCount, _lightIndicesBuffer + lightIndicesCount);
				lightIndicesCount += tileIndicesCount;
			}
			
			_lightOffsetBuffer[lightIndexOffsetCount ++] = static_cast<int>(lightIndicesCount - previous);
		}
		
		// Indices
		if(lightIndicesCount == 0)
			lightIndicesCount ++;
		
		glBindBuffer(GL_TEXTURE_BUFFER, indicesBuffer);
		glBufferData(GL_TEXTURE_BUFFER, lightIndicesCount * sizeof(int), 0, GL_DYNAMIC_DRAW);
		glBufferData(GL_TEXTURE_BUFFER, lightIndicesCount * sizeof(int), _lightIndicesBuffer, GL_DYNAMIC_DRAW);
		
		// Offsets
		glBindBuffer(GL_TEXTURE_BUFFER, offsetBuffer);
		glBufferData(GL_TEXTURE_BUFFER, lightIndexOffsetCount * sizeof(int), 0, GL_DYNAMIC_DRAW);
		glBufferData(GL_TEXTURE_BUFFER, lightIndexOffsetCount * sizeof(int), _lightOffsetBuffer, GL_DYNAMIC_DRAW);
	}
	
	int Renderer::CreatePointLightList(Camera *camera)
	{
		Light **lights = _pointLights.data();
		machine_uint lightCount = _pointLights.size();
		
		lightCount = MIN(camera->MaxLightsPerTile(), lightCount);
		
		_lightPointPosition.clear();
		_lightPointColor.clear();
		
		if(camera->DepthTiles())
		{
			GLuint indicesBuffer = _lightPointBuffers[kRNRendererPointLightListIndicesIndex];
			GLuint offsetBuffer = _lightPointBuffers[kRNRendererPointLightListOffsetIndex];
			
			CullLights(camera, lights, lightCount, indicesBuffer, offsetBuffer);
			
			// Write the position, range and colour of the lights
			Vector4 *lightData = 0;
			size_t lightDataSize = lightCount * 2 * sizeof(Vector4);
			
			if(lightDataSize == 0) // Makes sure that we don't end up with an empty buffer
				lightDataSize = 2 * sizeof(Vector4);
			
			glBindBuffer(GL_TEXTURE_BUFFER, _lightPointBuffers[kRNRendererPointLightListDataIndex]);
			if(lightDataSize > _lightPointDataSize)
			{
				glBufferData(GL_TEXTURE_BUFFER, _lightPointDataSize, 0, GL_DYNAMIC_DRAW);
				glBufferData(GL_TEXTURE_BUFFER, lightDataSize, 0, GL_DYNAMIC_DRAW);
				
				_lightPointDataSize = lightDataSize;
			}
 
			 lightData = (Vector4 *)glMapBufferRange(GL_TEXTURE_BUFFER, 0, _lightPointDataSize, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT);
			 
			 for(machine_uint i=0; i<lightCount; i++)
			 {
				 Light *light = lights[i];
				 const Vector3& position = light->WorldPosition();
				 const Vector3& color = light->ResultColor();
				 
				 if(i < kRNRendererFastPathLightCount)
				 {
					 _lightPointPosition.emplace_back(Vector4(position, light->Range()));
					 _lightPointColor.emplace_back(Vector4(color, 0.0f));
				 }
				 
				 lightData[i * 2 + 0] = Vector4(position, light->Range());
				 lightData[i * 2 + 1] = Vector4(color, 0.0f);
			 }
			 
			 glUnmapBuffer(GL_TEXTURE_BUFFER);
			 glBindBuffer(GL_TEXTURE_BUFFER, 0);
		}
		else
		{
			return 0;
		}
		
		return static_cast<int>(lightCount);
	}
	
	int Renderer::CreateSpotLightList(Camera *camera)
	{
		Light **lights = _spotLights.data();
		machine_uint lightCount = _spotLights.size();
		
		lightCount = MIN(camera->MaxLightsPerTile(), lightCount);
		
		_lightSpotPosition.clear();
		_lightSpotDirection.clear();
		_lightSpotColor.clear();
		
		if(camera->DepthTiles())
		{
			GLuint indicesBuffer = _lightSpotBuffers[kRNRendererSpotLightListIndicesIndex];
			GLuint offsetBuffer = _lightSpotBuffers[kRNRendererSpotLightListOffsetIndex];
			
			CullLights(camera, lights, lightCount, indicesBuffer, offsetBuffer);
			
			// Write the position, range, colour and direction of the lights
			Vector4 *lightData = 0;
			size_t lightDataSize = lightCount * 3 * sizeof(Vector4);
			
			if(lightDataSize == 0)
				lightDataSize = 3 * sizeof(Vector4); // Make sure that we don't end up with an empty buffer
				
			glBindBuffer(GL_TEXTURE_BUFFER, _lightSpotBuffers[kRNRendererSpotLightListDataIndex]);
			if(lightDataSize > _lightSpotDataSize)
			{
				glBufferData(GL_TEXTURE_BUFFER, _lightSpotDataSize, 0, GL_DYNAMIC_DRAW);
				glBufferData(GL_TEXTURE_BUFFER, lightDataSize, 0, GL_DYNAMIC_DRAW);
				
				_lightSpotDataSize = lightDataSize;
			}
			
			lightData = (Vector4 *)glMapBufferRange(GL_TEXTURE_BUFFER, 0, _lightSpotDataSize, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT);
			
			for(machine_uint i=0; i<lightCount; i++)
			{
				Light *light = lights[i];
				const Vector3& position  = light->WorldPosition();
				const Vector3& color     = light->ResultColor();
				const Vector3& direction = light->Forward();
				
				if(i < kRNRendererFastPathLightCount)
				{
					_lightSpotPosition.emplace_back(Vector4(position, light->Range()));
					_lightSpotDirection.emplace_back(Vector4(direction, light->Angle()));
					_lightSpotColor.emplace_back(Vector4(color, 0.0f));
				}
				
				lightData[i * 3 + 0] = Vector4(position, light->Range());
				lightData[i * 3 + 1] = Vector4(color, 0.0f);
				lightData[i * 3 + 2] = Vector4(direction, light->Angle());
			}
			
			glUnmapBuffer(GL_TEXTURE_BUFFER);
			glBindBuffer(GL_TEXTURE_BUFFER, 0);
		}
		else
		{
			return 0;
		}
		
		return static_cast<int>(lightCount);
	}
	
	int Renderer::CreateDirectionalLightList(Camera *camera)
	{
		Light **lights = _directionalLights.data();
		machine_uint lightCount = _directionalLights.size();
		
		_lightDirectionalDirection.clear();
		_lightDirectionalColor.clear();
		
		for(machine_uint i=0; i<lightCount; i++)
		{
			Light *light = lights[i];
			const Vector3& color = light->ResultColor();
			const Vector3& direction = light->Forward();
			
			_lightDirectionalDirection.push_back(direction);
			_lightDirectionalColor.emplace_back(Vector4(color, light->_shadow ? 1.0f : 0.0f));
			
			if(light->_shadow)
			{
				if(camera == light->_shadowcam || light->_shadowcams.ContainsObject(camera))
				{
					_lightDirectionalMatrix.clear();
					_lightDirectionalDepth.clear();
					
					for(int i = 0; i < 4; i++)
					{
						_lightDirectionalMatrix.push_back(light->_shadowmats[i]);
					}
					
					if(light->_shadowcam != 0)
					{
						_lightDirectionalDepth.push_back(light->_shadowcam->Storage()->DepthTarget());
					}
					else
					{
						_lightDirectionalDepth.push_back(light->_shadowcams.FirstObject<Camera>()->Storage()->DepthTarget());
					}
				}
			}
		}
		
		return static_cast<int>(lightCount);
	}
	
#undef Distance
		
	// ---------------------
	// MARK: -
	// MARK: Binding
	// ---------------------
	
	void Renderer::BindVAO(const std::tuple<ShaderProgram *, Mesh *>& tuple)
	{
		auto iterator = _autoVAOs.find(tuple);
		GLuint vao;
		
		if(iterator == _autoVAOs.end())
		{
			ShaderProgram *shader = std::get<0>(tuple);
			Mesh *mesh = std::get<1>(tuple);
			
			gl::GenVertexArrays(1, &vao);
			gl::BindVertexArray(vao);
			
			glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO());
			
			if(mesh->SupportsFeature(kMeshFeatureIndices))
			{
				GLuint ibo = mesh->IBO();
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			}
			
			// Vertices
			if(shader->attPosition != -1 && mesh->SupportsFeature(kMeshFeatureVertices))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureVertices);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureVertices);
				
				glEnableVertexAttribArray(shader->attPosition);
				glVertexAttribPointer(shader->attPosition, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Normals
			if(shader->attNormal != -1 && mesh->SupportsFeature(kMeshFeatureNormals))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureNormals);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureNormals);
				
				glEnableVertexAttribArray(shader->attNormal);
				glVertexAttribPointer(shader->attNormal, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Tangents
			if(shader->attTangent != -1 && mesh->SupportsFeature(kMeshFeatureTangents))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureTangents);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureTangents);
				
				glEnableVertexAttribArray(shader->attTangent);
				glVertexAttribPointer(shader->attTangent, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Texcoord0
			if(shader->attTexcoord0 != -1 && mesh->SupportsFeature(kMeshFeatureUVSet0))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureUVSet0);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureUVSet0);
				
				glEnableVertexAttribArray(shader->attTexcoord0);
				glVertexAttribPointer(shader->attTexcoord0, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Texcoord1
			if(shader->attTexcoord1 != -1 && mesh->SupportsFeature(kMeshFeatureUVSet1))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureUVSet1);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureUVSet1);
				
				glEnableVertexAttribArray(shader->attTexcoord1);
				glVertexAttribPointer(shader->attTexcoord1, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Color0
			if(shader->attColor0 != -1 && mesh->SupportsFeature(kMeshFeatureColor0))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureColor0);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureColor0);
				
				glEnableVertexAttribArray(shader->attColor0);
				glVertexAttribPointer(shader->attColor0, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Color1
			if(shader->attColor1 != -1 && mesh->SupportsFeature(kMeshFeatureColor1))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureColor1);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureColor1);
				
				glEnableVertexAttribArray(shader->attColor1);
				glVertexAttribPointer(shader->attColor1, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Bone Weights
			if(shader->attBoneWeights != -1 && mesh->SupportsFeature(kMeshFeatureBoneWeights))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureBoneWeights);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureBoneWeights);
				
				glEnableVertexAttribArray(shader->attBoneWeights);
				glVertexAttribPointer(shader->attBoneWeights, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Bone Indices
			if(shader->attBoneIndices != -1 && mesh->SupportsFeature(kMeshFeatureBoneIndices))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureBoneIndices);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureBoneIndices);
				
				glEnableVertexAttribArray(shader->attBoneIndices);
				glVertexAttribPointer(shader->attBoneIndices, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			_autoVAOs[tuple] = std::tuple<GLuint, uint32>(vao, 0);
			_currentVAO = vao;
			
			return;
		}
		
		uint32& age = std::get<1>(iterator->second);
		
		vao = std::get<0>(iterator->second);
		age = 0;
		
		BindVAO(vao);
	}
	
	void Renderer::BindMaterial(Material *material, ShaderProgram *program)
	{
		bool changedShader = (program != _currentProgram);
		UseShader(program);
		
		Material *surfaceMaterial = _currentCamera->Material();
		if(!surfaceMaterial)
			surfaceMaterial = material;
		
		if(changedShader || material != _currentMaterial)
		{
			_textureUnit = 0;
			
			const Array& textures = !(surfaceMaterial->override & Material::OverrideTextures)? material->Textures() : surfaceMaterial->Textures();
			const std::vector<GLuint>& textureLocations = program->texlocations;
			
			if(textureLocations.size() > 0)
			{
				machine_uint textureCount = MIN(textureLocations.size(), textures.Count());
				
				for(machine_uint i=0; i<textureCount; i++)
				{
					GLint location = textureLocations[i];
					Texture *texture = textures.ObjectAtIndex<Texture>(i);
					
					glUniform1i(location, BindTexture(texture));
					
					location = program->texinfolocations[i];
					if(location != -1)
						glUniform4f(location, 1.0f/static_cast<float>(texture->Width()), 1.0f/static_cast<float>(texture->Height()), texture->Width(), texture->Height());
				}
			}
		}

#define PickAttribute(_override, attribute) (material->override & Material::_override) ? material->attribute : surfaceMaterial->attribute
		
		SetCullingEnabled(PickAttribute(OverrideCulling, culling));
		SetCullMode(PickAttribute(OverrideCullmode, cullmode));

		SetDepthTestEnabled(PickAttribute(OverrideDepthtest, depthtest));
		SetDepthFunction(PickAttribute(OverrideDepthtestMode, depthtestmode));
		
		SetPolygonOffsetEnabled(PickAttribute(OverridePolygonOffset, polygonOffset));
		SetPolygonOffset(PickAttribute(OverridePolygonOffset, polygonOffsetFactor), PickAttribute(OverridePolygonOffset, polygonOffsetUnits));
		
		if(material->override & Material::OverrideDepthwrite)
		{
			SetDepthWriteEnabled((material->depthwrite && _currentCamera->AllowsDepthWrite()));
		}
		else
		{
			SetDepthWriteEnabled((surfaceMaterial->depthwrite && _currentCamera->AllowsDepthWrite()));
		}
		
		SetBlendingEnabled(PickAttribute(OverrideBlending, blending));
		
		if(material->override & Material::OverrideBlendmode)
		{
			SetBlendFunction(material->blendSource, material->blendDestination);
		}
		else
		{
			SetBlendFunction(surfaceMaterial->blendSource, surfaceMaterial->blendDestination);
		}
		
#undef PickAttribute
		
		_currentMaterial = material;
	}
	
	// ---------------------
	// MARK: -
	// MARK: Frame handling
	// ---------------------
	
	void Renderer::BeginFrame(float delta)
	{
		_time += delta;
		
		for(auto i=_autoVAOs.begin(); i!=_autoVAOs.end();)
		{
			uint32& age = std::get<1>(i->second);
			
			if((++ age) > kRNRendererMaxVAOAge)
			{
				i = _autoVAOs.erase(i);
				continue;
			}
			
			i ++;
		}
	}
	
	void Renderer::FinishFrame()
	{
		if(!_hasValidFramebuffer)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBO);
			
			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				_flushCameras.clear();
				return;
			}
			
			_hasValidFramebuffer = true;
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glViewport(0, 0, _defaultWidth * _scaleFactor, _defaultHeight * _scaleFactor);
		
		for(auto iterator=_flushCameras.begin(); iterator!=_flushCameras.end(); iterator++)
		{
			Camera *camera  = *iterator;
			FlushCamera(camera);
		}
		
		_flushCameras.clear();
		_debugFrameUI.clear();
		_debugFrameWorld.clear();
	}
	
	void Renderer::FlushCamera(Camera *camera)
	{
		_currentCamera = camera;
		_textureUnit = 0;
		
		SetDepthTestEnabled(false);
		SetCullMode(GL_CCW);
		
		if(camera->UseBlending())
		{
			SetBlendingEnabled(true);
			SetBlendFunction(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			SetBlendingEnabled(false);
		}
		
		if(_currentVAO != _copyVAO)
		{
			gl::BindVertexArray(_copyVAO);
			_currentVAO = _copyVAO;
			
			glBindBuffer(GL_ARRAY_BUFFER, _copyVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _copyIBO);
		}
		
		ShaderProgram *program = _copyShader->ProgramOfType(ShaderProgram::TypeNormal);
		
		UseShader(program);
		UpdateShaderData();
		
		glEnableVertexAttribArray(program->attPosition);
		glVertexAttribPointer(program->attPosition,  2, GL_FLOAT, GL_FALSE, 16, (const void *)0);
		
		glEnableVertexAttribArray(program->attTexcoord0);
		glVertexAttribPointer(program->attTexcoord0, 2, GL_FLOAT, GL_FALSE, 16, (const void *)8);
		
		uint32 targetmaps = MIN((uint32)program->targetmaplocations.size(), camera->RenderTargets());
		if(targetmaps >= 1)
		{
			Texture *texture = camera->RenderTarget(0);
			GLuint location = program->targetmaplocations.front();
			
			glUniform1i(location, BindTexture(texture));
			
			location = program->targetmapinfolocations.front();
			if(location != -1)
				glUniform4f(location, 1.0f/static_cast<float>(texture->Width()), 1.0f/static_cast<float>(texture->Height()), texture->Width(), texture->Height());
		}
		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		
		glDisableVertexAttribArray(program->attPosition);
		glDisableVertexAttribArray(program->attTexcoord0);
	}
	
	void Renderer::DrawCameraStage(Camera *camera, Camera *stage)
	{
		Material *material = stage->Material();
		
		ShaderProgram *program = material->Shader()->ProgramWithLookup(material->Lookup() + ShaderLookup(ShaderProgram::TypeNormal));
		
		_currentCamera = stage;
		_textureUnit = 0;
		
		SetDepthTestEnabled(false);
		SetBlendingEnabled(false);
		SetCullMode(GL_CCW);
		
		if(_currentVAO != _copyVAO)
		{
			gl::BindVertexArray(_copyVAO);
			_currentVAO = _copyVAO;
			
			glBindBuffer(GL_ARRAY_BUFFER, _copyVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _copyIBO);
		}
		
		BindMaterial(material, program);
		UpdateShaderData();
		
		glEnableVertexAttribArray(program->attPosition);
		glVertexAttribPointer(program->attPosition,  2, GL_FLOAT, GL_FALSE, 16, (const void *)0);
		
		glEnableVertexAttribArray(program->attTexcoord0);
		glVertexAttribPointer(program->attTexcoord0, 2, GL_FLOAT, GL_FALSE, 16, (const void *)8);
		
		uint32 targetmaps = MIN((uint32)program->targetmaplocations.size(), stage->RenderTargets());
		for(uint32 i=0; i<targetmaps; i++)
		{
			Texture *texture = camera->RenderTarget(i);
			GLuint location = program->targetmaplocations[i];
			
			glUniform1i(location, BindTexture(texture));
			
			location = program->targetmapinfolocations[i];
			if(location != -1)
				glUniform4f(location, 1.0f/static_cast<float>(texture->Width()), 1.0f/static_cast<float>(texture->Height()), texture->Width(), texture->Height());
		}
		
		if(program->depthmap != -1)
		{
			Texture *depthmap = camera->Storage()->DepthTarget();
			if(depthmap)
			{
				glUniform1i(program->depthmap, BindTexture(depthmap));
				
				if(program->depthmapinfo != -1)
					glUniform4f(program->depthmapinfo, 1.0f/static_cast<float>(depthmap->Width()), 1.0f/static_cast<float>(depthmap->Height()), depthmap->Width(), depthmap->Height());
			}
		}
		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		
		glDisableVertexAttribArray(program->attPosition);
		glDisableVertexAttribArray(program->attTexcoord0);
	}
	
	// ---------------------
	// MARK: -
	// MARK: Rendering
	// ---------------------
	
	void Renderer::BeginCamera(Camera *camera)
	{
		RN_ASSERT0(_frameCamera == 0);
		_frameCamera = camera;
		
		_currentProgram  = 0;
		_currentMaterial = 0;
		_currentCamera   = 0;
		_currentVAO      = 0;
		_textureUnit     = 0;
	}
	
	void Renderer::DrawCamera(Camera *camera, Camera *source, uint32 skyCubeMeshes)
	{
		bool changedCamera = true;
		bool changedShader;
		bool changedMaterial;
		
		SetDepthWriteEnabled(true);
		
		camera->Bind();
		camera->PrepareForRendering();
		
		if(_currentMaterial)
			SetDepthWriteEnabled(_currentMaterial->depthwrite);
		
		_currentCamera = camera;
		
		bool wantsFog = _currentCamera->usefog;
		bool wantsClipPlane = _currentCamera->useclipplane;
		
		Matrix identityMatrix;
		
		if(!source)
		{
			// Sort the objects
			if(!(camera->CameraFlags() & Camera::FlagNoSorting))
			{
				auto begin = _frame.begin();
				std::advance(begin, skyCubeMeshes);
			
				ParallelSort(begin, _frame.end(), [](const RenderingObject& a, const RenderingObject& b) {
					const Material *materialA = a.material;
					const Material *materialB = b.material;
					
					if(materialA->blending != materialB->blending)
					{
						if(!materialB->blending)
							return false;
						
						return true;
					}
					
					if(materialA->discard != materialB->discard)
					{
						if(!materialB->discard)
							return false;
						
						return true;
					}
					
					if(materialA->Shader() != materialB->Shader())
					{
						return (materialA->Shader() < materialB->Shader());
					}
					
					return a.mesh < b.mesh;
				});
			}

			Material *surfaceMaterial = camera->Material();
			if(!surfaceMaterial)
			{
				switch(_mode)
				{
					case Mode::ModeWorld:
						_frame.insert(_frame.end(), _debugFrameWorld.begin(), _debugFrameWorld.end());
						break;
						
					case Mode::ModeUI:
						_frame.insert(_frame.end(), _debugFrameUI.begin(), _debugFrameUI.end());
						break;
				}
			}
			
			// Create the light lists for the camera
			int lightPointCount = CreatePointLightList(camera);
			int lightSpotCount  = CreateSpotLightList(camera);
			int lightDirectionalCount = CreateDirectionalLightList(camera);
			
			// Update the shader
			const Matrix& projectionMatrix = camera->projectionMatrix;
			const Matrix& inverseProjectionMatrix = camera->inverseProjectionMatrix;
			
			const Matrix& viewMatrix = camera->viewMatrix;
			const Matrix& inverseViewMatrix = camera->inverseViewMatrix;
			
			Matrix projectionViewMatrix = projectionMatrix * viewMatrix;
			Matrix inverseProjectionViewMatrix = inverseProjectionMatrix * inverseViewMatrix;
			
			size_t objectsCount = _frame.size();
			size_t i = (camera->_flags & Camera::FlagNoSky) ? skyCubeMeshes : 0;
			
			for(; i<objectsCount; i++)
			{
				RenderingObject& object = _frame[i];
				if(object.prepare)
					object.prepare(this, object);
				
				Mesh     *mesh = object.mesh;
				Material *material = object.material;
				Shader   *shader = surfaceMaterial ? surfaceMaterial->Shader() : material->Shader();
				
				Matrix& transform = object.transform ? *object.transform : identityMatrix;
				Matrix inverseTransform = transform.Inverse();
				
				// Check if we can use instancing here
				bool wantsInstancing = (object.type == RenderingObject::Type::Instanced);
				if(wantsInstancing)
				{
					if(!shader->SupportsProgramOfType(ShaderProgram::TypeInstanced))
						continue;
				}
				
				// Grab the correct shader program
				uint32 programTypes = 0;
				ShaderProgram *program = 0;
				
				bool wantsDiscard = material->discard;
				if(surfaceMaterial && !(material->override & Material::OverrideDiscard))
					wantsDiscard = surfaceMaterial->discard;
				
				if(object.skeleton && shader->SupportsProgramOfType(ShaderProgram::TypeAnimated))
					programTypes |= ShaderProgram::TypeAnimated;
				
				bool wantsLighting = material->lighting;
				if(surfaceMaterial)
				{
					wantsLighting = surfaceMaterial->lighting;
				}
				
				if(wantsLighting && shader->SupportsProgramOfType(ShaderProgram::TypeLighting))
				{
					programTypes |= ShaderProgram::TypeLighting;
					
					if(_lightDirectionalDepth.size() > 0)
						programTypes |= ShaderProgram::TypeDirectionalShadows;
				}
				
				if(wantsFog && shader->SupportsProgramOfType(ShaderProgram::TypeFog))
					programTypes |= ShaderProgram::TypeFog;
				
				if(wantsClipPlane && shader->SupportsProgramOfType(ShaderProgram::TypeClipPlane))
					programTypes |= ShaderProgram::TypeClipPlane;
				
				if(wantsInstancing)
					programTypes |= ShaderProgram::TypeInstanced;
				
				if(wantsDiscard && shader->SupportsProgramOfType(ShaderProgram::TypeDiscard))
					programTypes |= ShaderProgram::TypeDiscard;
				
				// Set lighting defines
				std::vector<ShaderDefine> defines;
				if(lightPointCount > 0)
					defines.emplace_back(ShaderDefine("RN_POINT_LIGHTS", MIN(lightPointCount, kRNRendererFastPathLightCount)));
				if(lightSpotCount > 0)
					defines.emplace_back(ShaderDefine("RN_SPOT_LIGHTS", MIN(lightSpotCount, kRNRendererFastPathLightCount)));
				if(lightDirectionalCount > 0)
					defines.emplace_back(ShaderDefine("RN_DIRECTIONAL_LIGHTS", lightDirectionalCount));
				
				if(lightPointCount < kRNRendererFastPathLightCount)
					defines.emplace_back(ShaderDefine("RN_POINT_LIGHTS_FASTPATH", ""));
				if(lightSpotCount < kRNRendererFastPathLightCount)
					defines.emplace_back(ShaderDefine("RN_SPOT_LIGHTS_FASTPATH", ""));
				
				program = shader->ProgramWithLookup(material->Lookup() + ShaderLookup(programTypes) + ShaderLookup(defines));
				
				changedShader   = (_currentProgram != program);
				changedMaterial = (_currentMaterial != material);
				
				BindMaterial(material, program);
				
				// Update the shader data
				if(changedShader || changedCamera)
				{
					UpdateShaderData();
					changedCamera = false;
					changedShader = true;
				}
				
				if(changedShader)
				{
					// Light data
					if(program->lightPointCount != -1)
						glUniform1i(program->lightPointCount, lightPointCount);
					
					if(program->lightPointPosition != -1)
						glUniform4fv(program->lightPointPosition, lightPointCount, (float*)_lightPointPosition.data());
					
					if(program->lightPointColor != -1)
						glUniform4fv(program->lightPointColor, lightPointCount, (float*)_lightPointColor.data());
					
					
					if(program->lightSpotCount != -1)
						glUniform1i(program->lightSpotCount, lightSpotCount);
					
					if(program->lightSpotPosition != -1)
						glUniform4fv(program->lightSpotPosition, lightSpotCount, (float*)_lightSpotPosition.data());
					
					if(program->lightSpotDirection != -1)
						glUniform4fv(program->lightSpotDirection, lightSpotCount, (float*)_lightSpotDirection.data());
					
					if(program->lightSpotColor != -1)
						glUniform4fv(program->lightSpotColor, lightSpotCount, (float*)_lightSpotColor.data());
					
					
					if(program->lightDirectionalCount != -1)
						glUniform1i(program->lightDirectionalCount, lightDirectionalCount);
					
					if(program->lightDirectionalDirection != -1)
						glUniform3fv(program->lightDirectionalDirection, lightDirectionalCount, (float*)_lightDirectionalDirection.data());
					
					if(program->lightDirectionalColor != -1)
						glUniform4fv(program->lightDirectionalColor, lightDirectionalCount, (float*)_lightDirectionalColor.data());
					
					if(program->lightDirectionalMatrix != -1)
					{
						float *data = reinterpret_cast<float *>(_lightDirectionalMatrix.data());
						glUniformMatrix4fv(program->lightDirectionalMatrix, (GLuint)_lightDirectionalMatrix.size(), GL_FALSE, data);
					}
					
					if(camera->DepthTiles() != 0)
					{
						if(program->lightTileSize != -1)
						{
							Rect rect = camera->Frame();
							int tilesWidth  = ceil(rect.width / camera->LightTiles().x);
							int tilesHeight = ceil(rect.height / camera->LightTiles().y);
							
							Vector2 lightTilesSize = camera->LightTiles() * _scaleFactor;
							Vector2 lightTilesCount = Vector2(tilesWidth, tilesHeight);
							
							glUniform4f(program->lightTileSize, lightTilesSize.x, lightTilesSize.y, lightTilesCount.x, lightTilesCount.y);
						}
					}
					
					if(program->discardThreshold != -1)
					{
						float threshold = material->discardThreshold;
						
						if(surfaceMaterial && !(material->override & Material::OverrideDiscardThreshold))
							threshold = surfaceMaterial->discardThreshold;
						
						glUniform1f(program->discardThreshold, threshold);
					}
				}
				
				if(changedShader || changedMaterial)
				{
					if(program->lightDirectionalDepth != -1 && _lightDirectionalDepth.size() > 0)
					{
						uint32 textureUnit = BindTexture(_lightDirectionalDepth.front());
						glUniform1i(program->lightDirectionalDepth, textureUnit);
					}
					
					if(camera->DepthTiles() != 0)
					{
						// Point lights
						if(program->lightPointList != -1)
						{
							uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListIndicesIndex]);
							glUniform1i(program->lightPointList, textureUnit);
						}
						
						if(program->lightPointListOffset != -1)
						{
							uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListOffsetIndex]);
							glUniform1i(program->lightPointListOffset, textureUnit);
						}
						
						if(program->lightPointListData != -1)
						{
							uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListDataIndex]);
							glUniform1i(program->lightPointListData, textureUnit);
						}
						
						// Spot lights
						if(program->lightSpotList != -1)
						{
							uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListIndicesIndex]);
							glUniform1i(program->lightSpotList, textureUnit);
						}
						
						if(program->lightSpotListOffset!= -1)
						{
							uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListOffsetIndex]);
							glUniform1i(program->lightSpotListOffset, textureUnit);
						}
						
						if(program->lightSpotListData != -1)
						{
							uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListDataIndex]);
							glUniform1i(program->lightSpotListData, textureUnit);
						}
					}
					
					if(program->ambient != -1)
						glUniform4fv(program->ambient, 1, &material->ambient.r);
					
					if(program->diffuse != -1)
						glUniform4fv(program->diffuse, 1, &material->diffuse.r);
					
					if(program->emissive != -1)
						glUniform4fv(program->emissive, 1, &material->emissive.r);
					
					if(program->specular != -1)
						glUniform4fv(program->specular, 1, &material->specular.r);
					
					if(program->shininess != -1)
						glUniform1f(program->shininess, material->shininess);
				}
				
				if(wantsInstancing)
				{
					DrawMeshInstanced(object);
					continue;
				}
				
				// More updates
				if(object.skeleton && program->matBones != -1)
				{
					const float *data = reinterpret_cast<const float *>(object.skeleton->Matrices().data());
					glUniformMatrix4fv(program->matBones, object.skeleton->NumBones(), GL_FALSE, data);
				}
				
				if(program->matModel != -1)
					glUniformMatrix4fv(program->matModel, 1, GL_FALSE, transform.m);
				
				if(program->matModelInverse != -1)
					glUniformMatrix4fv(program->matModelInverse, 1, GL_FALSE, inverseTransform.m);
				
				if(object.rotation)
				{
					if(program->matNormal != -1)
						glUniformMatrix4fv(program->matNormal, 1, GL_FALSE, object.rotation->RotationMatrix().m);
					
					if(program->matNormalInverse != -1)
						glUniformMatrix4fv(program->matNormalInverse, 1, GL_FALSE, object.rotation->RotationMatrix().Inverse().m);
				}
				
				if(program->matViewModel != -1)
				{
					Matrix viewModel = viewMatrix * transform;
					glUniformMatrix4fv(program->matViewModel, 1, GL_FALSE, viewModel.m);
				}
				
				if(program->matViewModelInverse != -1)
				{
					Matrix viewModel = inverseViewMatrix * inverseTransform;
					glUniformMatrix4fv(program->matViewModelInverse, 1, GL_FALSE, viewModel.m);
				}
				
				if(program->matProjViewModel != -1)
				{
					Matrix projViewModel = projectionViewMatrix * transform;
					glUniformMatrix4fv(program->matProjViewModel, 1, GL_FALSE, projViewModel.m);
				}
				
				if(program->matProjViewModelInverse != -1)
				{
					Matrix projViewModelInverse = inverseProjectionViewMatrix * inverseTransform;
					glUniformMatrix4fv(program->matProjViewModelInverse, 1, GL_FALSE, projViewModelInverse.m);
				}
				
				if(object.type == RenderingObject::Type::Custom)
				{
					object.callback(this, object);
					continue;
				}
				
				DrawMesh(mesh, object.offset, object.count);
			}
		}
		else
		{
			DrawCameraStage(source, camera);
		}
		
		camera->Unbind();
	}
	
	void Renderer::FinishCamera()
	{
		Camera *previous = _frameCamera;
		Camera *camera = _frameCamera;
		
		// Skycube
		Model *skyCube = camera->SkyCube();
		uint32 skyCubeMeshes = 0;
		
		Matrix cameraRotation;
		cameraRotation.MakeRotate(camera->WorldRotation());
		
		if(skyCube)
		{
			skyCubeMeshes = skyCube->Meshes(0);
			
			for(uint32 j=0; j<skyCubeMeshes; j++)
			{
				RenderingObject object;
				
				object.mesh = skyCube->MeshAtIndex(0, j);
				object.material = skyCube->MaterialAtIndex(0, j);
				object.transform = &cameraRotation;
				object.skeleton = 0;
				
				_frame.insert(_frame.begin(), std::move(object));
			}
		}
		
		ParallelSort(_directionalLights.begin(), _directionalLights.end(), [](Light *a, Light *b) {
			return (a->Shadow());
        });
		
		// Render loop
		DrawCamera(camera, 0, skyCubeMeshes);
		Camera *lastPipeline = camera;
		
		auto pipelines = camera->PostProcessingPipelines();
		
		for(PostProcessingPipeline *pipeline : pipelines)
		{
			for(auto j=pipeline->stages.begin(); j!=pipeline->stages.end(); j++)
			{
				Camera *stage = j->Camera();
				
				switch(j->StageMode())
				{
					case RenderStage::Mode::ReRender:
						DrawCamera(stage, 0, skyCubeMeshes);
						break;
						
					case RenderStage::Mode::ReUseConnection:
						DrawCamera(stage, j->Connection(), skyCubeMeshes);
						break;
						
					case RenderStage::Mode::ReUseCamera:
						DrawCamera(stage, camera, skyCubeMeshes);
						break;
						
					case RenderStage::Mode::ReUsePipeline:
						DrawCamera(stage, lastPipeline, skyCubeMeshes);
						break;
						
					case RenderStage::Mode::ReUsePreviousStage:
						DrawCamera(stage, previous, skyCubeMeshes);
						break;
				}
				
				previous = stage;
			}
			
			lastPipeline = previous;
		}
		
		 if(!(previous->CameraFlags() & Camera::FlagHidden))
			 _flushCameras.push_back(previous);
		
		// Cleanup of the frame
		_frameCamera = 0;
		
		_frame.clear();
		_pointLights.clear();
		_spotLights.clear();
		_directionalLights.clear();
	}
	
	void Renderer::DrawMesh(Mesh *mesh, uint32 offset, uint32 count)
	{
		bool usesIndices = mesh->SupportsFeature(kMeshFeatureIndices);
		MeshDescriptor *descriptor = usesIndices ? mesh->Descriptor(kMeshFeatureIndices) : mesh->Descriptor(kMeshFeatureVertices);
		
		BindVAO(std::tuple<ShaderProgram *, Mesh *>(_currentProgram, mesh));
		
		GLsizei glCount = static_cast<GLsizei>(descriptor->elementCount);
		if(count != 0)
			glCount = MIN(glCount, count);
		
		if(usesIndices)
		{
			GLenum type = (descriptor->elementSize == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
			glDrawElements(mesh->Mode(), glCount, type, (void *)offset);
		}
		else
		{
			glDrawArrays(mesh->Mode(), 0, glCount);
		}
	}
	
	void Renderer::DrawMeshInstanced(const RenderingObject& object)
	{
		Mesh *mesh = object.mesh;
		MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureIndices);
		
		BindVAO(std::tuple<ShaderProgram *, Mesh *>(_currentProgram, mesh));
		RN_ASSERT0(_currentProgram->instancingData != -1);
		
		uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, object.instancingData);
		glUniform1i(_currentProgram->instancingData, textureUnit);
		
		if(descriptor)
		{
			GLenum type = (descriptor->elementSize == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
			glDrawElementsInstanced(mesh->Mode(), (GLsizei)descriptor->elementCount, type, 0, (GLsizei)object.count);
		}
		else
		{
			descriptor = mesh->Descriptor(kMeshFeatureVertices);
			glDrawArraysInstanced(mesh->Mode(), 0, (GLsizei)descriptor->elementCount, (GLsizei)object.count);
		}
	}
	
	void Renderer::RenderObject(RenderingObject object)
	{
		_frame.push_back(std::move(object));
	}
	
	void Renderer::RenderDebugObject(RenderingObject object, Mode mode)
	{
		switch(mode)
		{
			case Mode::ModeWorld:
				_debugFrameWorld.push_back(std::move(object));
				break;
				
			case Mode::ModeUI:
				_debugFrameUI.push_back(std::move(object));
				break;
		}		
	}
	
	void Renderer::RenderLight(Light *light)
	{
		switch(light->LightType())
		{
			case Light::TypePointLight:
				_pointLights.push_back(light);
				break;
				
			case Light::TypeSpotLight:
				_spotLights.push_back(light);
				break;
				
			case Light::TypeDirectionalLight:
				_directionalLights.push_back(light);
				break;
				
			default:
				break;
		}
	}
}
