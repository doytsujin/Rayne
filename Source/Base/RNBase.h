//
//  RNBase.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef _RAYNE_BASE_H_
#define _RAYNE_BASE_H_

#ifdef RN_BUILD_LIBRARY
	#include <RayneConfig.h>
#else
	#include "../RayneConfig.h"
#endif

// ---------------------------
// Platform dependent includes
// ---------------------------

#if RN_PLATFORM_MAC_OS
	#include <mach/mach.h>
	#include <CoreGraphics/CoreGraphics.h>
#endif
#if RN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
	#endif

	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#ifndef _USE_MATH_DEFINES
		#define _USE_MATH_DEFINES
	#endif

	//#define _AMD64_

	//#include <ntdef.h>
	#include <WinSock2.h>
	#include <windows.h>
	#include <NTDDNDIS.h>

	#include <commdlg.h>
	#include <ShlObj.h>
	#include <Psapi.h>

	#undef near
	#undef far
#endif
#if RN_PLATFORM_LINUX
	#include <xcb/xcb.h>
#endif

// ---------------------------
// Platform independent includes
// ---------------------------

#include "RNMemory.h"

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cstdarg>

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

#include "RNException.h"
#include "RNOptions.h"
#include "RNExpected.h"
#include "RNScopeGuard.h"
#include "../Math/RNConstants.h"
#include "../Math/RNMath.h"
#include "../Threads/RNCondition.h"
#include "../Threads/RNLockable.h"
#include "../Threads/RNRecursiveLockable.h"
#include "../Threads/RNLockGuard.h"
#include "../Threads/RNUniqueLock.h"
#include "../Threads/RNLockWrapper.h"


// ---------------------------
// Helper macros
// ---------------------------

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

#define RNVersionMake(major, minor, patch) \
	((RN::uint32)(((major & 0x7ff) << 21) | ((minor & 0x7ff) << 10) | (patch & 0x3ff)))

#define RNVersionGetMajor(version) ((RN::uint32)(version) >> 21)
#define RNVersionGetMinor(version) (((RN::uint32)(version) >> 10) & 0x7ff)
#define RNVersionGetPatch(version) ((RN::uint32)(version) & 0x3ff)

namespace RN
{
	class Kernel;
	class Application;

	RNAPI RN_NORETURN void Initialize(int argc, const char *argv[], Application *app);
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

	RNAPI String *GetVersionString(uint32 version);


	typedef uint64 Tag;

	enum class ComparisonResult : int
	{
		LessThan   = -1, // lhs < rhs
		EqualTo     = 0, // lhs == rhs
		GreaterThan = 1  // lhs > rhs
	};

	static RN_INLINE std::ostream &operator<< (std::ostream &os, const ComparisonResult &result)
	{
		switch(result)
		{
			case ComparisonResult::LessThan:
				return os << "ComparisonResult::LessThan";
			case ComparisonResult::EqualTo:
				return os << "ComparisonResult::EqualTo";
			case ComparisonResult::GreaterThan:
				return os << "ComparisonResult::GreaterThan";
		}

		return os << "{invalid}";
	}

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

		bool operator ==(const Range &other) const
		{
			if(origin == kRNNotFound || other.origin == kRNNotFound)
				return (origin == other.origin);

			return (origin == other.origin && length == other.length);
		}
		bool operator !=(const Range &other) const
		{
			return !(*this == other);
		}

		size_t origin;
		size_t length;
	};

	static RN_INLINE std::ostream &operator<< (std::ostream &os, const Range &range)
	{
		return os << "{" << range.origin << ", " << range.length << "}";
	}

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
