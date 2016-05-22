//
//  RNLockable.cpp
//  Rayne
//
//  Copyright 2016 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "../Base/RNBase.h"
#include "RNLockable.h"
#include "RNThreadPark.h"

namespace RN
{
	void Lockable::LockSlowPath()
	{
		size_t spinCount = 0;
		constexpr size_t spinLimit = 40;

		while(1)
		{
			uint8 value = _flag.load(std::memory_order_acquire);


			if(!(value & kLockFlagLocked) && _flag.compare_exchange_weak(value, value | kLockFlagLocked, std::memory_order_release))
			{
#if RN_BUILD_DEBUG
				_thread = std::this_thread::get_id();
#endif
				return;
			}

			if(!(value & kLockFlagParked) && spinCount < spinLimit)
			{
				spinCount ++;
				std::this_thread::yield();

				continue;
			}

			if(!(value & kLockFlagParked) && !_flag.compare_exchange_weak(value, value | kLockFlagParked, std::memory_order_release))
				continue;

			__Private::ThreadPark::CompareAndPark(&_flag, kLockFlagParked | kLockFlagLocked);
		}
	}

	void Lockable::UnlockSlowPath()
	{
		while(1)
		{
			uint8 value = _flag.load(std::memory_order_acquire);

			if(value == kLockFlagLocked)
			{
				if(!_flag.compare_exchange_weak(value, 0, std::memory_order_release))
					continue;

				return;
			}

			__Private::ThreadPark::UnparkThread(&_flag, [this](__Private::ThreadPark::UnparkResult result) {

				if(result & __Private::ThreadPark::UnparkResult::HasMoreThreads)
					_flag.store(kLockFlagParked, std::memory_order_release);
				else
					_flag.store(0, std::memory_order_release);

			});
		}
	}
}
