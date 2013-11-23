//
//  RNNull.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNNull.h"

namespace RN
{
	RNDeclareMeta(Null)
	
	Null::Null()
	{}
	
	Null::~Null()
	{}
	
	Null *Null::GetNull()
	{
		static std::once_flag flag;
		static Null *null = nullptr;
		
		std::call_once(flag, [&]() {
			null = new Null();
		});
		
		return null;
	}
}