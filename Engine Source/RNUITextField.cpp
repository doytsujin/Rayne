//
//  RNUITextField.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNUITextField.h"
#include "RNUIStyle.h"

namespace RN
{
	namespace UI
	{
		RNDeclareMeta(TextField)
		
		TextField::TextField(Dictionary *style)
		{
			RN_ASSERT(style, "Style mustn't be NULL!");
			
			Initialize();
			
			Style *styleSheet = Style::GetSharedInstance();
			Dictionary *background = style->GetObjectForKey<Dictionary>(RNCSTR("background"));
			Dictionary *insets = style->GetObjectForKey<Dictionary>(RNCSTR("contentInsets"));
			Dictionary *clipInsets = style->GetObjectForKey<Dictionary>(RNCSTR("clipInsets"));
			
			if(background)
			{
				Atlas atlas = Style::ParseAtlas(background->GetObjectForKey<Dictionary>(RNCSTR("atlas")));
				EdgeInsets insets = Style::ParseEdgeInsets(background->GetObjectForKey<Dictionary>(RNCSTR("insets")));
				Texture *texture = styleSheet->TextureWithName(background->GetObjectForKey<String>(RNCSTR("texture")));
				
				Image *image = new Image(texture);
				image->SetAtlas(atlas, false);
				image->SetEdgeInsets(insets);
				
				_background->SetImage(image);
				
				image->Release();
			}
			
			_contentInsets = Style::ParseEdgeInsets(insets);
			_background->SetClipInsets(Style::ParseEdgeInsets(clipInsets));
		}
		
		TextField::~TextField()
		{
			_background->Release();
			_editor->Release();
		}
		
		TextField *TextField::WithType(Type type)
		{
			Dictionary *dictionary = nullptr;
			Style *styleSheet = Style::GetSharedInstance();
			
			switch(type)
			{
				case Type::RoundedRect:
					dictionary = styleSheet->TextfieldStyle(RNCSTR("RNRoundedRect"));
					break;
					
				case Type::Bezel:
					dictionary = styleSheet->TextfieldStyle(RNCSTR("RNBezel"));
					break;
			}
			
			TextField *textField = new TextField(dictionary);
			return textField->Autorelease();
		}
		
		
		
		void TextField::Initialize()
		{
			_background = new ImageView();
			_editor = new TextEditor();
			_editor->SetDelegate(this);
			_editor->GetTypesetter()->SetLineBreak(LineBreakMode::None);
			
			_formatter = nullptr;
			_delegate = nullptr;
			
			AddSubview(_background);
			
			_background->AddSubview(_editor);
			_background->SetClipSubviews(true);
		}
		
		
		
		void TextField::SetDelegate(TextFieldDelegate *delegate)
		{
			_delegate = delegate;
		}
		
		void TextField::SetText(String *text)
		{
			_editor->SetText(text);
		}
		
		void TextField::SetAttributedText(AttributedString *string)
		{
			_editor->SetAttributedText(string);
		}
		
		void TextField::SetValue(Object *object)
		{
			RN_ASSERT(_formatter, "SetValue() requires a formatter to be set!");
			
			AttributedString *temp = _formatter->GetAttributedStringForObject(object, _editor->GetTypingAttributes());
			SetAttributedText(temp);
		}
		
		void TextField::SetFormatter(Formatter *formatter)
		{
			if(_formatter)
				_formatter->Release();
			
			_formatter = formatter->Retain();
		}
		
		Object *TextField::GetValue() const
		{
			RN_ASSERT(_formatter, "Getvalue() requires a formatter to be set!");
			return _formatter->GetObjectForString(_editor->GetText());
		}
		
		
		void TextField::SetValueForUndefinedKey(const std::string& key, Object *value)
		{
			if(_formatter && key == "value")
			{
				SetValue(value);
				return;
			}
			
			Control::SetValueForUndefinedKey(key, value);
		}
		
		Object *TextField::GetValueForUndefinedKey(const std::string& key)
		{
			if(_formatter && key == "value")
			{
				return GetValue();
			}
			
			return Control::GetValueForUndefinedKey(key);
		}
		
		
		
		bool TextField::TextEditorShouldReturn(TextEditor *editor)
		{
			if(_delegate && !_delegate->TextFieldShouldEndEditing(this))
				return false;
			
			try
			{
				Object *temp = _formatter->GetObjectForString(_editor->GetText());
				if(temp)
				{
					SetValue(temp);
					return true;
				}
			}
			catch(Exception e)
			{}
			
			return false;
		}
		
		bool TextField::CanBecomeFirstResponder()
		{
			if(_delegate)
				return _delegate->TextFieldShouldBeginEditing(this);
				
			return true;
		}
		
		void TextField::BecomeFirstResponder()
		{
			Control::BecomeFirstResponder();
			
			if(_delegate)
				_delegate->TextFieldDidBeginEditing(this);
		}
		
		void TextField::ResignFirstResponder()
		{
			Control::ResignFirstResponder();
			
			if(_delegate)
				_delegate->TextFieldDidEndEditing(this);
		}
		
		
		
		void TextField::KeyDown(Event *event)
		{
			_editor->ProcessEvent(event);
		}
		
		void TextField::KeyRepeat(Event *event)
		{
			_editor->ProcessEvent(event);
		}
		
		void TextField::LayoutSubviews()
		{
			Control::LayoutSubviews();
			
			Rect frame = Frame();
			Rect editorRect = Rect(Vector2(_contentInsets.left, _contentInsets.top), _editor->GetTypesetter()->Dimensions());

			_background->SetFrame(Rect(0.0f, 0.0f, frame.width, frame.height));
			_editor->SetFrame(editorRect);
		}
	}
}
