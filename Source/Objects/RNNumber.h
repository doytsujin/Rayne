//
//  RNNumber.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_NUMBER_H__
#define __RAYNE_NUMBER_H__

#include "../Base/RNBase.h"
#include "RNObject.h"
#include "RNSerialization.h"

namespace RN
{
	class Number : public Object
	{
	public:
		enum class Type
		{
			Int8,
			Int16,
			Int32,
			Int64,
			Uint8,
			Uint16,
			Uint32,
			Uint64,
			Float32,
			Float64,
			Boolean
		};
		
		RNAPI Number(Deserializer *deserializer);
		RNAPI Number(const Number *number);
		RNAPI explicit Number(bool value);
		RNAPI explicit Number(float value);
		RNAPI explicit Number(double value);
		
		RNAPI explicit Number(int8 value);
		RNAPI explicit Number(int16 value);
		RNAPI explicit Number(int32 value);
		RNAPI explicit Number(int64 value);
		
		RNAPI explicit Number(uint8 value);
		RNAPI explicit Number(uint16 value);
		RNAPI explicit Number(uint32 value);
		RNAPI explicit Number(uint64 value);
		RNAPI ~Number() override;
		
		RNAPI static Number *WithBool(bool value);
		RNAPI static Number *WithFloat(float value);
		RNAPI static Number *WithDouble(double value);
		
		RNAPI static Number *WithInt8(int8 value);
		RNAPI static Number *WithInt16(int16 value);
		RNAPI static Number *WithInt32(int32 value);
		RNAPI static Number *WithInt64(int64 value);
		
		RNAPI static Number *WithUint8(uint8 value);
		RNAPI static Number *WithUint16(uint16 value);
		RNAPI static Number *WithUint32(uint32 value);
		RNAPI static Number *WithUint64(uint64 value);
		
		RNAPI void Serialize(Serializer *serializer) const override;
		
		RNAPI bool GetBoolValue() const;
		RNAPI float GetFloatValue() const;
		RNAPI double GetDoubleValue() const;
		
		RNAPI int8 GetInt8Value() const;
		RNAPI int16 GetInt16Value() const;
		RNAPI int32 GetInt32Value() const;
		RNAPI int64 GetInt64Value() const;
		
		RNAPI uint8 GetUint8Value() const;
		RNAPI uint16 GetUint16Value() const;
		RNAPI uint32 GetUint32Value() const;
		RNAPI uint64 GetUint64Value() const;
		
		RNAPI size_t GetHash() const override;
		RNAPI bool IsEqual(const Object *other) const override;
		
		Type GetType() const { return _type; }
		
	private:
		static size_t SizeForType(Type type);
		void CopyData(const void *data, size_t size, Type type);
		
		uint8 *_buffer;
		Type _type;
		
		__RNDeclareMetaInternal(Number);
	};
	
	RNObjectClass(Number)
}

#endif /* __RAYNE_NUMBER_H__ */
