//
//  RNUIImage.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_UIIMAGE_H__
#define __RAYNE_UIIMAGE_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNTexture.h"
#include "RNUIGeometry.h"

namespace RN
{
	namespace UI
	{
		class Image : public Object
		{
		public:
			Image(Texture *texture);
			Image(const std::string& file);
			~Image() override;
			
			static Image *WithFile(const std::string& file);
			
			void SetAtlas(const Atlas& atlas, bool normalized=true);
			void SetEdgeInsets(const EdgeInsets& insets);
			
			const Atlas& Atlas() const { return _atlas; }
			const EdgeInsets& Insets() const { return _insets; }
			
			Texture *Texture() const { return _texture; }
			
			uint32 Width(bool atlasApplied=true) const;
			uint32 Height(bool atlasApplied=true) const;
			
		private:
			class Texture *_texture;
			class Atlas _atlas;
			EdgeInsets _insets;
			
			RNDefineConstructorlessMeta(Image, Object)
		};
	}
}

#endif /* __RAYNE_UIIMAGE_H__ */