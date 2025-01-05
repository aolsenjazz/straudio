
#include <TargetConditionals.h>
#if TARGET_OS_IOS == 1
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

#define IPLUG_AUVIEWCONTROLLER IPlugAUViewController_vStraudio
#define IPLUG_AUAUDIOUNIT IPlugAUAudioUnit_vStraudio
#import <StraudioAU/IPlugAUViewController.h>
#import <StraudioAU/IPlugAUAudioUnit.h>

//! Project version number for StraudioAU.
FOUNDATION_EXPORT double StraudioAUVersionNumber;

//! Project version string for StraudioAU.
FOUNDATION_EXPORT const unsigned char StraudioAUVersionString[];

@class IPlugAUViewController_vStraudio;
