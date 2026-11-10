# System-volume native ownership audit

## Provenance and scope

Baseline: `207883840432cdad1ddf915b036e688d2936ff01` on `master`.
Canonical guidance read through the GitHub contents API:
`miyako/skills/4d-skills/skills/4dplugin/SKILL.md` at immutable
`e6b492fcb81e5e1a7f7db723594b420e63bf6eb9`.
Host scaffolding adapted, not results copied, from `miyako/4d-plugin-script`
at `c13f12088f8c88b2f0fa6f5e79104759707ec7e2`.

The complete bundled SDK is compiled unchanged, not extracted functions or a
replacement API. Its implementation labels itself v14, while its public
header labels itself v13; neither proves an upstream SDK release identity.
The runner pins these exact Git blob IDs for baseline and fixed sources:

| SDK file under `4D Plugin API/` | Git blob |
|---|---|
| `4DPluginAPI.c` | `478081f0238f6bc5393222f574bc2656f010b67b` |
| `4DPluginAPI.h` | `9054b45ffc7ff6f5b8d57529a3788a55d977b314` |
| `EntryPoints.h` | `730f2f77ff2713fb2fe4b7d6562baeedf5f664cb` |
| `Flags.h` | `10a83185333dac06a62dda61b9f9428243d1e2ee` |
| `PrivateTypes.h` | `77053dc0c0a3718d2d3b6f158371a6af78c6ae66` |
| `PublicTypes.h` | `2bb392e73e9c556c274fe0a75b3a6b83e310b77c` |
| `4DPluginAPI.def` | `996ecc4ab11ad165de9192eba2e7478db141c757` |

## Complete native inventory and reachability

`git ls-files` identifies one SDK C implementation, one plugin dispatcher C++
implementation, and all 16 wrapper C++ files with matching headers. Both
`system-volume.vcxproj` (Debug/Release, Win32/x64) and the Xcode Sources phase
list all 18 implementations. None of those wrapper implementations is
uncompiled; many methods are dormant because the dispatcher never calls them.
`C_TYPES.h` supplies string/vector aliases and platform includes, not an
additional implementation. `4D Plugin_Prefix.pch` imports Carbon.
`4DPlugin.h`, SDK headers and `.def` complete the native header/export surface.
The only `.def` export is `FourDPackex`, which invokes actual `PluginMain`
and `CommandDispatcher`. There are no additional tracked native source trees.

| Implementation in `Classes/` | Storage and reviewed ownership surface |
|---|---|
| `ARRAY_BOOLEAN` | Owns heap `CboolArray`; numeric bit-array import/export |
| `ARRAY_DATE` | Owns heap `CDateArray`; copies value-type `C_DATE` elements |
| `ARRAY_INTEGER` | Owns heap `CShortArray`; numeric import/export |
| `ARRAY_LONGINT` | Owns heap `CIntArray`; numeric import/export |
| `ARRAY_REAL` | Owns heap `CDoubleArray`; numeric import/export |
| `ARRAY_TEXT` | Owns heap string vector; imports independent string copies; exports new allocations by array transfer; UTF-8/UTF-16 and synthetic URL/path methods reviewed |
| `ARRAY_TIME` | Owns heap `CIntArray`; numeric import/export |
| `C_BLOB` / internal `CBytes` | Wrapper owns heap `CBytes`; internal byte vector is a value; handle import/export/return and hex/base64 conversions reviewed |
| `C_DATE` | Inline year/month/day; copied into `ARRAY_DATE`, remains copyable |
| `C_INTEGER` | Inline short; no string owner |
| `C_LONGINT` | Inline integer; exported mute commands use this wrapper |
| `C_PICTURE` / internal `CPicture` | Wrapper owns heap `CPicture`; internal helper owns retained picture, heap `CBytes`, format map; all command callbacks, byte creation, borrowed input/output, retained returns and Objective-C adapters reviewed |
| `C_POINTER` | Borrowed engine pointer; reference metadata construction previously fetched an owning value |
| `C_REAL` | Inline double; exported volume commands use this wrapper |
| `C_TEXT` | Owns heap UTF-16 string; already noncopyable at baseline; scalar parameter/return use content setter; all conversion/path/URL adapters reviewed |
| `C_TIME` | Inline seconds; no string owner |

Both source and bundled resource `manifest.json` declare exactly four commands:
`AUDIO SET VOLUME(&8)`, `AUDIO Get volume:8`, `AUDIO SET MUTE(&L)`,
`AUDIO Get mute:L`. Neither declares text scalars or arrays.
Both `constants.xlf` copies contain an empty constant group.
The dispatcher performs CoreAudio operations on macOS and COM endpoint-volume
operations on Windows; it uses only `C_REAL` and `C_LONGINT`. The ownership
defects fixed here are compiled helper defects, not demonstrated exported
audio-command crashes. No production wrapper-copy call is identified; the
copy defects are latent capabilities reproduced by explicit copy/destruction.
No object/collection value kinds or property/collection getters are exposed
by this SDK or these wrappers (dialog UI "Object" names are unrelated).
Objects/collections are therefore reported as not applicable, not simulated.

