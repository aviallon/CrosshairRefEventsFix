CrosshairRefEventsFix - Skyrim AE 1.7.104
=========================================

Fixes the bug where continual CrosshairRef events are sent to the registered
Papyrus scripts while the activate/use key is disabled via
DisablePlayerControls, which overloads the script engine.

Upstream: https://github.com/yeahhowaboutnooo/CrosshairRefEventsFix (MIT)

Why a rebuild
-------------
Upstream was last built in January 2023 for the then-current AE runtime. Its
DLL is linked against a CommonLibSSE-NG revision that cannot read the Address
Library format used from Skyrim 1.7.99 on
(Data/SKSE/Plugins/versionlib-1-7-104-0.bin, format 5), so on 1.7.104 it
resolves no addresses. This build uses a CommonLibSSE-NG revision whose
include/REL/IDDB.h defines Format::SSEv5, and is otherwise upstream source.

What it patches
---------------
PlayerCharacter::PickCrosshairReference (Address Library id 40620). While the
activate handler is disabled the engine clears CrosshairPickData and sets
PlayerFlags::shouldUpdateCrosshair, which keeps re-firing the crosshair update
and the CrosshairRef event. The plugin removes the "activate handler disabled"
branch, exactly like upstream, but locates it by the reference to
PlayerControls::GetSingleton rather than by a hard-coded per-runtime byte
offset. If that reference is not found (a future game version changed the
function) the plugin logs an error and patches nothing instead of writing to
the wrong bytes.

Install
-------
Amethyst: Mods -> Install mod from file... and pick this archive (it is a
FOMOD). Mod Organizer 2 / Vortex: install as usual; the FOMOD has no choices.

Requirements: Skyrim Special Edition / Anniversary Edition 1.7.104 and SKSE64
2.2.6+. No other mod is needed. The plugin only uses the Address Library data
that SKSE/Address Library already provides.

The plugin writes SKSE/Plugins/CrosshairRefEventsFix.log next to the other SKSE
logs. A working install logs:

    CrosshairRefEventsFix v0.0.2 (Skyrim AE 1.7.104) loading
    fixed disablePlayerControls overloading the script engine
    NOPed the activate-handler branch at 0x... (+0x65 from PickCrosshairReference)

The "+0x65" is the expected offset on 1.7.104; it is reported, not assumed.
