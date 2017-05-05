//
//  RND3D12Renderer.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RND3D12Renderer.h"
#include "RND3D12ShaderLibrary.h"
#include "RND3D12GPUBuffer.h"
#include "RND3D12Texture.h"
#include "RND3D12UniformBuffer.h"
#include "RND3D12Device.h"
#include "RND3D12Framebuffer.h"
#include "RND3D12Internals.h"
#include "RND3D12SwapChain.h"

namespace RN
{
	RNDefineMeta(D3D12Renderer, Renderer)

	D3D12Renderer::D3D12Renderer(D3D12RendererDescriptor *descriptor, D3D12Device *device) :
		Renderer(descriptor, device),
		_mainWindow(nullptr),
		_mipMapTextures(new Array()),
		_currentRootSignature(nullptr),
		_currentSrvCbvHeap(nullptr),
		_submittedCommandLists(new Array()),
		_executedCommandLists(new Array()),
		_commandListPool(new Array()),
		_scheduledFenceValue(0),
		_completedFenceValue(0)
	{
		ID3D12Device *underlyingDevice = device->GetDevice();

		underlyingDevice->CreateFence(_scheduledFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

		D3D12_COMMAND_QUEUE_DESC queueDescriptor = {};
		queueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDescriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		underlyingDevice->CreateCommandQueue(&queueDescriptor, IID_PPV_ARGS(&_commandQueue));

		_rtvDescriptorSize = underlyingDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_srvCbvDescriptorSize = underlyingDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		_defaultShaderLibrary = CreateShaderLibraryWithFile(RNCSTR(":RayneD3D12:/Shaders.json"));
	}

	D3D12Renderer::~D3D12Renderer()
	{
		
	}

	D3D12CommandList *D3D12Renderer::GetCommandList()
	{
		D3D12CommandList *commandList = nullptr;
		if(_commandListPool->GetCount() == 0)
		{
			commandList = new D3D12CommandList(GetD3D12Device()->GetDevice());
		}
		else
		{
			commandList = _commandListPool->GetLastObject<D3D12CommandList>();
			commandList->Retain();
			_commandListPool->RemoveObjectAtIndex(_commandListPool->GetCount() - 1);
			commandList->Begin();
		}
		
		return commandList->Autorelease();
	}

	void D3D12Renderer::SubmitCommandList(D3D12CommandList *commandList)
	{
		_lock.Lock();
		_submittedCommandLists->AddObject(commandList);
		_lock.Unlock();
	}

	Window *D3D12Renderer::CreateAWindow(const Vector2 &size, Screen *screen)
	{
		D3D12Window *window = new D3D12Window(size, screen, this, 4);

		if(!_mainWindow)
			_mainWindow = window;

		return window;
	}

	Window *D3D12Renderer::GetMainWindow()
	{
		return _mainWindow;
	}


	void D3D12Renderer::CreateMipMapsForTexture(D3D12Texture *texture)
	{
		_lock.Lock();
		_mipMapTextures->AddObject(texture);
		_lock.Unlock();
	}

	void D3D12Renderer::CreateMipMaps()
	{
		if(_mipMapTextures->GetCount() == 0)
			return;

		//Calculate heap size
		uint32 requiredHeapSize = 0;
		bool waitingForUploads = false;
		_mipMapTextures->Enumerate<D3D12Texture>([&](D3D12Texture *texture, size_t index, bool &stop) {
			if(texture->GetDescriptor().mipMaps > 1)
				requiredHeapSize += texture->GetDescriptor().mipMaps - 1;

			if(!texture->_isReady)
			{
				waitingForUploads = true;
				stop = true;
			}
		});

		if(waitingForUploads)
			return;

		//No heap size, means that there was either no texture or none that requires any mipmaps
		if(requiredHeapSize == 0)
		{
			_mipMapTextures->RemoveAllObjects();
			return;
		}

		//Union used for shader constants
		struct DWParam
		{
			DWParam(FLOAT f) : Float(f) {}
			DWParam(UINT u) : Uint(u) {}

			void operator= (FLOAT f) { Float = f; }
			void operator= (UINT u) { Uint = u; }

			union
			{
				FLOAT Float;
				UINT Uint;
			};
		};

		//The compute shader expects 2 floats, the source texture and the destination texture
		CD3DX12_DESCRIPTOR_RANGE srvCbvRanges[2];
		CD3DX12_ROOT_PARAMETER rootParameters[3];
		srvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		srvCbvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
		rootParameters[0].InitAsConstants(3, 0);
		rootParameters[1].InitAsDescriptorTable(1, &srvCbvRanges[0]);
		rootParameters[2].InitAsDescriptorTable(1, &srvCbvRanges[1]);

		//Static sampler used to get the linearly interpolated color for the mipmaps
		D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		samplerDesc.ShaderRegister = 0;
		samplerDesc.RegisterSpace = 0;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		ID3D12Device *device = GetD3D12Device()->GetDevice();

		//Create the root signature for the mipmap compute shader from the parameters and sampler above
		ID3DBlob *signature;
		ID3DBlob *error;
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		ID3D12RootSignature *mipMapRootSignature;
		device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mipMapRootSignature));

