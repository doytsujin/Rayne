//
//  RNUILabel.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_UILABEL_H__
#define __RAYNE_UILABEL_H__

#include "RNBase.h"
#include "RNColor.h"
#include "RNUIView.h"
#include "RNUIFont.h"

namespace RN
{
	namespace UI
	{
		class Label : public View
		{
		public:
			Label();
			~Label();
			
			void SetText(String *text);
			void SetFont(Font *font);
			
		protected:
			void Update() override;
			bool Render(RenderingObject& object) override;
			
		private:
			void Initialize();
			
			Font *_font;
			Color _color;
			
			String *_text;
			Mesh *_mesh;
			
			bool _isDirty;
			
			RNDefineMeta(Label, View)
		};
	}
}

#endif /* __RAYNE_UILABEL_H__ */
