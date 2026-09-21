-- CrosshairRefEventsFix, ported to Skyrim AE 1.7.104.
--
-- Build system: xmake + CommonLibSSE-NG as a git submodule, the same shape as
-- the other local SKSE ports (SexLabpp). The upstream project used CMake +
-- vcpkg + the SkyrimScripting wrapper, which pinned a CommonLibSSE-NG that has
-- no reader for Address Library format 5 (versionlib-1-7-104-0.bin), so its
-- DLL cannot resolve anything on 1.7.104.
--
-- The submodule is pinned (gitlink) to the revision whose include/REL/IDDB.h
-- defines Format::SSEv5. CI asserts that before building.

set_xmakever("3.0.0")

-- Pulls in the `commonlibsse-ng` target and the `commonlibsse-ng.plugin` rule,
-- which generates the SKSEPluginInfo metadata and installs the DLL under
-- SKSE/Plugins.
includes("lib/CommonLibSSE-NG/xmake.lua")

set_project("CrosshairRefEventsFix")
set_version("0.0.2")
set_languages("c++23")
set_license("MIT")

set_allowedplats("windows")
set_allowedarchs("x64")
set_defaultplat("windows")
set_defaultarchs("x64")

add_rules("mode.debug", "mode.release")
set_runtimes("MD")
set_warnings("allextra")

if is_mode("debug") then
    add_defines("DEBUG")
    set_optimize("none")
elseif is_mode("release") then
    add_defines("NDEBUG")
    set_optimize("fastest")
    set_symbols("debug")
end

target("CrosshairRefEventsFix")
    add_deps("commonlibsse-ng")
    add_rules("commonlibsse-ng.plugin", {
        name = "CrosshairRefEventsFix",
        author = "yeahhowaboutnooo",
        description = "Fixes continual CrosshairRef events while player controls are disabled.",
    })

    set_pcxxheader("src/PCH.h")
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
target_end()
