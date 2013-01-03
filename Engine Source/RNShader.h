//
//  RNShader.h
//  Rayne
//
//  Copyright 2013 by Felix Pohl and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_SHADER_H__
#define __RAYNE_SHADER_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNFile.h"

namespace RN
{
	class Shader : public Object, public BlockingProxy
	{
	public:		
		Shader();
		virtual ~Shader();
		
		void SetVertexShader(const std::string& path);
		void SetVertexShader(File *file);
		
		void SetFragmentShader(const std::string& path);
		void SetFragmentShader(File *file);
		
		void SetGeometryShader(const std::string& path);
		void SetGeometryShader(File *file);
		
		void Link();
		
		GLuint program;
		
		GLuint matProj;
		GLuint matProjInverse;
		GLuint matView;
		GLuint matViewInverse;
		GLuint matModel;
		GLuint matModelInverse;
		GLuint matProjViewModel;
		GLuint matProjViewModelInverse;
		
		GLuint position;
		GLuint texcoord0;
		GLuint texcoord1;
		GLuint color0;
		GLuint color1;
		
	private:
		void SetShaderForType(const std::string& path, GLenum type);
		void SetShaderForType(File *file, GLenum type);
		void SetShaderForType(const std::vector<uint8>& data, GLenum type);
		
		GLuint _vertexShader;
		GLuint _fragmentShader;
		GLuint _geometryShader;
	};
}

#endif /* __RAYNE_SHADER_H__ */