## Source-derived contracts and fixes

SDK line numbers below refer to the immutable baseline:

| Surface | Exact contract and treatment |
|---|---|
| `PA_CreateVariable`, 3523 onward | String initialization creates an empty owned string; picture initialization creates an empty owned picture. Scalar setters overwrite shallowly and do not release those initial owners. String callback arguments now use `PA_SetUnistring` to update content. Picture arguments immediately receiving another picture now start Undefined. Output picture arguments still start with an owned empty picture. |
| `PA_SetStringVariable`, 3826 | Shallow descriptor assignment. `fromFileAtPath` previously disposed the same path both directly and by argument cleanup. The argument now owns one content-set string and is cleared once. |
| `PA_GetPictureVariable`, 3730 | Returned picture belongs to the argument. `fromFileAtPath` calls retaining `setPicture` before clearing its output argument. |
| `PA_DuplicatePicture`, 447 | `retainOnly=1` forwards engine mode zero, retaining the same picture. `setBytes` now releases its creation owner after `setPicture` retains. Parameter/return copies keep their independent retain. |
| `PA_ExecuteCommandByID`, 5257 | Success supplies an owning returned variable. A shared local helper captures last error immediately, clears successful returns, and never normalizes, adopts or clears unspecified failed returns. Handle-allocation failure produces raw `fType=0`, which `PA_GetVariableKind` normalizes as text; kind checks are not a success predicate. |
| `PA_ConvertCharsetToCharset`, 4776 | Returns bytes written, excluding terminator; capacities and source lengths are bytes. `CPicture::createCopy` now divides the UTF-16 byte result by `sizeof(PA_Unichar)` for the `CUTF16String` unit-count constructor. |
| `PA_GetPointerValue`, 2035 | Returns an owning value (header explicitly requires clear), not reference metadata. `C_POINTER` now uses existing `PA_SetVariableOrFieldReference`, 3923, for valid variable pointers, preserving the SDK's exact scope/tag/index/name mapping. Field-reference behavior is unchanged. |
| Heap owner wrappers | Ten additional owner types now delete copy construction/assignment, following local `C_TEXT` prior art. `CBytes` and `C_DATE` remain copyable values. All actual consumers compile with these restrictions. |
| `PA_SetStringInArray`, 4444 | Disposes old element then transfers by shallow assignment only for Unicode array, nonnull data/descriptor and `0 <= index <= count`. Index zero is valid. Existing wrapper transfer cleanup is correct on successful allocation/resize and is unchanged. Rejected SDK assignments retain caller ownership. |
| `PA_GetStringInArray` | Borrowed element alias; `ARRAY_TEXT` copies into its own strings before source mutation, replacement, shrink or clear. |
| `C_TEXT` scalar output | Uses engine content setter, not descriptor transfer. Source mutation and destruction do not invalidate independently content-set outputs in the shim. No blanket disposal removal was applied. |

## Safety envelope and deferred contracts

All workflow files, release helpers, pre-push hook, native project memberships,
build phases and scheme actions were inspected before validation or push.
Existing `Bump version` and `Build` workflows are manual-only. Build performs
binary cleanup, native builds, signing, notarization, packaging and publication;
it is never dispatched. Native projects write into the tracked plugin bundle;
neither project is built. No existing workflow, project, manifest, SDK/ABI header,
prebuilt bundle or version metadata is changed.

The new workflow only pushes this exact audit branch or `master`, only for
relevant paths, with read-only permissions on `macos-latest`. It runs the
unsigned test executable, never packaging or uploading an artifact.
The force-included test header redirects CoreAudio's four called entry points,
logging, URL/path operations and image classes before compiling any actual
source. Audio objects, selectors, scope, element and argument sizes are checked
against synthetic values. `nm -u` import gating runs before any executable
dispatch; real audio/device, COM, URL/file/UI execution boundaries are rejected.
Unexpected engine selectors and synthetic audio selectors terminate the test.
Pure in-memory Foundation/CF string conversion is allowed.
The dispatcher case executes all four actual macOS commands and the existing
low-volume branch: exactly 23 synthetic audio calls, zero real audio calls.

There are no new worker exceptions or error-reporting channels. Existing
picture-command failure behavior is deliberately retained: initialized
arguments still produce the old empty/original-picture or zero-size outcomes,
and failures still lack a caller-visible diagnostic. These are legacy gaps,
not advertised successful operations or a newly invented success fallback.
Failure tests assert that unchanged behavior and do not clear failed payloads.
Synthetic failures leave arguments unchanged; arbitrary engine argument
mutation on failure is not certified.

