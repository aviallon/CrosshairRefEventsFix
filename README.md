# CrosshairRefEventsFix

Fixes the bug that continual CrosshairRef Events are being sent to the
registered Papyrus scripts when the activate/use key is disabled via
DisablePlayerControls. Which would normally result in an overloaded script
engine.

supports SSE and SAE

## Port to Skyrim AE 1.7.104

This tree is an unofficial port of
[yeahhowaboutnooo/CrosshairRefEventsFix](https://github.com/yeahhowaboutnooo/CrosshairRefEventsFix)
(MIT) to Skyrim AE 1.7.104.

The upstream DLL is linked against a CommonLibSSE-NG revision that cannot read
the Address Library format used from 1.7.99 on
(`Data/SKSE/Plugins/versionlib-1-7-104-0.bin` is format 5), so on 1.7.104 it
resolves no addresses. The port is therefore the CommonLibSSE-NG bump, plus one
change to how the patch site is found.

* Build system: `xmake` + CommonLibSSE-NG as a git submodule (branch `ng`),
  pinned to the revision whose `include/REL/IDDB.h` defines `Format::SSEv5`.
* The patch site is located by its reference to `PlayerControls::GetSingleton`
  inside `PlayerCharacter::PickCrosshairReference` (Address Library id 40620),
  not by a hard-coded per-runtime byte offset. If the shape is not found the
  plugin logs an error and patches nothing.
* CI (`.github/workflows/port-windows-1.7.104.yml`) builds the DLL and packages
  a FOMOD archive (`fomod/ModuleConfig.xml` + `SKSE/Plugins/`) that Amethyst,
  MO2 and Vortex install in one click.

### Build locally (Windows)

```
git clone --recurse-submodules <this repo>
xmake f -p windows -a x64 -m release -y
xmake build CrosshairRefEventsFix
```

## Upstream requirements

- Visual Studio 2022, CMake and vcpkg were needed by the original CMake build.
  They are no longer used here; see the port section above.
