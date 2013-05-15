//
//  RNUIServer.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNUIServer.h"
#include "RNWorld.h"
#include "RNWindow.h"

namespace RN
{
	UIServer::UIServer()
	{
		uint32 flags = Camera::FlagOrthogonal | Camera::FlagUpdateAspect | Camera::FlagUpdateStorageFrame | Camera::FlagNoSorting;
		_camera = new Camera(Vector2(0.0f), TextureParameter::Format::RGBA8888, flags, RenderStorage::BufferFormatColor);
		_camera->SetClearColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
		_camera->SetAllowsDepthWrite(false);
		_camera->SetUseBlending(true);
		
		_camera->clipnear = -500.0f;
		
		if(_camera->Container())
			_camera->Container()->RemoveSceneNode(_camera);
	}
	
	UIServer::~UIServer()
	{
		_camera->Release();
	}
	
	uint32 UIServer::Height() const
	{
		return _frame.height;
	}
	
	uint32 UIServer::Width() const
	{
		return _frame.width;
	}
	
	void UIServer::AddWidget(Widget *widget)
	{
		RN_ASSERT0(widget->_server == 0);
		
		_widgets.push_front(widget);
		widget->_server = this;
		widget->Retain();
	}
	
	void UIServer::RemoveWidget(Widget *widget)
	{
		RN_ASSERT0(widget->_server == this);
		
		_widgets.erase(std::remove(_widgets.begin(), _widgets.end(), widget), _widgets.end());
		widget->_server = 0;
		widget->Release();
	}
	
	void UIServer::MoveWidgetToFront(Widget *widget)
	{
		RN_ASSERT0(widget->_server == this);
		
		_widgets.erase(std::remove(_widgets.begin(), _widgets.end(), widget), _widgets.end());
		_widgets.push_front(widget);
	}
	
	
	void UIServer::Render(Renderer *renderer)
	{
		if(_widgets.size() == 0)
			return;
		
		Rect actualFrame = Window::SharedInstance()->Frame();
		if(_frame != actualFrame)
		{
			_frame = actualFrame;
				
			_camera->ortholeft   = _frame.Left();
			_camera->orthobottom = _frame.Bottom();
			_camera->orthoright  = _frame.Right();
			_camera->orthotop    = _frame.Top();
			
			_camera->SetFrame(_frame);
			_camera->PostUpdate();
		}
		
		// Draw all widgets into the camera
		renderer->BeginCamera(_camera);
		
		for(Widget *widget : _widgets)
		{
			widget->Render(renderer);
		}
		
		renderer->FinishCamera();
	}
}
