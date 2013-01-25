//
//  RNModel.cpp
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNModel.h"
#include "RNFile.h"

namespace RN
{
	Model::Model(const std::string& path)
	{
		File *file = new File(path);
		
		_materials = new ObjectArray();
		
		uint32 magic = file->ReadUint32();
		uint32 version = file->ReadUint32();
		
		if(magic == 0x8A7FEED1)
		{
			switch(version)
			{
				case 0:
					ReadModelVersion0(file);
					break;
					
				default:
					break;
			}
		}
		
		file->Release();
	}
	
	Model::Model(Mesh *mesh, Material *material, const std::string& name)
	{
		MeshGroup group;
		group.mesh = mesh->Retain<Mesh>();
		group.material = material;
		group.name = name;
		
		_materials = new ObjectArray();
		
		_groups.push_back(group);
		_materials->AddObject(material);
	}
	
	Model::~Model()
	{
	}
	
	
	uint32 Model::Meshes() const
	{
		return (uint32)_groups.size();
	}
	
	uint32 Model::Materials() const
	{
		return (uint32)_materials->Count();
	}
	
	Mesh *Model::MeshWithName(const std::string& name) const
	{
		for(auto i=_groups.begin(); i!=_groups.end(); i++)
		{
			if(i->name == name)
				return i->mesh;
		}
		
		return 0;
	}
	
	Mesh *Model::MeshAtIndex(uint32 index) const
	{
		return _groups[index].mesh;
	}
	
	Material *Model::MaterialForMesh(Mesh *mesh) const
	{
		for(auto i=_groups.begin(); i!=_groups.end(); i++)
		{			
			if(i->mesh == mesh)
				return i->material;
		}
		
		return 0;
	}
	
	void Model::ReadModelVersion0(File *file)
	{
		uint32 materialCount = file->ReadUint32();
		uint32 groups = file->ReadUint32();
		
		ReadMaterials(file, materialCount);
		ReadGroups(file, groups);
	}
	
	void Model::ReadMaterials(File *file, uint32 count)
	{
		while(count)
		{
			Material *material = new Material(0);
			
			// Culling
			uint8 culling = file->ReadUint8();
			
			material->culling = (culling & 0x1);
			material->cullmode = ((culling >> 1) & 0x1) ? GL_CCW : GL_CW;
			
			// Alpha test
			uint8 alphaTest = file->ReadUint8();
			
			material->alphatest = (alphaTest);
			
			// Diffuse color
			uint32 diffuse = file->ReadUint32();
			uint8 diffuseR = (diffuse >> 24) & 0xff;
			uint8 diffuseG = (diffuse >> 16) & 0xff;
			uint8 diffuseB = (diffuse >> 8)  & 0xff;
			uint8 diffuseA = (diffuse >> 0)  & 0xff;
			
			material->diffuse = Color((diffuseR / 255.0f), (diffuseG / 255.0f), (diffuseB / 255.0f), (diffuseA / 255.0f));
			
			// Grab the textures
			uint32 textures = file->ReadUint32();
			while(textures)
			{
				uint32 length = file->ReadUint32();
				char *name = new char[length + 1];
				
				file->ReadIntoBuffer(name, length);
				name[length] = '\0';
				
				Texture *texture = new Texture(std::string(name), Texture::FormatRGBA8888);
				material->AddTexture(texture);
				texture->Release();
				
				delete [] name;
				
				textures --;
			}
			
			_materials->AddObject(material);
			material->Release();
			
			count --;
		}
	}
	
	void Model::ReadGroups(File *file, uint32 count)
	{
		while(count)
		{
			MeshGroup group;
			
			Material *material = (Material *)_materials->ObjectAtIndex(file->ReadUint32());
			Mesh *mesh = new Mesh();
			
			uint32 features = file->ReadUint32();
			
			// Read the name
			{
				uint32 length = file->ReadUint32();
				char *name = new char[length + 1];
				
				file->ReadIntoBuffer(name, length);
				name[length] = '\0';
				
				group.name = std::string(name);
				delete [] name;
			}
			
			// Read all features of the mesh
			std::vector<FeatureDescriptor> descriptors;
			Array<MeshDescriptor> meshDescriptors;
			
			while(features)
			{
				FeatureDescriptor descriptor = ReadFeature(file);
				
				descriptors.push_back(descriptor);
				meshDescriptors.AddObject(descriptor.meshDescriptor);
				
				features --;
			}
			
			// Create the mesh
			MeshLODStage *stage = mesh->AddLODStage(meshDescriptors);
			
			for(auto i=descriptors.begin(); i!=descriptors.end(); i++)
			{
				FeatureDescriptor descriptor = *i;
				uint8 *target = stage->Data<uint8>(descriptor.meshDescriptor.feature);
				uint8 *source = descriptor.data;
				
				std::copy(source, source + descriptor.size, target);
			}
			
			mesh->UpdateMesh();
			
			group.material = material;
			group.mesh = mesh;
			
			_groups.push_back(group);
			count --;
		}
	}
	
	Model::FeatureDescriptor Model::ReadFeature(File *file)
	{
		MeshDescriptor meshDescriptor;
		meshDescriptor.feature = (MeshFeature)file->ReadUint32();
		meshDescriptor.elementCount = file->ReadUint32();
		meshDescriptor.elementSize = file->ReadUint32();
		meshDescriptor.elementMember = file->ReadUint32();
		
		uint32 size = file->ReadUint32();
		uint8 *data = new uint8[size];
		
		file->ReadIntoBuffer(data, size);
		
		FeatureDescriptor descriptor;
		descriptor.meshDescriptor = meshDescriptor;
		descriptor.data = data;
		descriptor.size = size;
		
		return descriptor;
	}
}