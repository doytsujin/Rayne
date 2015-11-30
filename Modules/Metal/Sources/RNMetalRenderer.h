//
//  RNMetalRenderer.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_METALRENDERER_H__
#define __RAYNE_METALRENDERER_H__

#include <Rayne.h>
#include "RNMetalWindow.h"

namespace RN
{
	struct MetalRendererInternals;
	struct MetalDrawable;
	class MetalWindow;
	class MetalTexture;
	class GPUBuffer;

	class MetalRenderer : public Renderer
	{
	public:
		friend class MetalTexture;
		friend class MetalWindow;

		RNAPI MetalRenderer(const Dictionary *parameters);
		RNAPI ~MetalRenderer();

		RNAPI Window *CreateAWindow(const Vector2 &size, Screen *screen) final;
		RNAPI Window *GetMainWindow() final;

		RNAPI void RenderIntoWindow(Window *window, Function &&function) final;
		RNAPI void RenderIntoCamera(Camera *camera, Function &&function) final;

		RNAPI bool SupportsTextureFormat(const String *format) const final;
		RNAPI bool SupportsDrawMode(DrawMode mode) const final;

		RNAPI const String *GetTextureFormatName(const Texture::Format format) const final;
		RNAPI size_t GetAlignmentForType(PrimitiveType type) const final;
		RNAPI size_t GetSizeForType(PrimitiveType type) const final;

		RNAPI GPUBuffer *CreateBufferWithLength(size_t length, GPUResource::UsageOptions options) final;
		RNAPI GPUBuffer *CreateBufferWithBytes(const void *bytes, size_t length, GPUResource::UsageOptions options) final;

		RNAPI ShaderLibrary *GetShaderLibraryWithFile(const String *file) final;
		RNAPI ShaderLibrary *GetShaderLibraryWithSource(const String *source) final;

		RNAPI Texture *CreateTextureWithDescriptor(const Texture::Descriptor &descriptor) final;

		RNAPI Drawable *CreateDrawable() final;
		RNAPI void SubmitDrawable(Drawable *drawable) final;

	protected:
		void RenderDrawable(MetalDrawable *drawable);

		PIMPL<MetalRendererInternals> _internals;
		MetalWindow *_mainWindow;

		SpinLock _lock;

		void CreateMipMapForeTexture(MetalTexture *texture);
		void CreateMipMaps();

		Set *_mipMapTextures;
		Dictionary *_textureFormatLookup;

		RNDeclareMeta(MetalRenderer)
	};
}


#endif /* __RAYNE_METALRENDERER_H__ */
