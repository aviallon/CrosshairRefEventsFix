#include "PCH.h"

#include <REL/Relocation.h>

#include <array>
#include <cstring>

// CrosshairRefEventsFix - port to Skyrim AE 1.7.104.
//
// PlayerCharacter::PickCrosshairReference runs every frame. When the
// activate/use key is disabled (DisablePlayerControls), the engine takes the
// function's "reset" path: it clears CrosshairPickData and sets
// PlayerFlags::shouldUpdateCrosshair. That keeps forcing the crosshair UI -
// and the CrosshairRef event dispatched to Papyrus through SKSE - to fire
// again, which overloads the script engine while the player cannot activate
// anything.
//
// The engine already tests the activate handler. The second boolean check at
// the top of the function is
//     auto* ah = PlayerControls::GetSingleton()->GetActivateHandler();
//     if (!ah || !ah->disabled) { ...proceed... }
// and the branch removed here is the "disabled -> reset" edge.
//
// Address Library IDs:
//   39534 / 40620    PlayerCharacter::PickCrosshairReference
//   514706 / 400864  RE::PlayerControls singleton pointer (the same ID
//                    CommonLibSSE-NG's PlayerControls::GetSingleton uses)

namespace
{
	constexpr std::uint64_t kPickCrosshairReferenceSE = 39534;
	constexpr std::uint64_t kPickCrosshairReferenceAE = 40620;
	constexpr std::uint64_t kPlayerControlsSingletonSE = 514706;
	constexpr std::uint64_t kPlayerControlsSingletonAE = 400864;

	constexpr std::size_t kBranchSize = 6;  // jcc rel32
	constexpr std::size_t kScanLimit = 0x200;

	void SetupLog()
	{
		const auto path = SKSE::log::log_directory();
		if (!path) {
			return;
		}

		const auto logFile = *path / "CrosshairRefEventsFix.log";
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFile.string(), true);
		auto log = std::make_shared<spdlog::logger>("CrosshairRefEventsFix", std::move(sink));
		log->set_level(spdlog::level::trace);
		log->flush_on(spdlog::level::trace);
		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
	}

	// Locate the "activate handler disabled" conditional branch inside
	// PickCrosshairReference without relying on a per-runtime byte offset. The
	// only code in the function that loads PlayerControls::GetSingleton is the
	// disabled check, and it has a fixed shape:
	//     mov  rcx, [rip+rel]   ; 48 8B 0D <rel32>  -> &PlayerControls::GetSingleton()
	//     call <check>          ; E8 <rel32>
	//     test al, al           ; 84 C0
	//     je   <reset>          ; 0F 84 <rel32>
	// Returns the address of the jcc, or 0 when the shape is not found.
	std::uintptr_t FindDisabledCheckBranch(const std::uint8_t* a_func, std::uintptr_t a_playerControlsSingleton)
	{
		for (std::size_t i = 0; i + 20 <= kScanLimit; ++i) {
			if (a_func[i] != 0x48 || a_func[i + 1] != 0x8B || a_func[i + 2] != 0x0D) {
				continue;
			}

			const auto movRel = *reinterpret_cast<const std::int32_t*>(a_func + i + 3);
			const auto movDst = reinterpret_cast<std::uintptr_t>(a_func + i + 7) + movRel;
			if (movDst != a_playerControlsSingleton) {
				continue;
			}
			if (a_func[i + 7] != 0xE8) {  // call rel32
				continue;
			}
			if (a_func[i + 12] != 0x84 || a_func[i + 13] != 0xC0) {  // test al, al
				continue;
			}
			if (a_func[i + 14] != 0x0F || (a_func[i + 15] != 0x84 && a_func[i + 15] != 0x85)) {  // je / jne rel32
				continue;
			}
			const auto jccRel = *reinterpret_cast<const std::int32_t*>(a_func + i + 16);
			if (jccRel <= 0) {  // the edge we remove jumps forward, to the reset block
				continue;
			}

			return reinterpret_cast<std::uintptr_t>(a_func + i + 14);
		}

		return 0;
	}

	void Install()
	{
		const REL::RelocationID func{ kPickCrosshairReferenceSE, kPickCrosshairReferenceAE };
		const REL::RelocationID playerControls{ kPlayerControlsSingletonSE, kPlayerControlsSingletonAE };

		const auto funcAddr = func.address();
		const auto playerControlsAddr = playerControls.address();
		if (!funcAddr || !playerControlsAddr) {
			logger::error("could not resolve PlayerCharacter::PickCrosshairReference / PlayerControls from the address library - not patching");
			return;
		}

		const auto* bytes = reinterpret_cast<const std::uint8_t*>(funcAddr);
		const auto branch = FindDisabledCheckBranch(bytes, playerControlsAddr);
		if (!branch) {
			logger::error("the activate-handler branch was not found in PickCrosshairReference (0x{:X}) - not patching", funcAddr);
			return;
		}

		// Verify the six bytes we are about to overwrite are still the branch we
		// located, then NOP them. safe_write fails (and writes nothing) if the
		// memory does not match what we expect.
		std::array<std::uint8_t, kBranchSize> expected{};
		std::memcpy(expected.data(), reinterpret_cast<const void*>(branch), kBranchSize);

		const std::array<std::uint8_t, kBranchSize> nops{ 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		if (!REL::safe_write(branch, nops.data(), nops.size(), expected.data(), expected.size())) {
			logger::error("safe_write verification failed at 0x{:X} - not patching", branch);
			return;
		}

		logger::info("fixed disablePlayerControls overloading the script engine");
		logger::info("NOPed the activate-handler branch at 0x{:X} (+0x{:X} from PickCrosshairReference)", branch, branch - funcAddr);
	}
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();

	logger::info("CrosshairRefEventsFix v0.0.2 (Skyrim AE 1.7.104) loading");
	Install();
	return true;
}
