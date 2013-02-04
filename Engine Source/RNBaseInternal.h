//
//  RNBaseInternal.h
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_BASEINTERNAL_H__
#define __RAYNE_BASEINTERNAL_H__

/**
 * This header is for INTERNAL use only!
 * Don't include it from other .h files!
 **/

#include "RNBase.h"

#if RN_PLATFORM_IOS
	#import <UIKit/UIKit.h>
	#import <QuartzCore/QuartzCore.h>
	#import <CoreMotion/CoreMotion.h>
#endif

#endif /* __RAYNE_BASEINTERNAL_H__ */