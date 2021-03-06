//
//  RNDictionary.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_DICTIONARY_H__
#define __RAYNE_DICTIONARY_H__

#include "../Base/RNBase.h"
#include "RNObject.h"

namespace RN
{
	class Array;
	class DictionaryInternal;
	
	class Dictionary : public Object
	{
	public:
		RNAPI Dictionary();
		RNAPI Dictionary(size_t capacity);
		RNAPI Dictionary(const Dictionary *other);
		RNAPI Dictionary(Deserializer *deserializer);
		RNAPI ~Dictionary() override;
		
		RNAPI void Serialize(Serializer *serializer) const override;
		RNAPI const String *GetDescription() const override;
		
		RNAPI size_t GetHash() const override;
		RNAPI bool IsEqual(const Object *other) const override;
		
		template<typename T=Object>
		T *GetObjectForKey(const Object *key) const
		{
			Object *object = GetPrimitiveObjectForKey(key);
			if(object)
				return object->Downcast<T>();
			
			return nullptr;
		}
		
		RNAPI void AddEntriesFromDictionary(const Dictionary *other);
		RNAPI void SetObjectForKey(Object *object, const Object *key);
		RNAPI void RemoveObjectForKey(const Object *key);
		RNAPI void RemoveAllObjects();
		
		RNAPI void Enumerate(const std::function<void (Object *object, const Object *key, bool &stop)>& callback) const;
		
		template<class T, class K>
		void Enumerate(const std::function<void (T *object, const K *key, bool &stop)>& callback) const
		{
			Enumerate([&](Object *object, const Object *key, bool &stop) {
				callback(static_cast<T *>(object), static_cast<const K *>(key), stop);
			});
		}
		
		RNAPI Array *GetAllObjects() const;
		RNAPI Array *GetAllKeys() const;
		
		RNAPI size_t GetCount() const;
		
	protected:
		RNAPI void SetValueForUndefinedKey(Object *value, const char *key) override;
		RNAPI Object *GetValueForUndefinedKey(const char *key) const override;
		
	private:
		PIMPL<DictionaryInternal> _internals;
		
		RNAPI Object *GetPrimitiveObjectForKey(const Object *key) const;
		
		__RNDeclareMetaInternal(Dictionary)
	};
	
	RNObjectClass(Dictionary)
}

#endif /* __RAYNE_DICTIONARY_H__ */
