#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
#import <CoreAudio/CoreAudio.h>
#import <AudioToolbox/AudioServices.h>

// Force-included before every translation unit, including the complete SDK.
@interface OwnershipURL : NSObject
- (id)initWithString:(NSString *)value;
- (NSString *)absoluteString;
@end
@interface OwnershipImage : NSObject
- (id)initWithCGImage:(CGImageRef)image size:(NSSize)size;
- (NSData *)TIFFRepresentation;
@end
CFURLRef OwnershipCreateURL(CFAllocatorRef, CFStringRef, CFURLPathStyle, Boolean);
CFStringRef OwnershipCopyPath(CFURLRef, CFURLPathStyle);
Boolean OwnershipAudioHas(AudioObjectID, const AudioObjectPropertyAddress*);
OSStatus OwnershipAudioGet(AudioObjectID, const AudioObjectPropertyAddress*, UInt32, const void*, UInt32*, void*);
OSStatus OwnershipAudioSet(AudioObjectID, const AudioObjectPropertyAddress*, UInt32, const void*, UInt32, const void*);
OSStatus OwnershipAudioSettable(AudioObjectID, const AudioObjectPropertyAddress*, Boolean*);
void OwnershipLog(NSString*, ...);
#define NSURL OwnershipURL
#define NSImage OwnershipImage
#define CFURLCreateWithFileSystemPath OwnershipCreateURL
#define CFURLCopyFileSystemPath OwnershipCopyPath
#define AudioObjectHasProperty OwnershipAudioHas
#define AudioObjectGetPropertyData OwnershipAudioGet
#define AudioObjectSetPropertyData OwnershipAudioSet
#define AudioObjectIsPropertySettable OwnershipAudioSettable
#define NSLog OwnershipLog
