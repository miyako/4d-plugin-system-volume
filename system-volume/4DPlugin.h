/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.h
 #	source generated by 4D Plugin Wizard
 #	Project : system-volume
 #	author : miyako
 #	2015/07/08
 #
 # --------------------------------------------------------------------------------*/


#if VERSIONMAC
#import <CoreAudio/CoreAudio.h>
#import <AudioToolbox/AudioServices.h>
#else
#include <commctrl.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#endif

// --- System Volume
void AUDIO_SET_VOLUME(sLONG_PTR *pResult, PackagePtr pParams);
void AUDIO_Get_volume(sLONG_PTR *pResult, PackagePtr pParams);
void AUDIO_SET_MUTE(sLONG_PTR *pResult, PackagePtr pParams);
void AUDIO_Get_mute(sLONG_PTR *pResult, PackagePtr pParams);

#if VERSIONMAC
void AUDIO_SET_MUTE(UInt32 muted);
#else
void AUDIO_SET_MUTE(BOOL bMute);
#endif

#define	THRESHOLD	0.005