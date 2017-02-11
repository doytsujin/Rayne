//
//  RNMetalShader.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//


#ifndef __RAYNE_METALSHADER_H_
#define __RAYNE_METALSHADER_H_

#include "RNMetal.h"

namespace RN
{
	class MetalShader : public Shader
	{
	public:
		friend class MetalShaderLibrary;
		friend class MetalStateCoordinator;

		class MetalAttribute : public Attribute
		{
		public:
			MetalAttribute(const String *name, PrimitiveType type, size_t index) :
				Attribute(name, type),
				_index(index)
			{}

			size_t GetIndex() const { return _index; }

		private:
			size_t _index;
		};

		MTLAPI ~MetalShader() override;

		MTLAPI const String *GetName() const override;
		MTLAPI const Array *GetAttributes() const override;

	private:
		MetalShader(void *shader, ShaderLibrary *library);

		void *_shader;
		Array *_attributes;

		RNDeclareMetaAPI(MetalShader, MTLAPI)
	};
}


#endif /* __RAYNE_METALSHADER_H_ */