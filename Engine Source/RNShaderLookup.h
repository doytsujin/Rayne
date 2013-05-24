//
//  RNShaderLookup.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_SHADERLOOKUP_H__
#define __RAYNE_SHADERLOOKUP_H__

#include "RNBase.h"

namespace RN
{
	struct ShaderDefine
	{
		ShaderDefine(const std::string& tname, const std::string& tvalue) :
			name(tname),
			value(tvalue)
		{}
		ShaderDefine(const std::string& tname, const int tvalue) :
		name(tname)
		{
			char buffer[32];
			sprintf(buffer, "%i", tvalue);
			value = buffer;
		}
		
		
		std::string name;
		std::string value;
	};
	
	struct ShaderLookup
	{
		ShaderLookup(uint32 ttype)
		{
			std::hash<std::string> hash_fn;
			
			hash = 0;
			type = ttype;
			hash = hash_fn("");
		}
		
		ShaderLookup(const std::vector<ShaderDefine>& tdefines)
		{
			defines = tdefines;
			std::sort(defines.begin(), defines.end(), [](const ShaderDefine& left, const ShaderDefine& right) {
				return (left.name.compare(right.name) < 0);
			});
			
			for(auto i=defines.begin(); i!=defines.end(); i++)
			{
				mangledDefines += i->name;
				mangledDefines += i->value;
			}
			
			std::hash<std::string> hash_fn;
			hash = (mangledDefines.length() > 0) ? hash_fn(mangledDefines) : 0;
			type = 0;
		}
		
		ShaderLookup operator +(const ShaderLookup& other) const
		{
			ShaderLookup lookup(*this);
			std::hash<std::string> hash_fn;
			
			if(other.mangledDefines.length() > 0)
			{
				lookup.defines.insert(lookup.defines.end(), other.defines.begin(), other.defines.end());
				lookup.mangledDefines += other.mangledDefines;
				lookup.hash = hash_fn(lookup.mangledDefines);
			}
			
			lookup.type |= other.type;
			
			return lookup;
		}

		uint32 type;
		size_t hash;
		std::string mangledDefines;
		std::vector<ShaderDefine> defines;
	};
}

namespace std
{
	template<>
	struct hash<RN::ShaderLookup>
	{
		size_t operator()(const RN::ShaderLookup& lookup) const
		{
			size_t hash = static_cast<size_t>(lookup.type);
			
			hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
			hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
			hash = ((hash >> 16) ^ hash);
			
			size_t result = hash ^ lookup.hash;
			return result;
		}
	};
	
	template<>
	struct equal_to<RN::ShaderLookup>
	{
		bool operator()(const RN::ShaderLookup& lookup1, const RN::ShaderLookup& lookup2) const 
		{
			return (lookup1.type == lookup2.type && lookup1.mangledDefines == lookup2.mangledDefines);
		}
	};
}

#endif /* __RAYNE_SHADERLOOKUP_H__ */
