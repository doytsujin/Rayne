//
//  RNShader.h
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_SHADER_H__
#define __RAYNE_SHADER_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNFile.h"
#include "RNArray.h"

namespace RN
{
	class Shader : public Object
	{
	public:		
		RNAPI Shader();
		RNAPI Shader(const std::string& shader, bool link=true);
		RNAPI virtual ~Shader();
		
		static Shader *WithFile(const std::string& shader, bool link=true);
		
		RNAPI void Define(const std::string& define);
		RNAPI void Define(const std::string& define, const std::string& value);
		RNAPI void Define(const std::string& define, int32 value);
		RNAPI void Define(const std::string& define, float value);
		RNAPI void Undefine(const std::string& define);
		
		RNAPI void SetVertexShader(const std::string& path);
		RNAPI void SetVertexShader(File *file);
		
		RNAPI void SetFragmentShader(const std::string& path);
		RNAPI void SetFragmentShader(File *file);
		
		RNAPI void SetGeometryShader(const std::string& path);
		RNAPI void SetGeometryShader(File *file);
		
		RNAPI void Link();
		RNAPI bool IsLinked() const;
		
		GLuint program;
		
		GLuint matProj;
		GLuint matProjInverse;
		GLuint matView;
		GLuint matViewInverse;
		GLuint matModel;
		GLuint matModelInverse;
		GLuint matViewModel;
		GLuint matViewModelInverse;
		GLuint matProjView;
		GLuint matProjViewInverse;
		GLuint matProjViewModel;
		GLuint matProjViewModelInverse;
		GLuint matBones;
		
		GLuint imatModel;
		GLuint imatModelInverse;
		
		GLuint vertPosition;
		GLuint vertNormal;
		GLuint vertTangent;
		GLuint vertTexcoord0;
		GLuint vertTexcoord1;
		GLuint vertColor0;
		GLuint vertColor1;
		GLuint vertBoneWeights;
		GLuint vertBoneIndices;
		
		GLuint time;
		GLuint frameSize;
		GLuint clipPlanes;
		
		GLuint lightPosition;
		GLuint lightColor;
		GLuint lightCount;
		GLuint lightList;
		GLuint lightListOffset;
		GLuint lightListPosition;
		GLuint lightListColor;
		GLuint lightTileSize;
		
		Array<GLuint> texlocations;
		Array<GLuint> targetmaplocations;
		Array<GLuint> fraglocations;
		GLuint depthmap;
		
	private:
		struct ShaderDefine
		{
			std::string name;
			std::string value;
		};
		
		void SetShaderForType(const std::string& path, GLenum type);
		void SetShaderForType(File *file, GLenum type);
		void AddDefines();
		
		Array<ShaderDefine> _defines;
		
		GLuint _vertexShader;
		GLuint _fragmentShader;
		GLuint _geometryShader;
	};
}

#endif /* __RAYNE_SHADER_H__ */