SDK allocation/resize safety is not established by passing success tests.
The resize-failure probe demonstrates that logical element count changes even
when the physical handle resize fails. The shim repairs only its synthetic
descriptor for cleanup. Wrapper OOM/failure preservation is deferred because
the SDK contract is insufficient for a surgical, error-channel-preserving fix.
Picture creation/retention and charset-conversion failures likewise remain
unverified; unspecified failed creation values must not be assumed owned.
Invalid pointer scopes are outside the SDK reference constructor's valid
0..3 domain; no invented metadata or new scope-error behavior is added.

Successful `PA_GetPictureData` metadata ownership is not stated by this exact
SDK. `_GetCount` and `loadImageOfType` successful metadata cleanup is deferred,
not certified leak-free. Tests supply a terminating enumeration error with
poisoned metadata, never a conveniently chosen successful ownership contract.
Existing `CBytes` null/empty cache semantics, embedded-NUL truncation in
null-terminated output APIs, and unrelated text-array conversion/index behavior
are not broadened into this ownership fix.

## Reproducible evidence

Run `PYTHONDONTWRITEBYTECODE=1 python3 .github/tests/test_ownership.py -v`.
To compare, extract only actual sources with
`git archive 207883840432cdad1ddf915b036e688d2936ff01 '4D Plugin API' 4DPlugin.cpp 4DPlugin.h`
into an isolated directory and set `OWNERSHIP_SOURCE_ROOT` to it while running
the **same final harness from this checkout**. No second checkout is needed.
Only the selected actual source root changes; host/platform/runner files do not.

Final harness SHA-256 identities used for both local runs:

| File in `.github/tests/` | SHA-256 |
|---|---|
| `ownership_host.mm` | `4870d239d1b783104142607b58c589425cb7a2cc5849a43d894a1dfad402ca05` |
| `ownership_platform.h` | `dc6cdcbc5fc288b3290ae7de0d5dff1a8c95a5d3143265b4673db1bb205941dc` |
| `test_ownership.py` | `44e69a9f01ddb44b9c53c12f1948a3fc50f0d0f68b94a3b9c9ed1a7981aa52d3` |

Local provenance: macOS 26.7 arm64, Apple clang 21.0.0
(`clang-2100.1.1.101`), C++14 Objective-C++, ASan and UBSan.
Alignment sanitizer alone is disabled for legacy packed engine structures;
other undefined-behavior checks halt on error. LeakSanitizer is disabled on
macOS; explicit allocation and picture-reference ledgers assert final zero
strings, pictures and handles for every passing leaf.

| Named leaf group | Cases | Baseline pass/fail | Fixed pass/fail |
|---|---:|---:|---:|
| Path/copy exact ASCII, BMP, supplementary, empty; allocated/null engine empties | 16 | 0/16 | 16/0 |
| Byte creation repeated 20 times, gray, size, thumbnail; retained outputs | 4 | 0/4 | 4/0 |
| Array copy/churn/replacement/shrink/clear/Undefined creation/rejections/index zero | 2 | 2/0 | 2/0 |
| Scalar copy, mutation, output/return, destruction; four texts and both empties | 8 | 8/0 | 8/0 |
| Five picture callbacks, handle/command failure, poison/raw-kind0 payloads | 20 | 0/20 | 20/0 |
| Exact SDK failure predicates | 1 | 1/0 | 1/0 |
| Five callback returns with Undefined/string/picture result kinds | 15 | 0/15 | 15/0 |
| Variable reference all four valid scopes | 1 | 0/1 | 1/0 |
| Field reference metadata | 1 | 1/0 | 1/0 |
| Named actual owner-copy probes / fixed traits and two value-type controls | 12 | 2/10 | 12/0 |
| Actual macOS dispatcher with synthetic audio | 1 | 1/0 | 1/0 |
| **Total (11 unittest methods)** | **81** | **15/66** | **81/0** |

Baseline named failures include `duplicate/unowned string disposal` in path
callbacks, `unbalanced string allocations` and `unbalanced picture references`,
and `reference construction fetched an owning value`. ASCII `createCopy`
with each empty representation produces ASan heap-buffer-overflow (2 leaves).
Nine explicit owner-copy leaves produce ASan heap-use-after-free; `CPicture`
copy instead hits the picture ledger's duplicate-disposal assertion first.
The preexisting noncopyable `C_TEXT` and value controls pass.
UTF-16 content/units/terminators and converter capacity bytes are asserted;
BMP/supplementary baseline copy leaves fail later owner balances rather than
being mislabeled as sanitizer overreads.

This is synthetic ownership evidence, not a Windows native ABI compile,
actual Windows branch execution, 4D host crash reproduction, real picture
conversion or real audio/device test. No 4D database/private user files were
opened. All 16 actual wrappers, the complete unchanged SDK and actual macOS
dispatcher are compiled together; macOS evidence does not certify Windows.
