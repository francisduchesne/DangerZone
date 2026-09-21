#pragma once

namespace dz
{

// Insert-slot ids. These name Arcades / Fashion Victim modules.
// Danger Zone does not link or host FVS_Host. See FvsInsert for the stub DSP
// and docs/MIXER.md for which ids are fixed aux destinations.
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
