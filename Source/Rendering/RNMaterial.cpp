//
//  RNMaterial.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNMaterial.h"

namespace RN
{
	RNDefineMeta(Material, Object)

	MaterialDescriptor::MaterialDescriptor() :
		fragmentShader(nullptr),
		vertexShader(nullptr),
		_textures(new Array())
	{}

	MaterialDescriptor::~MaterialDescriptor()
	{
		SafeRelease(_textures);
	}


	void MaterialDescriptor::AddTexture(Texture *texture)
	{
		_textures->AddObject(texture);
	}
	void MaterialDescriptor::RemoveAllTextures()
	{
		_textures->RemoveAllObjects();
	}
	const Array *MaterialDescriptor::GetTextures() const
	{
		return _textures;
	}





	Material::Material(const MaterialDescriptor &descriptor) :
		_fragmentShader(SafeRetain(descriptor.fragmentShader)),
		_vertexShader(SafeRetain(descriptor.vertexShader)),
		_depthMode(DepthMode::Less),
		_depthWriteEnabled(true),
		_ambientColor(1.0f, 1.0f, 1.0f, 1.0f),
		_diffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
		_specularColor(1.0f, 1.0f, 1.0f, 4.0f),
		_emissiveColor(0.0f, 0.0f, 0.0f, 0.0f),
		_textures(SafeCopy(descriptor.GetTextures()))
	{
		RN_ASSERT(!_fragmentShader || _fragmentShader->GetType() == Shader::Type::Fragment, "Fragment shader must be a fragment shader");
		RN_ASSERT(_vertexShader->GetType() == Shader::Type::Vertex, "Vertex shader must be a vertex shader");
	}

	Material::Material(const Material *other) :
		_fragmentShader(SafeRetain(other->_fragmentShader)),
		_vertexShader(SafeRetain(other->_vertexShader)),
		_depthMode(other->_depthMode),
		_depthWriteEnabled(other->_depthWriteEnabled),
		_ambientColor(other->_ambientColor),
		_diffuseColor(other->_diffuseColor),
		_specularColor(other->_specularColor),
		_emissiveColor(other->_emissiveColor),
		_textures(SafeRetain(other->_textures))
	{}

	Material *Material::WithDescriptor(const MaterialDescriptor &descriptor)
	{
		Material *material = new Material(descriptor);
		return material;
	}

	Material::~Material()
	{
		SafeRelease(_fragmentShader);
		SafeRelease(_vertexShader);
		SafeRelease(_textures);
	}

	void Material::SetDepthWriteEnabled(bool depthWrite)
	{
		_depthWriteEnabled = depthWrite;
	}
	void Material::SetDepthMode(DepthMode mode)
	{
		_depthMode = mode;
	}

	void Material::SetAmbientColor(const Color &color)
	{
		_ambientColor = color;
	}
	void Material::SetDiffuseColor(const Color &color)
	{
		_diffuseColor = color;
	}
	void Material::SetSpecularColor(const Color &color)
	{
		_specularColor = color;
	}
	void Material::SetEmissiveColor(const Color &color)
	{
		_emissiveColor = color;
	}
}
