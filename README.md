# 4d-plugin-system-volume
4D plugin to control system audio volume and mute state.

```
$volume:=AUDIO Get volume 
AUDIO SET VOLUME (1)

$mute:=AUDIO Get mute 
AUDIO SET MUTE (1)

  //restore
AUDIO SET VOLUME ($volume)
AUDIO SET MUTE ($mute)
```
