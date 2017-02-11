//
//  RND3D12UniformBuffer.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_D3D12UNIFORMBUFFER_H_
#define __RAYNE_D3D12UNIFORMBUFFER_H_

#include "RND3D12.h"
#include "RND3D12StateCoordinator.h"

#define kRND3D12UniformBufferCount 3

namespace RN
{
	class Renderer;
	class GPUBuffer;
	struct D3D12RenderingStateUniformBufferArgument;

	class D3D12UniformBuffer : public Object
	{
	public:
		RN_OPTIONS(Feature, uint32,
				   Custom = 0,
				   Time = (1 << 0),
				   ModelMatrix = (1 << 1),
				   ModelViewMatrix = (1 << 2),
				   ModelViewProjectionMatrix = (1 << 3),
				   ViewMatrix = (1 << 4),
				   ViewProjectionMatrix = (1 << 5),
				   ProjectionMatrix = (1 << 6),
				   InverseModelMatrix = (1 << 7),
				   InverseModelViewMatrix = (1 << 8),
				   InverseModelViewProjectionMatrix = (1 << 9),
				   InverseViewMatrix = (1 << 10),
				   InverseViewProjectionMatrix = (1 << 11),
				   InverseProjectionMatrix = (1 << 12),
				   AmbientColor = (1 << 13),
				   DiffuseColor = (1 << 14),
				   SpecularColor = (1 << 15),
				   EmissiveColor = (1 << 16),
			       DiscardThreshold = (1 << 17));

		class Member
		{
		public:
			Member(const String *featureString, size_t offset) :
				_featureString(featureString->Copy()),
				_feature(Feature::Custom),
				_offset(offset)
			{}
			Member(Feature feature, size_t offset) :
				_featureString(nullptr),
				_feature(feature),
				_offset(offset)
			{}

			~Member()
			{
				SafeRelease(_featureString);
			}

			Member(const Member &other) :
				_feature(other._feature),
				_featureString(SafeRetain(other._featureString)),
				_offset(other._offset)
			{}

			Member &operator =(const Member &other)
			{
				SafeRelease(_featureString);

				_featureString = SafeRetain(other._featureString);
				_feature = other._feature;
				_offset = other._offset;

				return *this;
			}

			Feature GetFeature() const { return _feature; }
			size_t GetOffset() const { return _offset; }

		private:
			String *_featureString;
			Feature _feature;
			size_t _offset;
		};

		D3D12UniformBuffer(Renderer *renderer, D3D12RenderingStateUniformBufferArgument *uniformBuffer);
		~D3D12UniformBuffer();

		GPUBuffer *Advance();
		GPUBuffer *GetActiveBuffer() const { return _buffers[_bufferIndex]; }

		const Member *GetMemberForFeature(Feature feature) const;

		size_t GetIndex() const { return _index; }

	private:
		size_t _index;
		GPUBuffer *_buffers[kRND3D12UniformBufferCount];
		size_t _bufferIndex;
		Feature _supportedFeatures;
		std::vector<Member> _members;

		RNDeclareMetaAPI(D3D12UniformBuffer, D3DAPI)
	};
}

#endif /* __RAYNE_D3D12UNIFORMBUFFER_H_ */