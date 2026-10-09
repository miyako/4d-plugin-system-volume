# 4d-plugin-system-volume

A 4D plugin that reads and sets the **system output volume** and **mute state** of the computer 4D is running on. It talks to the operating system's audio layer directly (Core Audio on macOS, the Windows Core Audio endpoint API on Windows) and always acts on the **default output device**. Volume is a `Real` between `0.0` and `1.0`; mute is a `Longint` used as a boolean.

| Command | Returns | Purpose |
|---|---|---|
| [AUDIO SET VOLUME](#audio-set-volume) | – | Set the master volume of the default output device |
| [AUDIO Get volume](#audio-get-volume) | `Real` | Read the master volume of the default output device |
| [AUDIO SET MUTE](#audio-set-mute) | – | Mute or unmute the default output device |
| [AUDIO Get mute](#audio-get-mute) | `Longint` | Read the mute state of the default output device |

**Platforms:** macOS (Carbon and Cocoa builds) · Windows 32-bit and 64-bit

---

## Requirements & platform notes

- **Windows Vista or later** is required (the plugin uses the Core Audio endpoint volume API, which does not exist on Windows XP).
- **All four commands act on the *default output device* only.** On Windows this is the default playback device for the "console" role; on macOS it is the system's default output device. Changing the default device in the OS changes which device these commands affect. There is no parameter to pick a device.
- **Every command has exactly the parameters listed below, and they are mandatory.** There are no optional parameters.
- **Failure is silent.** None of the commands raises a 4D error. If there is no output device, the device has no volume/mute control, or the OS call fails, a setter does nothing and a getter returns an unspecified default (most likely `0`; see [Error handling & troubleshooting](#error-handling--troubleshooting)). If your code needs to be sure a change took effect, read the value back (example under [AUDIO SET VOLUME](#audio-set-volume)).
- **Very low volume values behave differently from other values.** Values below a small internal threshold are *not* written as a volume; they unmute instead. See [AUDIO SET VOLUME](#audio-set-volume).
- **macOS devices without a master control.** Some devices (certain USB, HDMI and professional audio interfaces) expose volume or mute only per channel. On those devices the commands have nothing to act on and do nothing.

---

## AUDIO SET VOLUME

### Syntax

```4d
AUDIO SET VOLUME ( volume )
```

| Parameter | Type | Description |
|---|---|---|
| `volume` | Real | New volume, from `0.0` (silent) to `1.0` (maximum). Out-of-range values are clamped. |

This command returns nothing.

### Description

Sets the master volume of the default output device to `volume`.

- Values above `1.0` are treated as `1.0`; values below `0.0` are treated as `0.0`.
- **Very small values are not applied as a volume.** If `volume` is below a small internal threshold (defined in the plugin's source; its exact value is not documented here), the command **does not change the volume** and instead **clears the mute flag** (it unmutes). In practice this means `AUDIO SET VOLUME (0)` will not silence the system and will not lower the volume. To silence output, use [AUDIO SET MUTE](#audio-set-mute).
- Setting the volume does **not** unmute the device. If the device is muted, the new volume is stored but you will hear nothing until you call [AUDIO SET MUTE](#audio-set-mute) with `0`.
- Passing a non-number (NaN) is ignored and changes nothing. *(This guard exists in the revised source; builds made before that revision pass the value straight to the OS, which will most likely reject it.)*

The `volume` you pass is a scalar level, not decibels, and it is not guaranteed that reading it back returns exactly the same number: the hardware or driver may round it to the nearest step it supports.

### Example

From the plugin's own test method (`Method1.4dm`):

```4d
$volume:=AUDIO Get volume
AUDIO SET VOLUME(1)

$mute:=AUDIO Get mute
AUDIO SET MUTE(1)

//restore
AUDIO SET VOLUME($volume)
AUDIO SET MUTE($mute)
```

Because of the low-value rule above, the "restore" line `AUDIO SET VOLUME($volume)` will leave the volume at `1` (and unmute) if the original `$volume` was below the internal threshold. The mute restore on the next line still works.

Set a specific level and confirm that the OS accepted it:

```4d
C_REAL($target;$actual)
$target:=0.5

AUDIO SET VOLUME($target)
$actual:=AUDIO Get volume

If (Abs($actual-$target)>0.05)
  ALERT("The system volume could not be set to "+String($target))
End if
```

---

## AUDIO Get volume

### Syntax

```4d
AUDIO Get volume → Real
```

| Parameter | Type | Description |
|---|---|---|
| Result | Real | Current master volume, from `0.0` to `1.0`. |

### Description

Returns the current master volume of the default output device. The value is a scalar between `0.0` and `1.0`; on macOS it is clamped to that range before being returned.

The command reports the volume **setting**, regardless of whether the device is muted. A muted device still reports its stored volume. Use [AUDIO Get mute](#audio-get-mute) to find out whether output is actually audible.

If the volume cannot be read (no output device, no master volume control, OS error), the command does not raise an error and the returned value is whatever default a `Real` result holds. Do not treat a `0` result as proof that the volume really is zero.

### Example

```4d
C_REAL($volume)
$volume:=AUDIO Get volume
ALERT("System volume: "+String(Round($volume*100;0))+"%")
```

---

## AUDIO SET MUTE

### Syntax

```4d
AUDIO SET MUTE ( mute )
```

| Parameter | Type | Description |
|---|---|---|
| `mute` | Longint | `0` unmutes the output device. Any non-zero value mutes it. |

This command returns nothing.

### Description

Mutes or unmutes the default output device. Muting does not change the stored volume level; unmuting restores audio at whatever volume is currently set.

Any non-zero `mute` value (`1`, `2`, `-1`, ...) is treated as "mute". Use `0` and `1` for clarity.

On macOS, the command only has an effect if the device exposes a settable master mute control. Devices that provide mute per channel only are left unchanged.

### Example

Mute while something else happens, then restore the original state:

```4d
C_LONGINT($wasMuted)
$wasMuted:=AUDIO Get mute

AUDIO SET MUTE(1)
  //... do something that should be silent ...
AUDIO SET MUTE($wasMuted)
```

Toggle mute:

```4d
AUDIO SET MUTE(1-AUDIO Get mute)
```

---

## AUDIO Get mute

### Syntax

```4d
AUDIO Get mute → Longint
```

| Parameter | Type | Description |
|---|---|---|
| Result | Longint | `1` if the default output device is muted, `0` if it is not. |

### Description

Returns the mute state of the default output device, normalised to `1` or `0`.

If the state cannot be read (no output device, no mute control on the device, OS error), the command raises no error and returns the default `Longint` result, most likely `0`. A `0` result can therefore mean either "not muted" or "could not tell".

### Example

```4d
If (AUDIO Get mute=1)
  ALERT("The system is muted.")
Else
  ALERT("The system is not muted.")
End if
```

---

## Worked examples

### Fade in from quiet to full volume

Starts at `0.1` rather than `0`, because values below the internal threshold would unmute rather than set a level (see [AUDIO SET VOLUME](#audio-set-volume)).

```4d
C_LONGINT($i)
C_REAL($original)

$original:=AUDIO Get volume
AUDIO SET MUTE(0)

For ($i;1;10)
  AUDIO SET VOLUME($i/10)
  DELAY PROCESS(Current process;30)
End for

  //put it back
AUDIO SET VOLUME($original)
```

### Play something at a guaranteed level, then restore everything

```4d
C_REAL($oldVolume)
C_LONGINT($oldMute)

$oldVolume:=AUDIO Get volume
$oldMute:=AUDIO Get mute

AUDIO SET MUTE(0)
AUDIO SET VOLUME(0.8)

  //... play a sound or alert ...

AUDIO SET VOLUME($oldVolume)
AUDIO SET MUTE($oldMute)
```

---

## Error handling & troubleshooting

- **No command ever raises a 4D error.** A command that cannot do its job simply returns (setters) or returns a default value (getters). Build any check you need yourself by reading the value back after setting it.
- **A getter returning `0` is ambiguous.** `AUDIO Get volume` returning `0` or `AUDIO Get mute` returning `0` can mean "silent / not muted" or "the read failed". If it matters, test on the actual hardware you ship against.
- **`AUDIO SET VOLUME (0)` does not silence the system.** Values below the internal threshold clear the mute flag and leave the volume alone. Use `AUDIO SET MUTE (1)` to silence output.
- **Volume restored to the wrong level.** If you save the volume, change it, and restore it later, a saved value below the internal threshold will not be re-applied (the restore unmutes instead). Restore the mute state afterwards with [AUDIO SET MUTE](#audio-set-mute).
- **You set the volume but hear nothing.** The device may be muted; setting the volume does not unmute it. Check [AUDIO Get mute](#audio-get-mute).
- **Nothing happens on macOS with a USB, HDMI or pro audio device.** These often have volume or mute per channel only, with no master control. The plugin only drives the master control, so it has nothing to change.
- **Wrong device is affected.** The commands always follow the OS default output device. If headphones or an external display became the default, they are the device being controlled.
- **Windows XP or earlier.** Not supported; Windows Vista or later is required.
- **Platform and version history.** Behaviour described as "revised source" above (ignoring NaN, and a fix that stops the Windows build leaking one COM object reference per call) applies once you build or install a version that includes those changes.

---

## Quick reference

```4d
$volume:=AUDIO Get volume    //Real, 0.0 to 1.0
AUDIO SET VOLUME(0.5)        //values below a small threshold unmute instead of setting a level

$mute:=AUDIO Get mute        //Longint, 1 = muted, 0 = not muted
AUDIO SET MUTE(1)            //mute (any non-zero)
AUDIO SET MUTE(0)            //unmute

  //restore
AUDIO SET VOLUME($volume)
AUDIO SET MUTE($mute)
```