		//Create the descriptor heap with layout: source texture - destination texture
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 2*requiredHeapSize;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ID3D12DescriptorHeap *descriptorHeap;
		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));
		UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		//Get the compute shader from the default shader library
		D3D12Shader *compute = _defaultShaderLibrary->GetShaderWithName(RNCSTR("GenerateMipMaps"))->Downcast<D3D12Shader>();
		ID3DBlob *computeShader = static_cast<ID3DBlob*>(compute->_shader);

		//Create pipeline state object for the compute shader using the root signature.
		D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = mipMapRootSignature;
		psoDesc.CS = { reinterpret_cast<UINT8*>(computeShader->GetBufferPointer()), computeShader->GetBufferSize() };
		ID3D12PipelineState *psoMipMaps;
		device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&psoMipMaps));

		//Prepare the shader resource view description for the source texture
		D3D12_SHADER_RESOURCE_VIEW_DESC srcTextureSRVDesc = {};
		srcTextureSRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srcTextureSRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		//Prepare the unordered access view description for the destination texture
		D3D12_UNORDERED_ACCESS_VIEW_DESC destTextureUAVDesc = {};
		destTextureUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		//Get a new empty command list in recording state
		D3D12CommandList *rnCommandList = GetCommandList();
		ID3D12GraphicsCommandList *commandList = rnCommandList->GetCommandList();

		//Set root signature, pso and descriptor heap
		commandList->SetComputeRootSignature(mipMapRootSignature);
		commandList->SetPipelineState(psoMipMaps);
		commandList->SetDescriptorHeaps(1, &descriptorHeap);

		//CPU handle for the first descriptor on the descriptor heap, used to fill the heap
		CD3DX12_CPU_DESCRIPTOR_HANDLE currentCPUHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, descriptorSize);

		//GPU handle for the first descriptor on the descriptor heap, used to initialize the descriptor tables
		CD3DX12_GPU_DESCRIPTOR_HANDLE currentGPUHandle(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), 0, descriptorSize);

		std::vector<IUnknown*> temporaryResources;

		_mipMapTextures->Enumerate<D3D12Texture>([&](D3D12Texture *texture, size_t index, bool &stop) {
			const Texture::Descriptor &textureDescriptor = texture->GetDescriptor();

			//Skip textures without mipmaps
			if(textureDescriptor.mipMaps <= 1)
				return;

			//TODO: Do this only for textures not supporting unordered access
			//Create temporary texture resource supporting unordered access (SRGB textures do not!)
			D3D12_RESOURCE_DESC tempTextureDescriptor = {};
			tempTextureDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	//TODO: Support other texture dimensions
			tempTextureDescriptor.Alignment = 0;
			tempTextureDescriptor.Width = textureDescriptor.width;
			tempTextureDescriptor.Height = textureDescriptor.height;
			tempTextureDescriptor.DepthOrArraySize = textureDescriptor.depth;
			tempTextureDescriptor.MipLevels = textureDescriptor.mipMaps;
			tempTextureDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//TODO: Base the format on the original texture
			tempTextureDescriptor.SampleDesc.Count = 1;
			tempTextureDescriptor.SampleDesc.Quality = 0;
			tempTextureDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			tempTextureDescriptor.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			ID3D12Resource *textureResource;
			device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &tempTextureDescriptor, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&textureResource));
			temporaryResources.push_back(textureResource);

			//Transition from pixel shader resource to copy source
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->_resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));

			//Copy into the temporary resource
			commandList->CopyResource(textureResource, texture->_resource);

			//Transition temporary resource from copy dest to unordered access
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

			//Loop through the mipmaps copying from the bigger mipmap to the smaller one with downsampling in a compute shader
			for(uint32_t TopMip = 0; TopMip < textureDescriptor.mipMaps-1; TopMip++)
			{
				//Get mipmap dimensions
				uint32_t dstWidth = std::max(textureDescriptor.width >> (TopMip+1), 1u);
				uint32_t dstHeight = std::max(textureDescriptor.height >> (TopMip+1), 1u);

				//Create shader resource view for the source texture in the descriptor heap
				srcTextureSRVDesc.Format = tempTextureDescriptor.Format;
				srcTextureSRVDesc.Texture2D.MipLevels = 1;
				srcTextureSRVDesc.Texture2D.MostDetailedMip = TopMip;
				device->CreateShaderResourceView(textureResource, &srcTextureSRVDesc, currentCPUHandle);
				currentCPUHandle.Offset(1, descriptorSize);

				//Create unordered access view for the destination texture in the descriptor heap
				destTextureUAVDesc.Format = tempTextureDescriptor.Format;
				destTextureUAVDesc.Texture2D.MipSlice = TopMip+1;
				device->CreateUnorderedAccessView(textureResource, nullptr, &destTextureUAVDesc, currentCPUHandle);
				currentCPUHandle.Offset(1, descriptorSize);

				//Pass the destination texture pixel size to the shader as constants
				commandList->SetComputeRoot32BitConstant(0, DWParam(1.0f/dstWidth).Uint, 0);
				commandList->SetComputeRoot32BitConstant(0, DWParam(1.0f/dstHeight).Uint, 1);
				commandList->SetComputeRoot32BitConstant(0, DWParam((textureDescriptor.format == Texture::Format::RGBA8888SRGB)?2.2f:1.0f).Uint, 2);	//TODO: Adjust gamma curve based on the original texture format

				//Pass the source and destination texture views to the shader via descriptor tables
				commandList->SetComputeRootDescriptorTable(1, currentGPUHandle);
				currentGPUHandle.Offset(1, descriptorSize);
				commandList->SetComputeRootDescriptorTable(2, currentGPUHandle);
				currentGPUHandle.Offset(1, descriptorSize);

				//Dispatch the compute shader with one thread per 8x8 pixels
				commandList->Dispatch(std::max(dstWidth / 8, 1u), std::max(dstHeight / 8, 1u), 1);

				//Wait for all accesses to the destination texture UAV to be finished before generating the next mipmap, as it will be the source texture for the next mipmap
				commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(textureResource));
			}

			//Copy temp resource back into the original one
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->_resource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
			commandList->CopyResource(texture->_resource, textureResource);

			//When done with the texture, transition it's state back to be a pixel shader resource
			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture->_resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
			texture->_needsMipMaps = false;
		});

		//Close and submit the command list
		rnCommandList->End();
		rnCommandList->SetFinishedCallback([temporaryResources, descriptorHeap, mipMapRootSignature, psoMipMaps](){
			//Cleanup
			for(IUnknown *resource : temporaryResources)
			{
				resource->Release();
			}

			descriptorHeap->Release();
			mipMapRootSignature->Release();
			psoMipMaps->Release();
		});
		SubmitCommandList(rnCommandList);

		_mipMapTextures->RemoveAllObjects();
	}


	void D3D12Renderer::Render(Function &&function)
	{
		_currentDrawableIndex = 0;
		_internals->renderPasses.clear();
		_internals->totalDrawableCount = 0;
		_internals->currentRenderPassIndex = 0;
		_internals->currentDrawableResourceIndex = 0;
		_internals->totalDescriptorTables = 0;
		_internals->swapChains.clear();
		_currentRootSignature = nullptr;
		_currentSrvCbvIndex = 0;

		_completedFenceValue = _fence->GetCompletedValue();

		//Delete command lists that finished execution on the graphics card (the command allocator needs to be alive the whole time)
		for(int i = _executedCommandLists->GetCount() - 1; i >= 0; i--)
		{
			if(_executedCommandLists->GetObjectAtIndex<D3D12CommandList>(i)->_fenceValue <= _completedFenceValue)
			{
				D3D12CommandList *commandList = _executedCommandLists->GetObjectAtIndex<D3D12CommandList>(i);
				commandList->Finish();
				_commandListPool->AddObject(commandList);
				_executedCommandLists->RemoveObjectAtIndex(i);
			}
		}

		//Free other frame resources such as descriptor heaps, that are not in use by the gpu anymore
		for(int i = _internals->frameResources.size()-1; i >= 0; i--)
		{
			D3D12FrameResource &frameResource = _internals->frameResources[i];
			if(frameResource.frame <= _completedFenceValue)
			{
				frameResource.resource->Release();
				_internals->frameResources.erase(_internals->frameResources.begin() + i);
			}
		}

		CreateMipMaps();		

		//SubmitCamera is called for each camera and creates lists of drawables per camera
		function();

		for(D3D12SwapChain *swapChain : _internals->swapChains)
		{
			swapChain->AcquireBackBuffer();
		}

		CreateDescriptorHeap();

		_currentCommandList = GetCommandList();

		for(D3D12SwapChain *swapChain : _internals->swapChains)
		{
			swapChain->Prepare(_currentCommandList);
		}

		ID3D12GraphicsCommandList *commandList = _currentCommandList->GetCommandList();
		
		_internals->currentRenderPassIndex = 0;
		_internals->currentDrawableResourceIndex = 0;
		for(const D3D12RenderPass &renderPass : _internals->renderPasses)
		{
			SetupRendertargets(_currentCommandList, renderPass);

			if(renderPass.drawables.size() > 0)
			{
				//Draw drawables
				for(D3D12Drawable *drawable : renderPass.drawables)
				{
					//TODO: Sort drawables by camera and root signature
					if(drawable->_cameraSpecifics[_internals->currentDrawableResourceIndex].pipelineState->rootSignature != _currentRootSignature)
					{
						_currentRootSignature = drawable->_cameraSpecifics[_internals->currentDrawableResourceIndex].pipelineState->rootSignature;
						commandList->SetGraphicsRootSignature(_currentRootSignature->signature);

						// Set the one big descriptor heap for the whole frame
						ID3D12DescriptorHeap* srvCbvHeaps[] = { _currentSrvCbvHeap };
						commandList->SetDescriptorHeaps(_countof(srvCbvHeaps), srvCbvHeaps);
					}
					RenderDrawable(commandList, drawable);
				}

				_internals->currentDrawableResourceIndex += 1;
			}

			_internals->currentRenderPassIndex += 1;
		}

		for(D3D12SwapChain *swapChain : _internals->swapChains)
		{
			swapChain->Finalize(_currentCommandList);
		}
		
		_currentCommandList->End();
		SubmitCommandList(_currentCommandList);
		_currentCommandList = nullptr;

		// Execute all command lists
		std::vector<ID3D12CommandList*> commandLists;
		_submittedCommandLists->Enumerate<D3D12CommandList>([&](D3D12CommandList *list, size_t index, bool &stop) {
			commandLists.push_back(list->GetCommandList());
			list->_fenceValue = _scheduledFenceValue;
		});
		_executedCommandLists->AddObjectsFromArray(_submittedCommandLists);
		_submittedCommandLists->RemoveAllObjects();
		_commandQueue->ExecuteCommandLists(commandLists.size(), &commandLists[0]);

		_commandQueue->Signal(_fence, _scheduledFenceValue++);

		for(D3D12SwapChain *swapChain : _internals->swapChains)
		{
			swapChain->PresentBackBuffer();
		}
	}

	void D3D12Renderer::SubmitCamera(Camera *camera, Function &&function)
	{
		D3D12RenderPass renderPass;
		renderPass.renderPass = camera->GetRenderPass();
		renderPass.shaderHint = camera->GetShaderHint();
		renderPass.overrideMaterial = camera->GetMaterial();
		renderPass.drawables.resize(0);

		renderPass.viewPosition = camera->GetWorldPosition();
		renderPass.viewMatrix = camera->GetViewMatrix();
		renderPass.inverseViewMatrix = camera->GetInverseViewMatrix();

		renderPass.projectionMatrix = camera->GetProjectionMatrix();
		//renderPass.projectionMatrix.m[5] *= -1.0f;
		renderPass.inverseProjectionMatrix = camera->GetInverseProjectionMatrix();

		renderPass.projectionViewMatrix = renderPass.projectionMatrix * renderPass.viewMatrix;
		renderPass.directionalShadowDepthTexture = nullptr;

		//TODO: Always return a valid framebuffer
		Framebuffer *framebuffer = camera->GetRenderPass()->GetFramebuffer();
		D3D12SwapChain *newSwapChain = nullptr;
		if(!framebuffer)
		{
			D3D12Window *window = static_cast<D3D12Window *>(_mainWindow);
			newSwapChain = window->GetSwapChain();
			framebuffer = newSwapChain->GetFramebuffer();
		}
		else
		{
			newSwapChain = framebuffer->Downcast<D3D12Framebuffer>()->GetSwapChain();
		}

		renderPass.framebuffer = framebuffer->Downcast<D3D12Framebuffer>();
		
		if(newSwapChain)
		{
			bool notIncluded = true;
			for(D3D12SwapChain *swapChain : _internals->swapChains)
			{
				if(swapChain == newSwapChain)
				{
					notIncluded = false;
					break;
				}
			}
			if(notIncluded)
			{
				_internals->swapChains.push_back(newSwapChain);
			}
		}

		_internals->currentRenderPassIndex = _internals->renderPasses.size();
		_internals->renderPasses.push_back(renderPass);

		// Create drawables
		function();

		size_t numberOfDrawables = _internals->renderPasses[_internals->currentRenderPassIndex].drawables.size();
		_internals->totalDrawableCount += numberOfDrawables;

		if(numberOfDrawables > 0)
			_internals->currentDrawableResourceIndex += 1;
	}

	GPUBuffer *D3D12Renderer::CreateBufferWithLength(size_t length, GPUResource::UsageOptions usageOptions, GPUResource::AccessOptions accessOptions)
	{
		return new D3D12GPUBuffer(nullptr, length, usageOptions);
	}
	GPUBuffer *D3D12Renderer::CreateBufferWithBytes(const void *bytes, size_t length, GPUResource::UsageOptions usageOptions, GPUResource::AccessOptions accessOptions)
	{
		return new D3D12GPUBuffer(bytes, length, usageOptions);
	}

	ShaderLibrary *D3D12Renderer::CreateShaderLibraryWithFile(const String *file)
	{
		return new D3D12ShaderLibrary(file);
	}

	ShaderLibrary *D3D12Renderer::CreateShaderLibraryWithSource(const String *source)
	{
		D3D12ShaderLibrary *lib = new D3D12ShaderLibrary(nullptr);
		return lib;
	}

	Shader *D3D12Renderer::GetDefaultShader(Shader::Type type, Shader::Options *options, Shader::UsageHint usageHint)
	{
		Shader *shader = nullptr;
		if(type == Shader::Type::Vertex)
		{
			if(usageHint == Shader::UsageHint::Depth)
			{
				shader = _defaultShaderLibrary->GetShaderWithName(RNCSTR("depth_vertex"), options);
			}
			else
			{
				const String *skyDefine = options->GetDefines()->GetObjectForKey<const String>(RNCSTR("RN_SKY"));
				if(skyDefine && !skyDefine->IsEqual(RNCSTR("0")))	//Use a different shader for the sky
				{
					shader = _defaultShaderLibrary->GetShaderWithName(RNCSTR("sky_vertex"), options);
				}
				else
				{
					shader = _defaultShaderLibrary->GetShaderWithName(RNCSTR("gouraud_vertex"), options);
				}
			}
		}
		else if(type == Shader::Type::Fragment)
		{
			if(usageHint == Shader::UsageHint::Depth)
			{
				shader = _defaultShaderLibrary->GetShaderWithName(RNCSTR("depth_fragment"), options);
			}
			else
			{
				const String *skyDefine = options->GetDefines()->GetObjectForKey<const String>(RNCSTR("RN_SKY"));
				if(skyDefine && !skyDefine->IsEqual(RNCSTR("0")))	//Use a different shader for the sky
				{
					shader = _defaultShaderLibrary->GetShaderWithName(RNCSTR("sky_fragment"), options);
				}
				else
				{
					shader = _defaultShaderLibrary->GetShaderWithName(RNCSTR("gouraud_fragment"), options);
				}
			}
		}

		return shader;
	}

	bool D3D12Renderer::SupportsTextureFormat(const String *format) const
	{
		//TODO: Fix this
		return true;
	}

	bool D3D12Renderer::SupportsDrawMode(DrawMode mode) const
	{
		return true;
	}

	size_t D3D12Renderer::GetAlignmentForType(PrimitiveType type) const
	{
		// TODO!!!!
		switch(type)
		{
			case PrimitiveType::Uint8:
			case PrimitiveType::Int8:
				return 1;

			case PrimitiveType::Uint16:
			case PrimitiveType::Int16:
				return 2;

			case PrimitiveType::Uint32:
			case PrimitiveType::Int32:
			case PrimitiveType::Float:
				return 4;

			case PrimitiveType::Vector2:
				return 8;

			case PrimitiveType::Vector3:
			case PrimitiveType::Vector4:
			case PrimitiveType::Matrix:
			case PrimitiveType::Quaternion:
			case PrimitiveType::Color:
				return 16;

			default:
				return 1;
		}
	}

	size_t D3D12Renderer::GetSizeForType(PrimitiveType type) const
	{
		// TODO!!!
		switch(type)
		{
			case PrimitiveType::Uint8:
			case PrimitiveType::Int8:
				return 1;

			case PrimitiveType::Uint16:
			case PrimitiveType::Int16:
				return 2;

			case PrimitiveType::Uint32:
			case PrimitiveType::Int32:
			case PrimitiveType::Float:
				return 4;

			case PrimitiveType::Vector2:
				return 8;

			case PrimitiveType::Vector3:
			case PrimitiveType::Vector4:
			case PrimitiveType::Quaternion:
			case PrimitiveType::Color:
				return 16;

			case PrimitiveType::Matrix:
				return 64;

			default:
				return 1;
		}
	}

	Texture *D3D12Renderer::CreateTextureWithDescriptor(const Texture::Descriptor &descriptor)
	{
		return new D3D12Texture(descriptor, this);
	}

	Framebuffer *D3D12Renderer::CreateFramebuffer(const Vector2 &size)
	{
		return new D3D12Framebuffer(size, this);
	}

	Drawable *D3D12Renderer::CreateDrawable()
	{
		D3D12Drawable *newDrawable = new D3D12Drawable();
		return newDrawable;
	}

	void D3D12Renderer::DeleteDrawable(Drawable *drawable)
	{
		delete drawable;
	}

	void D3D12Renderer::SetupRendertargets(D3D12CommandList *commandList, const D3D12RenderPass &renderpass)
	{
		ID3D12Device *underlyingDevice = GetD3D12Device()->GetDevice();
		ID3D12GraphicsCommandList *d3dCommandList = commandList->GetCommandList();

		//TODO: Call PrepareAsRendertargetForFrame() only once per framebuffer per frame
		renderpass.framebuffer->PrepareAsRendertargetForFrame(_scheduledFenceValue);
		renderpass.framebuffer->SetAsRendertarget(commandList);

		//Setup viewport and scissor rect
		Rect cameraRect = renderpass.renderPass->GetFrame();

		D3D12_VIEWPORT viewport;
		viewport.Width = cameraRect.width;
		viewport.Height = cameraRect.height;
		viewport.TopLeftX = cameraRect.x;
		viewport.TopLeftY = cameraRect.y;
		viewport.MaxDepth = 1.0;
		viewport.MinDepth = 0.0;

		D3D12_RECT viewportRect;
		viewportRect.right = static_cast<LONG>(cameraRect.width + cameraRect.x);
		viewportRect.bottom = static_cast<LONG>(cameraRect.height + cameraRect.y);
		viewportRect.left = cameraRect.x;
		viewportRect.top = cameraRect.y;

		d3dCommandList->RSSetViewports(1, &viewport);
		d3dCommandList->RSSetScissorRects(1, &viewportRect);

		
		// Cameras always clear the whole framebuffer to be more consistent with the metal renderer
		if(renderpass.renderPass->GetFlags() & RenderPass::Flags::ClearColor)
		{
			renderpass.framebuffer->ClearColorTargets(commandList, renderpass.renderPass->GetClearColor());
		}
		
		if(renderpass.renderPass->GetFlags() & RenderPass::Flags::ClearDepthStencil)
		{
			renderpass.framebuffer->ClearDepthStencilTarget(commandList, renderpass.renderPass->GetClearDepth(), renderpass.renderPass->GetClearStencil());
		}
	}

	void D3D12Renderer::CreateDescriptorHeap()
	{
		if(_internals->totalDrawableCount == 0)
			return;

		ID3D12Device *device = GetD3D12Device()->GetDevice();

		ID3D12DescriptorHeap *srvCbvHeap;

		// Layout: textures + 1 cbv
		D3D12_DESCRIPTOR_HEAP_DESC srvCbvHeapDesc = {};
		srvCbvHeapDesc.NumDescriptors = _internals->totalDescriptorTables;
		srvCbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvCbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		device->CreateDescriptorHeap(&srvCbvHeapDesc, IID_PPV_ARGS(&srvCbvHeap));
		_currentSrvCbvHeap = srvCbvHeap;
		_internals->frameResources.push_back({ srvCbvHeap, _scheduledFenceValue });

		// Describe null SRVs. Null descriptors are used to "unbind" textures
		D3D12_SHADER_RESOURCE_VIEW_DESC nullSrvDesc = {};
		nullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		nullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		nullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		nullSrvDesc.Texture2D.MipLevels = 1;
		nullSrvDesc.Texture2D.MostDetailedMip = 0;
		nullSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		CD3DX12_CPU_DESCRIPTOR_HANDLE currentCPUHandle(srvCbvHeap->GetCPUDescriptorHandleForHeapStart(), 0, _srvCbvDescriptorSize);

		size_t cameraID = 0;
		for(const D3D12RenderPass &renderPass : _internals->renderPasses)
		{
			if(renderPass.drawables.size() == 0)
				continue;

			for(D3D12Drawable *drawable : renderPass.drawables)
			{
				const D3D12RootSignature *rootSignature = drawable->_cameraSpecifics[cameraID].pipelineState->rootSignature;

				//Create texture descriptors
				const Array *textures = drawable->material->GetTextures();
				textures->Enumerate<D3D12Texture>([&](D3D12Texture *texture, size_t i, bool &stop) {
					//Respect the textures limit of the root signature
					if(i >= (rootSignature->textureCount - rootSignature->wantsDirectionalShadowTexture))
					{
						stop = true;
						return;
					}

					//Check if texture finished uploading to the vram
					if(texture->_isReady && !texture->_needsMipMaps)
					{
						device->CreateShaderResourceView(texture->_resource, &texture->_srvDescriptor, currentCPUHandle);
					}
					else
					{
						device->CreateShaderResourceView(nullptr, &nullSrvDesc, currentCPUHandle);
					}

					currentCPUHandle.Offset(1, _srvCbvDescriptorSize);
				});

				//TODO: Find a cleaner more general solution
				if(rootSignature->wantsDirectionalShadowTexture && renderPass.directionalShadowDepthTexture)
				{
					device->CreateShaderResourceView(renderPass.directionalShadowDepthTexture->_resource, &renderPass.directionalShadowDepthTexture->_srvDescriptor, currentCPUHandle);
					currentCPUHandle.Offset(1, _srvCbvDescriptorSize);
				}

				//Create null texture descriptors for those that are too many in the root signature
				for(int i = rootSignature->textureCount - (textures->GetCount() + (rootSignature->wantsDirectionalShadowTexture && renderPass.directionalShadowDepthTexture)); i > 0; i--)
				{
					device->CreateShaderResourceView(nullptr, &nullSrvDesc, currentCPUHandle);
					currentCPUHandle.Offset(1, _srvCbvDescriptorSize);
				}

				//Create constant buffer descriptor
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};

				D3D12UniformBuffer *vertexUniformBuffer = drawable->_cameraSpecifics[cameraID].uniformState->vertexUniformBuffer;
				if (vertexUniformBuffer)
				{
					D3D12GPUBuffer *actualBuffer = vertexUniformBuffer->GetActiveBuffer()->Downcast<D3D12GPUBuffer>();
					cbvDesc.BufferLocation = actualBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
					cbvDesc.SizeInBytes = actualBuffer->GetLength();
					GetD3D12Device()->GetDevice()->CreateConstantBufferView(&cbvDesc, currentCPUHandle);
					currentCPUHandle.Offset(1, _srvCbvDescriptorSize);
				}

				D3D12UniformBuffer *fragmentUniformBuffer = drawable->_cameraSpecifics[cameraID].uniformState->fragmentUniformBuffer;
				if(fragmentUniformBuffer)
				{
					D3D12GPUBuffer *actualBuffer = fragmentUniformBuffer->GetActiveBuffer()->Downcast<D3D12GPUBuffer>();
					cbvDesc.BufferLocation = actualBuffer->GetD3D12Resource()->GetGPUVirtualAddress();
					cbvDesc.SizeInBytes = actualBuffer->GetLength();
					GetD3D12Device()->GetDevice()->CreateConstantBufferView(&cbvDesc, currentCPUHandle);
					currentCPUHandle.Offset(1, _srvCbvDescriptorSize);
				}
			}

			cameraID += 1;
		}
	}

	void D3D12Renderer::FillUniformBuffer(uint8 *buffer, D3D12Drawable *drawable, Shader *shader, size_t &offset)
	{
		Material *material = drawable->material;
		const D3D12RenderPass &renderPass = _internals->renderPasses[_internals->currentRenderPassIndex];

		buffer += offset;

		shader->GetSignature()->GetUniformDescriptors()->Enumerate<Shader::UniformDescriptor>([&](Shader::UniformDescriptor *descriptor, size_t index, bool &stop) {
			offset = descriptor->GetOffset() + descriptor->GetSize();

			switch(descriptor->GetIdentifier())
			{
				case Shader::UniformDescriptor::Identifier::Time:
				{
					float temp = static_cast<float>(Kernel::GetSharedInstance()->GetTotalTime());
					std::memcpy(buffer + descriptor->GetOffset(), &temp, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::ModelMatrix:
				{
					std::memcpy(buffer + descriptor->GetOffset(), drawable->modelMatrix.m, descriptor->GetSize());
					break;
				}
				
				case Shader::UniformDescriptor::Identifier::ModelViewMatrix:
				{
					Matrix result = renderPass.viewMatrix * drawable->modelMatrix;
					std::memcpy(buffer + descriptor->GetOffset(), result.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::ModelViewProjectionMatrix:
				{
					Matrix result = renderPass.projectionViewMatrix * drawable->modelMatrix;
					std::memcpy(buffer + descriptor->GetOffset(), result.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::ViewMatrix:
				{
					std::memcpy(buffer + descriptor->GetOffset(), renderPass.viewMatrix.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::ViewProjectionMatrix:
				{
					std::memcpy(buffer + descriptor->GetOffset(), renderPass.projectionViewMatrix.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::ProjectionMatrix:
				{
					std::memcpy(buffer + descriptor->GetOffset(), renderPass.projectionMatrix.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::InverseModelMatrix:
				{
					std::memcpy(buffer + descriptor->GetOffset(), drawable->inverseModelMatrix.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::InverseModelViewMatrix:
				{
					Matrix result = renderPass.inverseViewMatrix * drawable->inverseModelMatrix;
					std::memcpy(buffer + descriptor->GetOffset(), result.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::InverseModelViewProjectionMatrix:
				{
					Matrix result = renderPass.inverseProjectionViewMatrix * drawable->inverseModelMatrix;
					std::memcpy(buffer + descriptor->GetOffset(), result.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::InverseViewMatrix:
				{
					std::memcpy(buffer + descriptor->GetOffset(), renderPass.inverseViewMatrix.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::InverseViewProjectionMatrix:
				{
					std::memcpy(buffer + descriptor->GetOffset(), renderPass.inverseProjectionViewMatrix.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::InverseProjectionMatrix:
				{
					std::memcpy(buffer + descriptor->GetOffset(), renderPass.inverseProjectionMatrix.m, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::AmbientColor:
				{
					std::memcpy(buffer + descriptor->GetOffset(), &material->GetAmbientColor().r, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::DiffuseColor:
				{
					std::memcpy(buffer + descriptor->GetOffset(), &material->GetDiffuseColor().r, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::SpecularColor:
				{
					std::memcpy(buffer + descriptor->GetOffset(), &material->GetSpecularColor().r, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::EmissiveColor:
				{
					std::memcpy(buffer + descriptor->GetOffset(), &material->GetEmissiveColor().r, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::TextureTileFactor:
				{
					float temp = material->GetTextureTileFactor();
					std::memcpy(buffer + descriptor->GetOffset(), &temp, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::DiscardThreshold:
				{
					float temp = material->GetDiscardThreshold();
					std::memcpy(buffer + descriptor->GetOffset(), &temp, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::CameraPosition:
				{
					RN::Vector3 cameraPosition = renderPass.viewPosition;
					std::memcpy(buffer + descriptor->GetOffset(), &cameraPosition.x, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::DirectionalLightsCount:
				{
					size_t lightCount = renderPass.directionalLights.size();
					std::memcpy(buffer + descriptor->GetOffset(), &lightCount, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::DirectionalLights:
				{
					size_t lightCount = renderPass.directionalLights.size();
					if(lightCount > 0)
					{
						std::memcpy(buffer + descriptor->GetOffset(), &renderPass.directionalLights[0], (16 + 16) * lightCount);
					}
					break;
				}

				case Shader::UniformDescriptor::Identifier::DirectionalShadowMatricesCount:
				{
					size_t matrixCount = renderPass.directionalShadowMatrices.size();
					if(matrixCount > 0)
					{
						std::memcpy(buffer + descriptor->GetOffset(), &matrixCount, descriptor->GetSize());
					}
					break;
				}

				case Shader::UniformDescriptor::Identifier::DirectionalShadowMatrices:
				{
					size_t matrixCount = renderPass.directionalShadowMatrices.size();
					if (matrixCount > 0)
					{
						std::memcpy(buffer + descriptor->GetOffset(), &renderPass.directionalShadowMatrices[0].m[0], 64 * matrixCount);
					}
					break;
				}

				case Shader::UniformDescriptor::Identifier::DirectionalShadowInfo:
				{
					std::memcpy(buffer + descriptor->GetOffset(), &renderPass.directionalShadowInfo.x, descriptor->GetSize());
					break;
				}

				case Shader::UniformDescriptor::Identifier::Custom:
				{
					//TODO: Implement custom shader variables!
					break;
				}

				default:
					break;
			}
		});
	}

	void D3D12Renderer::SubmitLight(const Light *light)
	{
		_lock.Lock();
		D3D12RenderPass &renderPass = _internals->renderPasses[_internals->currentRenderPassIndex];
		_lock.Unlock();

		if(light->GetType() == Light::Type::DirectionalLight)
		{
			if(renderPass.directionalLights.size() < 5) //TODO: Don't hardcode light limit here
			{
				renderPass.directionalLights.push_back(D3D12LightDirectional{ light->GetForward(), 0.0f, light->GetColor() });
			}

			//TODO: Allow more lights with shadows or prevent multiple light with shadows overwriting each other
			if(light->HasShadows())
			{
				renderPass.directionalShadowDepthTexture = light->GetShadowDepthTexture()->Downcast<D3D12Texture>();
				renderPass.directionalShadowMatrices = light->GetShadowMatrices();
				renderPass.directionalShadowInfo = Vector2(1.0f / light->GetShadowParameters().resolution);
			}
		}
	}

	void D3D12Renderer::SubmitDrawable(Drawable *tdrawable)
	{
		D3D12Drawable *drawable = static_cast<D3D12Drawable *>(tdrawable);
		drawable->AddUniformStateIfNeeded(_internals->currentDrawableResourceIndex);

		_lock.Lock();
		D3D12RenderPass &renderPass = _internals->renderPasses[_internals->currentRenderPassIndex];
		_lock.Unlock();

		Material *material = drawable->material;
		if(drawable->dirty)
		{
			//TODO: Fix the camera situation...
			_lock.Lock();
			const D3D12PipelineState *pipelineState = _internals->stateCoordinator.GetRenderPipelineState(material, drawable->mesh, renderPass.framebuffer, renderPass.shaderHint, renderPass.overrideMaterial);
			D3D12UniformState *uniformState = _internals->stateCoordinator.GetUniformStateForPipelineState(pipelineState, material);
			_lock.Unlock();

			drawable->UpdateRenderingState(_internals->currentDrawableResourceIndex, pipelineState, uniformState);
			drawable->dirty = false;
		}

		const D3D12PipelineState *pipelineState = drawable->_cameraSpecifics[_internals->currentDrawableResourceIndex].pipelineState;
		D3D12UniformState *uniformState = drawable->_cameraSpecifics[_internals->currentDrawableResourceIndex].uniformState;
		if(uniformState->vertexUniformBuffer)
		{
			GPUBuffer *gpuBuffer = uniformState->vertexUniformBuffer->Advance();
			uint8 *buffer = reinterpret_cast<uint8 *>(gpuBuffer->GetBuffer());
			size_t offset = 0;
			FillUniformBuffer(buffer, drawable, pipelineState->descriptor.vertexShader, offset);
			gpuBuffer->Invalidate();
		}
		if (uniformState->fragmentUniformBuffer)
		{
			GPUBuffer *gpuBuffer = uniformState->fragmentUniformBuffer->Advance();
			uint8 *buffer = reinterpret_cast<uint8 *>(gpuBuffer->GetBuffer());
			size_t offset = 0;
			FillUniformBuffer(buffer, drawable, pipelineState->descriptor.fragmentShader, offset);
			gpuBuffer->Invalidate();
		}

		// Push into the queue
		_lock.Lock();
		renderPass.drawables.push_back(drawable);
		_internals->totalDescriptorTables += drawable->_cameraSpecifics[_internals->currentDrawableResourceIndex].pipelineState->rootSignature->textureCount;
		_internals->totalDescriptorTables += drawable->_cameraSpecifics[_internals->currentDrawableResourceIndex].pipelineState->rootSignature->constantBufferCount;
		_lock.Unlock();
	}

	void D3D12Renderer::RenderDrawable(ID3D12GraphicsCommandList *commandList, D3D12Drawable *drawable)
	{
		D3D12SwapChain *swapChain = _mainWindow->GetSwapChain();
		const D3D12RootSignature *rootSignature = drawable->_cameraSpecifics[_internals->currentDrawableResourceIndex].pipelineState->rootSignature;

		UINT rootParameter = 0;
		if(rootSignature->textureCount)
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE srvGPUHandle(_currentSrvCbvHeap->GetGPUDescriptorHandleForHeapStart(), _currentSrvCbvIndex, _srvCbvDescriptorSize);
			commandList->SetGraphicsRootDescriptorTable(rootParameter++, srvGPUHandle);
			_currentSrvCbvIndex += rootSignature->textureCount;
		}

		if(rootSignature->constantBufferCount)
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE cbvGPUHandle(_currentSrvCbvHeap->GetGPUDescriptorHandleForHeapStart(), _currentSrvCbvIndex, _srvCbvDescriptorSize);
			commandList->SetGraphicsRootDescriptorTable(rootParameter++, cbvGPUHandle);
			_currentSrvCbvIndex += rootSignature->constantBufferCount;
		}

		commandList->SetPipelineState(drawable->_cameraSpecifics[_internals->currentDrawableResourceIndex].pipelineState->state);

		D3D12GPUBuffer *buffer = static_cast<D3D12GPUBuffer *>(drawable->mesh->GetVertexBuffer());
		D3D12GPUBuffer *indices = static_cast<D3D12GPUBuffer *>(drawable->mesh->GetIndicesBuffer());

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
		vertexBufferView.BufferLocation = buffer->GetD3D12Resource()->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = drawable->mesh->GetStride();
		vertexBufferView.SizeInBytes = buffer->GetLength();
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

		D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
		indexBufferView.BufferLocation = indices->GetD3D12Resource()->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		indexBufferView.SizeInBytes = indices->GetLength();
		commandList->IASetIndexBuffer(&indexBufferView);

		commandList->DrawIndexedInstanced(drawable->mesh->GetIndicesCount(), 1, 0, 0, 0);

		_currentDrawableIndex += 1;
	}

	void D3D12Renderer::AddFrameResouce(IUnknown *resource, uint32 frame)
	{
		_internals->frameResources.push_back({ resource, frame });
	}
}
