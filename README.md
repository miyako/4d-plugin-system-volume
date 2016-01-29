# 4d-plugin-system-volume
4D plugin to control system audio volume and mute state.

##Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🆗|🆗|

Commands
---

```c
// --- System Volume
AUDIO_SET_VOLUME
AUDIO_Get_volume
AUDIO_SET_MUTE
AUDIO_Get_mute
```

```
$volume:=AUDIO Get volume 
AUDIO SET VOLUME (1)

$mute:=AUDIO Get mute 
AUDIO SET MUTE (1)

  //restore
AUDIO SET VOLUME ($volume)
AUDIO SET MUTE ($mute)
```

Uses [IAudioEndpointVolume](https://msdn.microsoft.com/en-us/library/aa964574.aspx) on Windows, [Audio Hardware Services](https://developer.apple.com/library/mac/documentation/AudioToolbox/Reference/AudioHardwareServicesReference/index.html#//apple_ref/c/func/AudioHardwareServiceGetPropertyData) on OS X.

**Note**: Windows require Vista or later.
