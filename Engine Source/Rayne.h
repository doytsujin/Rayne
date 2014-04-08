//
//  Rayne.h
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_H__
#define __RAYNE_H__

#include "RNBase.h"
#include "RNSerialization.h"
#include "RNAutoreleasePool.h"
#include "RNObject.h"
#include "RNString.h"
#include "RNArray.h"
#include "RNSet.h"
#include "RNCountedSet.h"
#include "RNDictionary.h"
#include "RNData.h"
#include "RNNumber.h"
#include "RNValue.h"
#include "RNJSONSerialization.h"
#include "RNIndexSet.h"
#include "RNIndexPath.h"
#include "RNKVO.h"
#include "RNKVOImplementation.h"
#include "RNFormatter.h"
#include "RNProgress.h"

#include "RNOpenGL.h"
#include "RNOpenGLQueue.h"

#include "RNKernel.h"
#include "RNApplication.h"
#include "RNSettings.h"
#include "RNInput.h"
#include "RNMessage.h"
#include "RNTimer.h"
#include "RNSignal.h"
#include "RNFile.h"
#include "RNPathManager.h"
#include "RNFileManager.h"
#include "RNOpenPanel.h"
#include "RNDebug.h"
#include "RNLogging.h"
#include "RNLoggingEngine.h"

#include "RNModule.h"
#include "RNWorldAttachment.h"
#include "RNSceneNodeAttachment.h"

#include "RNResourceCoordinator.h"
#include "RNResourceLoader.h"
#include "RNSkeleton.h"
#include "RNModel.h"

#include "RNMath.h"
#include "RNSIMD.h"
#include "RNCPU.h"
#include "RNMemory.h"
#include "RNRandom.h"

#include "RNAABB.h"
#include "RNColor.h"
#include "RNMatrixQuaternion.h"
#include "RNMatrix.h"
#include "RNQuaternion.h"
#include "RNSphere.h"
#include "RNRect.h"
#include "RNVector.h"
#include "RNInterpolation.h"
#include "RNHit.h"

#include "RNRenderer.h"

#include "RNCamera.h"
#include "RNMaterial.h"
#include "RNMesh.h"
#include "RNShader.h"
#include "RNShaderUnit.h"
#include "RNTexture.h"
#include "RNTextureAtlas.h"

#include "RNUIStyle.h"
#include "RNUIFont.h"
#include "RNUITypesetter.h"
#include "RNUIImage.h"
#include "RNUIGeometry.h"
#include "RNUIColor.h"
#include "RNUIServer.h"
#include "RNUIResponder.h"
#include "RNUIWidget.h"
#include "RNUIView.h"
#include "RNUIControl.h"
#include "RNUIScrollView.h"
#include "RNUIScroller.h"
#include "RNUITableView.h"
#include "RNUITableViewCell.h"
#include "RNUIOutlineView.h"
#include "RNUIOutlineViewCell.h"
#include "RNUIButton.h"
#include "RNUILabel.h"
#include "RNUITextField.h"
#include "RNUITextEditor.h"
#include "RNUIImageView.h"
#include "RNUISegmentView.h"
#include "RNUIProgressIndicator.h"
#include "RNUIUtilities.h"
#include "RNUIMenu.h"

#include "RNWorldCoordinator.h"
#include "RNWorld.h"
#include "RNSceneNode.h"
#include "RNEntity.h"
#include "RNLight.h"
#include "RNBillboard.h"
#include "RNDecal.h"
#include "RNWater.h"
#include "RNInstancingNode.h"
#include "RNParticleEmitter.h"
#include "RNParticle.h"
#include "RNTriggerZone.h"
#include "RNTextNode.h"
#include "RNTerrain.h"

#include "RNThread.h"
#include "RNThreadPool.h"
#include "RNSemaphore.h"
#include "RNSpinLock.h"
#include "RNMutex.h"
#include "RNLockGuard.h"
#include "RNFunction.h"
#include "RNScopeGuard.h"
#include "RNWindow.h"

#include "RNSTL.h"
#include "RNAny.h"
#include "RNRingBuffer.h"
#include "RNIntervalTree.h"
#include "RNSHA2.h"
#include "RNSpatialMap.h"
#include "RNSyncPoint.h"

#if RN_PLATFORM_WINDOWS
	#undef RNAPI_DEFINEBASE

	#if RN_BUILD_MODULE
		#define RNAPI_DEFINEBASE __declspec(dllexport)
	#else
		#define RNAPI_DEFINEBASE
	#endif
#endif

#endif /* __RAYNE_H__ */
