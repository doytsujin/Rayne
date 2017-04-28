//
//  RNOculusSwapChain.cpp
//  Rayne-Oculus
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNOculusSwapChain.h"

namespace RN
{
	RNDefineMeta(OculusSwapChain, D3D12SwapChain)

	OculusSwapChain::OculusSwapChain() : _submitResult(0)
	{
		_session = nullptr;

		ovrResult result = ovr_Initialize(nullptr);
		if(OVR_FAILURE(result))
			return;

		
		result = ovr_Create(&_session, &_luID);
		if(OVR_FAILURE(result))
		{
			_session = nullptr;
			RNInfo(GetHMDInfoDescription());
			ovr_Shutdown();
			return;
		}

		_hmdDescription = ovr_GetHmdDesc(_session);
		RNInfo(GetHMDInfoDescription());

		// Configure Stereo settings.
		ovrSizei recommenedTex0Size = ovr_GetFovTextureSize(_session, ovrEye_Left, _hmdDescription.DefaultEyeFov[0], 2.0f);	//TODO: Set last parameter back to 1
		ovrSizei recommenedTex1Size = ovr_GetFovTextureSize(_session, ovrEye_Right, _hmdDescription.DefaultEyeFov[1], 2.0f); //TODO: Set last parameter back to 1
		ovrSizei bufferSize;
		bufferSize.w = recommenedTex0Size.w + recommenedTex1Size.w;
		bufferSize.h = std::max(recommenedTex0Size.h, recommenedTex1Size.h);
		_size.x = bufferSize.w;
		_size.y = bufferSize.h;

		ovrTextureSwapChainDesc swapChainDesc = {};
		swapChainDesc.Type = ovrTexture_2D;
		swapChainDesc.ArraySize = 1;
		swapChainDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		swapChainDesc.Width = bufferSize.w;
		swapChainDesc.Height = bufferSize.h;
		swapChainDesc.MipLevels = 1;
		swapChainDesc.SampleCount = 1;
		swapChainDesc.MiscFlags = ovrTextureMisc_DX_Typeless;
		swapChainDesc.StaticImage = ovrFalse;
		swapChainDesc.BindFlags = ovrTextureBind_DX_RenderTarget;

		_renderer = Renderer::GetActiveRenderer()->Downcast<D3D12Renderer>();

		result = ovr_CreateTextureSwapChainDX(_session, _renderer->GetCommandQueue(), &swapChainDesc, &_textureSwapChain);
		if(!OVR_SUCCESS(result))
			return;
		int textureCount = 0;
		ovr_GetTextureSwapChainLength(_session, _textureSwapChain, &textureCount);
		_bufferCount = textureCount;

		_framebuffer = new D3D12Framebuffer(_size, this, _renderer, Texture::Format::RGBA8888SRGB, Texture::Format::Depth24Stencil8);

		// Initialize VR structures, filling out description.
		_eyeRenderDesc[0] = ovr_GetRenderDesc(_session, ovrEye_Left, _hmdDescription.DefaultEyeFov[0]);
		_eyeRenderDesc[1] = ovr_GetRenderDesc(_session, ovrEye_Right, _hmdDescription.DefaultEyeFov[1]);
		_hmdToEyeViewOffset[0] = _eyeRenderDesc[0].HmdToEyeOffset;
		_hmdToEyeViewOffset[1] = _eyeRenderDesc[1].HmdToEyeOffset;

		// Initialize our single full screen Fov layer.
		_layer.Header.Type = ovrLayerType_EyeFov;
		_layer.Header.Flags = 0;
		_layer.ColorTexture[0] = _textureSwapChain;
		_layer.ColorTexture[1] = _textureSwapChain;
		_layer.Fov[0] = _eyeRenderDesc[0].Fov;
		_layer.Fov[1] = _eyeRenderDesc[1].Fov;
		_layer.Viewport[0].Pos.x = 0;
		_layer.Viewport[0].Pos.y = 0;
		_layer.Viewport[0].Size.w = bufferSize.w / 2;
		_layer.Viewport[0].Size.h = bufferSize.h;
		_layer.Viewport[1].Pos.x = bufferSize.w / 2;
		_layer.Viewport[1].Pos.y = 0;
		_layer.Viewport[1].Size.w = bufferSize.w / 2;
		_layer.Viewport[1].Size.h = bufferSize.h;

		ovr_SetTrackingOriginType(_session, ovrTrackingOrigin_FloorLevel);
	}

	OculusSwapChain::~OculusSwapChain()
	{
		ovr_Destroy(_session);
		ovr_Shutdown();
	}

	const String *OculusSwapChain::GetHMDInfoDescription() const
	{
		if(!_session)
			return RNCSTR("No HMD found.");

		String *description = new String("Using HMD: ");
		description->Append(_hmdDescription.ProductName);
		description->Append(", Vendor: ");
		description->Append(_hmdDescription.Manufacturer);
		description->Append(", Firmware: %i.%i", _hmdDescription.FirmwareMajor, _hmdDescription.FirmwareMinor);

		return description;
	}


	void OculusSwapChain::AcquireBackBuffer()
	{
		// Get next available index of the texture swap chain
		int currentIndex = 0;
		ovr_GetTextureSwapChainCurrentIndex(_session, _textureSwapChain, &currentIndex);
		_frameIndex = currentIndex;
	}

	void OculusSwapChain::Prepare(D3D12CommandList *commandList)
	{

	}

	void OculusSwapChain::Finalize(D3D12CommandList *commandList)
	{

	}

	void OculusSwapChain::PresentBackBuffer()
	{
		// Commit the changes to the texture swap chain
		ovr_CommitTextureSwapChain(_session, _textureSwapChain);

		// Submit frame with one layer we have.
		ovrLayerHeader* layers = &_layer.Header;
		_submitResult = ovr_SubmitFrame(_session, 0, nullptr, &layers, 1);	//TODO: Frameindex as second param
		//isVisible = (result == ovrSuccess);	//TODO: Pause application and rendering if not true
	}

	ID3D12Resource *OculusSwapChain::GetD3D12Buffer(int i) const
	{
		ID3D12Resource *buffer;
		ovr_GetTextureSwapChainBufferDX(_session, _textureSwapChain, i, IID_PPV_ARGS(&buffer));
		return buffer;
	}

	void OculusSwapChain::UpdatePredictedPose()
	{
		double displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, 0);	//TODO: Frameindex as second param
		_hmdState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);
		ovr_CalcEyePoses(_hmdState.HeadPose.ThePose, _hmdToEyeViewOffset, _layer.RenderPose);
	}
}
