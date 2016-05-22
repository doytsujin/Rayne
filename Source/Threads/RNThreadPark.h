//
//  RNPThreadPark.h
//  Rayne
//
//  Copyright 2016 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_THREADPARK_H_
#define __RAYNE_THREADPARK_H_

#ifdef RN_BUILD_LIBRARY
	#include <RayneConfig.h>
#else
	#include "../RayneConfig.h"
#endif

#include <atomic>
#include <functional>
#include "../Base/RNOptions.h"

namespace RN
{
	namespace __Private
	{
		class ThreadPark
		{
		public:
			RN_OPTIONS(UnparkResult, uint32,
				UnparkedThread = (1 << 0),
				HasMoreThreads = (1 << 1));

			ThreadPark() = delete;
			ThreadPark(const ThreadPark &) = delete;

			template<class Validation, class BeforeSleep>
			static bool Park(const void *address, Validation &&validation, BeforeSleep &&beforeSleep, Clock::time_point timeout)
			{
				return __ParkConditionally(address, std::function<bool()>(std::forward<Validation>(validation)), std::function<void()>(std::forward<BeforeSleep>(beforeSleep)), timeout);
			}

			template<class Validation>
			static bool Park(const void *address, Validation &&validation)
			{
				return __ParkConditionally(address, std::function<bool()>(std::forward<Validation>(validation)), []() { }, Clock::time_point::max());
			}

			static bool Park(const void *address)
			{
				return __ParkConditionally(address, []() -> bool {
					return true;
				}, []() { }, Clock::time_point::max());
			}

			template<class T, class U>
			RNAPI static bool CompareAndPark(const std::atomic<T> *address, U expected)
			{
				return Park(address, [address, expected]() -> bool {

					U value = address->load();
					return (value == expected);

				});
			}

			RNAPI static UnparkResult UnparkThread(const void *address);
			RNAPI static void UnparkThread(const void *address, std::function<void(UnparkResult)> callback);
			RNAPI static void UnparkAllThreads(const void *address);

		private:
			RNAPI static bool __ParkConditionally(const void *address, std::function<bool()> validation, std::function<void()> beforeSleep, Clock::time_point timeout);
		};
	}
}


#endif /* __RAYNE_THREADPARK_H_ */
