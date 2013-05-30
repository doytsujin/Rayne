//
//  RNJSONSerialization.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include <jansson.h>
#include "RNJSONSerialization.h"
#include "RNArray.h"
#include "RNDictionary.h"
#include "RNNumber.h"
#include "RNString.h"

namespace RN
{
	static MetaClass *__JSONArrayClass = 0;
	static MetaClass *__JSONDictionaryClass = 0;
	static MetaClass *__JSONNumberClass = 0;
	static MetaClass *__JSONStringClass = 0;
	
	void JSONReadClasses()
	{
		static std::once_flag onceToken;
		std::call_once(onceToken, []{
			__JSONArrayClass = Array::MetaClass();
			__JSONDictionaryClass = Dictionary::MetaClass();
			__JSONNumberClass = Number::MetaClass();
			__JSONStringClass = String::MetaClass();
		});
	}
	
	
	bool JSONSerialization::IsValidJSONObject(Object *object)
	{
		JSONReadClasses();
		
		if(object->IsKindOfClass(__JSONArrayClass))
			return true;
		
		if(object->IsKindOfClass(__JSONDictionaryClass))
			return true;
		
		if(object->IsKindOfClass(__JSONNumberClass))
			return true;
		
		if(object->IsKindOfClass(__JSONStringClass))
			return true;
		
		return false;
	}
	
	void *JSONSerialization::SerializeObject(Object *object)
	{
		json_t *json = nullptr;
		
		if(object->IsKindOfClass(__JSONNumberClass))
		{
			Number *number = static_cast<Number *>(object);
			switch(number->NumberType())
			{
				case Number::Type::Float32:
				case Number::Type::Float64:
					json = json_real(number->DoubleValue());
					break;
					
				case Number::Type::Uint8:
				case Number::Type::Uint16:
				case Number::Type::Uint32:
					json = json_integer(number->Uint32Value());
					break;
					
				case Number::Type::Int8:
				case Number::Type::Int16:
				case Number::Type::Int32:
					json = json_integer(number->Int32Value());
					break;
					
				case Number::Type::Uint64:
					json = json_integer(number->Uint64Value());
					break;
					
				case Number::Type::Int64:
					json = json_integer(number->Int64Value());
					break;
					
				case Number::Type::Boolean:
					json = json_boolean(number->BoolValue());
					break;
			}
		}
		
		if(object->IsKindOfClass(__JSONStringClass))
		{
			String *string = static_cast<String *>(object);
			const char *utf8 = reinterpret_cast<const char *>(string->BytesWithEncoding(String::Encoding::UTF8, false, nullptr));
			
			json = json_string_nocheck(utf8);
		}
		
		if(object->IsKindOfClass(__JSONArrayClass))
		{
			Array *array = static_cast<Array *>(object);
			json = json_array();
			
			array->Enumerate([&](Object *object, size_t index, bool *stop) {
				json_t *data = static_cast<json_t *>(SerializeObject(object));
				json_array_append_new(json, data);
			});
		}
		
		if(object->IsKindOfClass(__JSONDictionaryClass))
		{
			Dictionary *dictionary = static_cast<Dictionary *>(object);
			json = json_object();
			
			dictionary->Enumerate([&](Object *object, Object *key, bool *stop) {
				if(key->IsKindOfClass(__JSONStringClass))
				{
					String *string = static_cast<String *>(key);
					const char *utf8 = reinterpret_cast<const char *>(string->BytesWithEncoding(String::Encoding::UTF8, false, nullptr));
					
					json_t *data = static_cast<json_t *>(SerializeObject(object));
					json_object_set_new_nocheck(json, utf8, data);
				}
			});
		}
		
		return json;
	}
	
	Data *JSONSerialization::JSONDataFromObject(Object *root, SerializerOptions options)
	{
		JSONReadClasses();
		
		if(!root->IsKindOfClass(__JSONArrayClass) && !root->IsKindOfClass(__JSONDictionaryClass))
			return nullptr;
		
		size_t flags = 0;
		
		if(options & PrettyPrint)
		{
			flags |= JSON_INDENT(4);
		}
		
		json_t *json = static_cast<json_t *>(SerializeObject(root));
		char *data = json_dumps(json, flags);
		
		Data *temp = new Data(reinterpret_cast<uint8 *>(data), strlen(data));
		free(data);
		
		return temp->Autorelease();
	}
	
	Object *JSONObjectFromData(Data *data)
	{
		return 0;
	}
}
