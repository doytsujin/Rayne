//
//  RNBase.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include <locale>
#include "RNBaseInternal.h"
#include "RNKernel.h"

#if RN_PLATFORM_MAC_OS

@interface RNApplication : NSApplication <NSApplicationDelegate>
@end

@implementation RNApplication

- (void)sendEvent:(NSEvent *)event
{
	if([event type] == NSKeyUp && ([event modifierFlags] & NSCommandKeyMask))
		[[self keyWindow] sendEvent:event];

	[super sendEvent:event];
}

@end

#endif

namespace RN
{
	static MemoryPool *__functionPool;

	struct __KernelBootstrapHelper
	{
	public:
		static Kernel *BootstrapKernel(Application *app, const ArgumentParser &arguments)
		{
			__functionPool = new MemoryPool();

			if(!arguments.HasArgument("no-locale", 'l'))
			{
				const char *result = setlocale(LC_ALL, "en_US.UTF-8");
				if(!result)
					std::cerr << "Couldn't set locale" << std::endl;
			}

			Kernel *result = new Kernel(app, arguments);
#if RN_PLATFORM_MAC_OS
			@autoreleasepool {
				result->Bootstrap();
			}
#else
			result->Bootstrap();
#endif

			return result;
		}

		static void TearDownKernel(Kernel *kernel)
		{
#if RN_PLATFORM_MAC_OS
			@autoreleasepool {
				kernel->TearDown();
			}
#else
			kernel->TearDown();
#endif

			delete __functionPool;
			__functionPool = nullptr;
		}
	};

	RNAPI Kernel *__BootstrapKernel(Application *app, const ArgumentParser &arguments);
	RNAPI void __TearDownKernel(Kernel *kernel);


	MemoryPool *__GetFunctionPool()
	{
		return __functionPool;
	}

	Kernel *__BootstrapKernel(Application *app, const ArgumentParser &arguments)
	{
		Kernel *result = __KernelBootstrapHelper::BootstrapKernel(app, arguments);
		return result;
	}

	void __TearDownKernel(Kernel *kernel)
	{
		__KernelBootstrapHelper::TearDownKernel(kernel);
	}

	void Initialize(int argc, const char *argv[], Application *app)
	{
		RN_ASSERT(app, "Application mustn't be NULL");

#if RN_PLATFORM_MAC_OS
		@autoreleasepool {
			[RNApplication sharedApplication];
			[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
			[NSApp finishLaunching];

			[[RNApplication sharedApplication] setDelegate:(RNApplication *)[RNApplication sharedApplication]];
			[[RNApplication sharedApplication] activateIgnoringOtherApps:YES];
			
			@autoreleasepool {
				
				NSDate *date = [NSDate date];
				NSEvent *event;
				
				while((event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:date inMode:NSDefaultRunLoopMode dequeue:YES]))
				{
					[NSApp sendEvent:event];
					[NSApp updateWindows];
				}

				NSMenu *menu = [[NSMenu alloc] init];
				[menu setAutoenablesItems:NO];

				NSString *quitTitle = [@"Quit " stringByAppendingString:[[NSProcessInfo processInfo] processName]];
				NSMenuItem *quitMenuItem = [[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"];
				NSMenu *quitMenu = [[NSMenu alloc] init];
				[quitMenu addItem:[quitMenuItem autorelease]];

				NSMenuItem *appMenu = [[NSMenuItem alloc] init];
				[appMenu setSubmenu:[quitMenu autorelease]];
				[menu addItem:[appMenu autorelease]];

				[NSApp setMainMenu:[menu autorelease]];
			}
		}
#endif

		ArgumentParser arguments(argc, argv);

		Kernel *result = __BootstrapKernel(app, arguments);
		result->Run();

		__TearDownKernel(result);

		std::exit(EXIT_SUCCESS);
	}

	void __Assert(const char *func, const char *file, int line, const char *expression, const char *message, ...)
	{
		va_list args;
		va_start(args, message);

		char reason[1024];
		vsprintf(reason, message, args);
		reason[1023] = '\0';

		va_end(args);


		{
			RNError("Assertion '" << expression << "' failed in '" << func << "', " << file << ":" << line);
			RNError("Reason: " << reason);
		}

		abort();
	}

	uint32 GetABIVersion() RN_NOEXCEPT
	{
		return kRNABIVersion;
	}
	uint32 GetAPIVersion() RN_NOEXCEPT
	{
		return RNVersionMake(kRNVersionMajor, kRNVersionMinor, kRNVersionPatch);
	}

	uint32 GetMajorVersion() RN_NOEXCEPT
	{
		return kRNVersionMajor;
	}
	uint32 GetMinorVersion() RN_NOEXCEPT
	{
		return kRNVersionMinor;
	}
	uint32 GetPatchVersion() RN_NOEXCEPT
	{
		return kRNVersionPatch;
	}
	String *GetVersionString(uint32 version)
	{
		String *versionString = RNSTR(RNVersionGetMajor(version) << "." << RNVersionGetMinor(version));

		if(RNVersionGetPatch(version) > 0)
			versionString->Append(RNSTR("." << RNVersionGetPatch(version)));

		return versionString;
	}
}
