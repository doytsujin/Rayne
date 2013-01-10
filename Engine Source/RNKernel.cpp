//
//  RNKernel.cpp
//  Rayne
//
//  Copyright 2013 by Felix Pohl and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNKernel.h"
#include "RNOpenGL.h"

namespace RN
{
	static Kernel *sharedKernel = 0;
	
	Kernel::Kernel()
	{
		_context  = new Context::Context();
		_renderer = new RendererFrontend();
		
		_context->MakeActiveContext();
		
		_window  = new Window::Window("Rayne", this);
		_window->SetContext(_context);
		
		ReadOpenGLExtensions();
		
		// Mesh
		mesh = new Mesh();
		
		MeshDescriptor vertexDescriptor;
		vertexDescriptor.feature = kMeshFeatureVertices;
		vertexDescriptor.elementSize = sizeof(Vector3);
		vertexDescriptor.elementMember = 3;
		vertexDescriptor.elementCount  = 4;
		
		MeshDescriptor colorDescriptor;
		colorDescriptor.feature = kMeshFeatureColor0;
		colorDescriptor.elementSize = sizeof(Color);
		colorDescriptor.elementMember = 4;
		colorDescriptor.elementCount  = 4;
		
		MeshDescriptor indicesDescriptor;
		indicesDescriptor.feature = kMeshFeatureIndices;
		indicesDescriptor.elementSize = sizeof(uint16);
		indicesDescriptor.elementMember = 1;
		indicesDescriptor.elementCount  = 6;
		
		Array<MeshDescriptor> descriptors;
		descriptors.AddObject(vertexDescriptor);
		descriptors.AddObject(colorDescriptor);
		descriptors.AddObject(indicesDescriptor);
		
		
		MeshLODStage *stage = mesh->AddLODStage(descriptors);
		
		Vector3 *vertices = stage->Data<Vector3>(kMeshFeatureVertices);
		Color *colors     = stage->Data<Color>(kMeshFeatureColor0);
		uint16 *indices   = stage->Data<uint16>(kMeshFeatureIndices);
		
		*vertices ++ = Vector3(-32.0f, 32.0f, 0.0f);
		*vertices ++ = Vector3(32.0f, 32.0f, 0.0f);
		*vertices ++ = Vector3(32.0f, -32.0f,  0.0f);
		*vertices ++ = Vector3(-32.0f, -32.0f,  0.0f);
		
		*colors ++ = Color(1.0f, 0.0f, 0.0f, 1.0f);
		*colors ++ = Color(0.0f, 1.0f, 0.0f, 1.0f);
		*colors ++ = Color(0.0f, 0.0f, 1.0f, 1.0f);
		*colors ++ = Color(0.0f, 0.0f, 0.0f, 1.0f);
		
		*indices ++ = 0;
		*indices ++ = 3;
		*indices ++ = 1;
		*indices ++ = 2;
		*indices ++ = 1;
		*indices ++ = 3;
		
		stage->GenerateMesh();
		
		// Shader
		shader = new Shader();
		shader->SetFragmentShader("shader/Test.fsh");
		shader->SetVertexShader("shader/Test.vsh");
		
		shader->Link();
		
		// Material
		material = new Material(shader);
		material->culling = false;
		material->depthtest = false;
		
		_context->DeactivateContext();
	}	
	
	Kernel::~Kernel()
	{
		if(sharedKernel == this)
			sharedKernel = 0;
		
		delete _window;
	}
	
	
	void Kernel::Update(float delta)
	{
		_context->MakeActiveContext();
		_renderer->BeginFrame();
		
		static float rot = 0;
		static float dist = -128.0f;
		
		RenderingIntent intent;
		intent.transform = transform;
		intent.mesh = mesh;
		intent.material = material;
		
		rot += 1.0f;
		//dist -= 1.0f;
		
		transform.MakeTranslate(Vector3(0.0f, 0.0f, dist));
		transform.Rotate(Vector3(0.0f, rot, -rot));
		
		_renderer->PushIntent(intent);
		_renderer->CommitFrame();
		
		_context->DeactivateContext();
		
		CheckOpenGLError("Kernel::Update()");
	}
	
	void Kernel::SetContext(Context *context)
	{
		_context->Release();
		_context = context;
		_context->Retain();
		
		_window->SetContext(_context);
	}
	
	
	bool Kernel::SupportsExtension(const char *extension)
	{
		std::string extensions((const char *)glGetString(GL_EXTENSIONS));
		return (extensions.rfind(extension) != std::string::npos);
	}
	
	void Kernel::CheckOpenGLError(const char *context)
	{
		GLenum error;
		while((error = glGetError()) != GL_NO_ERROR)
		{
			switch(error)
			{
				case GL_INVALID_ENUM:
					printf("OpenGL Error: GL_INVALID_ENUM. Context: %s\n", context);
					break;
					
				case GL_INVALID_VALUE:
					printf("OpenGL Error: GL_INVALID_VALUE. Context: %s\n", context);
					break;
					
				case GL_INVALID_OPERATION:
					printf("OpenGL Error: GL_INVALID_OPERATION. Context: %s\n", context);
					break;
					
				case GL_INVALID_FRAMEBUFFER_OPERATION:
					printf("OpenGL Error: GL_INVALID_FRAMEBUFFER_OPERATION. Context: %s\n", context);
					break;
					
				case GL_OUT_OF_MEMORY:
					printf("OpenGL Error: GL_OUT_OF_MEMORY. Context: %s\n", context);
					break;
					
#if defined(__gl_h_)
				case GL_STACK_UNDERFLOW:
					printf("OpenGL Error: GL_STACK_UNDERFLOW. Context: %s\n", context);
					break;
					
				case GL_STACK_OVERFLOW:
					printf("OpenGL Error: GL_STACK_OVERFLOW. Context: %s\n", context);
					break;
					
				case GL_TABLE_TOO_LARGE:
					printf("OpenGL Error: GL_STACK_OVERFLOW. Context: %s\n", context);
					break;
#endif
					
				default:
					printf("Unknown OpenGL Error: %i. Context: %s\n", error, context);
					break;
			}
			
			fflush(stdout);
		}
	}
	
	
	Kernel *Kernel::SharedInstance()
	{
		if(!sharedKernel)
			sharedKernel = new Kernel();
		
		return sharedKernel;
	}
}
