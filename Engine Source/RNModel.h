//
//  RNModel.h
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_MODEL_H__
#define __RAYNE_MODEL_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNMesh.h"
#include "RNMaterial.h"
#include "RNArray.h"

namespace RN
{
	class Model : public Object
	{
	public:
		Model(const std::string& path);
		Model(Mesh *mesh, Material *material, const std::string& name="Unnamed");
		
		virtual ~Model();
		
		uint32 Meshes() const;
		uint32 Materials() const;
		
		Mesh *MeshWithName(const std::string& name) const;
		Mesh *MeshAtIndex(uint32 index) const;
		
		Material *MaterialForMesh(Mesh *mesh) const;
		
	private:
		struct MeshGroup
		{
			std::string name;
			Mesh *mesh;
			Material *material;
		};
		
		struct FeatureDescriptor
		{
			MeshDescriptor meshDescriptor;
			uint8 *data;
			uint32 size;
		};
		
		void ReadModelVersion0(File *file);
		void ReadMaterials(File *file, uint32 count);
		void ReadGroups(File *file, uint32 count);
		
		FeatureDescriptor ReadFeature(File *file);
		
		ObjectArray *_materials;
		std::vector<MeshGroup> _groups;
		
		//std::map<Mesh *, Material *> _materials;
		//std::map<std::string, std::tuple<Mesh *, uint32>> _meshes;
	};
}

#endif /* __RAYNE_MODEL_H__ */
