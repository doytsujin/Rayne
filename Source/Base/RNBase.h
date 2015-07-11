//
//  RNBase.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef _RAYNE_BASE_H_
#define _RAYNE_BASE_H_

// ---------------------------
// Platform independent includes
// ---------------------------

#ifdef RAYNE_INCLUDE_PREFIX
	#include <RAYNE_INCLUDE_PREFIX/RayneConfig.h>
#else
	#include <RayneConfig.h>
#endif

#include "RNMemory.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <type_traits>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>
#include <iterator>
#include <array>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <deque>
#include <tuple>
#include <chrono>
#include <utility>
#include <regex>
#include <thread>
#include <future>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <unordered_map>
#include <unordered_set>

#include "RNDefines.h"
#include "RNException.h"
#include "RNOptions.h"
#include "RNExpected.h"
#include "RNScopeGuard.h"
#include "RNLockGuard.h"
#include "../Math/RNConstants.h"
#include "../Math/RNMath.h"
#include "../Threads/RNSpinLock.h"

// ---------------------------
// Platform dependent includes
// ---------------------------

#if RN_PLATFORM_MAC_OS
	#include <mach/mach.h>
	#include <CoreGraphics/CoreGraphics.h>
#endif

// ---------------------------
// Helper macros
// ---------------------------


#define RN_ASSERT(e, ...) RN_EXPECT_FALSE(!(e)) ? RN::__Assert(RN_FUNCTION_SIGNATURE, __FILE__, __LINE__, #e, __VA_ARGS__) : (void)0

#define RN_REGISTER_INITIALIZER(name, body) \
	namespace { \
		static void __RNGlobalInit##name##Callback() { body; } \
		static RN::Initializer __RNGlobalInit##name (__RNGlobalInit##name##Callback, nullptr); \
	}

#define RN_REGISTER_DESTRUCTOR(name, body) \
	namespace { \
		static void __RNGlobalDestructor##name##Callback() { body; } \
		static RN::Initializer __RNGlobalDestructor##name (nullptr, __RNGlobalDestructor##name##Callback); \
	}

namespace RN
{
	class Kernel;
	class Application;

	using Clock = std::chrono::high_resolution_clock;

	RNAPI RN_NORETURN void Initialize(int argc, char *argv[], Application *app);
	RNAPI RN_NORETURN void __Assert(const char *func, const char *file, int line, const char *expression, const char *message, ...);

	/**
	 * Returns the ABI version. Rayne differentiates between ABI and API version,
	 * meaning that builds with different API versions may return the same ABI version
	 * and thus don't require re-compilation of the whole project to properly link (ie,
	 * the libraries can be replaced via drag and drop.
	 * This function can be used as a reference as it's ABI is guaranteed to be stable,
	 * so it's safe to link against this function, call it, and compare the value with the
	 * compile time constant kRNABIVersion, to validate that both match
	 **/
	RNAPI uint32 GetABIVersion() RN_NOEXCEPT;
	/**
	 * Returns the API version of Rayne. The API version is a single 32-bit unsigned integer
	 * value that is created by putting the major, minor and patch version together.
	 * The API version is generated by (major << 16) | (minor << 8) | (patch)
	 **/
	RNAPI uint32 GetAPIVersion() RN_NOEXCEPT;

	/**
	 * Returns the major version of Rayne
	 **/
	RNAPI uint32 GetMajorVersion() RN_NOEXCEPT;
	/**
	 * Returns the minor version of Rayne
	 **/
	RNAPI uint32 GetMinorVersion() RN_NOEXCEPT;
	/**
	 * Returns the patch version of Rayne
	 **/
	RNAPI uint32 GetPatchVersion() RN_NOEXCEPT;

	typedef uint64 Tag;

	enum class ComparisonResult : int
	{
		LessThan   = -1,
		EqualTo     = 0,
		GreaterThan = 1
	};

	class Initializer
	{
	public:
		typedef void (*Callback)();

		Initializer(Callback ctor, Callback dtor) :
				_dtor(dtor)
		{
			if(ctor)
				ctor();
		}

		~Initializer()
		{
			if(_dtor)
				_dtor();
		}

	private:
		Callback _dtor;
	};

	class Range
	{
	public:
		Range() = default;
		Range(size_t torigin, size_t tlength) :
				origin(torigin),
				length(tlength)
		{}

		size_t GetEnd() const
		{
			return origin + length;
		}

		bool Contains(const Range &other) const
		{
			return (other.origin >= origin && GetEnd() >= other.GetEnd());
		}

		bool Overlaps(const Range &other) const
		{
			return (GetEnd() >= other.origin && origin <= other.GetEnd());
		}

		size_t origin;
		size_t length;
	};

	template <class T>
	class PIMPL
	{
	public:
		template<class ...Args>
		PIMPL(Args &&...args) :
				_ptr(new T(std::forward<Args>(args)...))
		{}

		operator T* ()
		{
			return _ptr.get();
		}
		operator const T* () const
		{
			return _ptr.get();
		}

		T *operator ->()
		{
			return _ptr.get();
		}
		const T *operator ->() const
		{
			return _ptr.get();
		}

	private:
		std::unique_ptr<T> _ptr;
	};
}

#endif /* _RAYNE_BASE_H_ */
