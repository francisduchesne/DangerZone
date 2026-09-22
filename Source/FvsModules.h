#pragma once

namespace dz
{

// Provisional insert-picker ids. The exact Fashion Victim Single menu is TBD.
// These names keep the slot a real APVTS choice until that menu is locked.
// Danger Zone does not link or host FVS_Host. Aux 1 is Mover and Aux 2 is
// Spatializer (two reverb engines) in FxChain — see docs/MIXER.md.
enum class FvsModule : int
{
    none = 0,
    equalizer,
    amplifier,
    shifter,
    widener,
    pusher,
    toughener,
    mover,
    spatializer,
    count
};

inline constexpr const char* kFvsModuleNames[] = {
    "None",
    "Equalizer",
    "Amplifier",
    "Shifter",
    "Widener",
    "Pusher",
    "Toughener",
    "Mover",
    "Spatializer",
};

static_assert ((int) FvsModule::count
                   == (int) (sizeof (kFvsModuleNames) / sizeof (kFvsModuleNames[0])),
               "FVS module names must match FvsModule");

} // namespace dz
