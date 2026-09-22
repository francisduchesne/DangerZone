#pragma once

#include <JuceHeader.h>

#include <cstring>

namespace dz
{

// Classic LinnDrum roster plus the mythical / era page.
// Folder names under Resources/Samples match `id` exactly.
enum class Bank
{
    linn,
    mythical
};

struct VoiceSpec
{
    const char* id;
    const char* label;
    Bank bank;
    int midiNote;
    int targetVariants;
    int chokeGroup; // 0 = overlap freely. 1 = hi-hat (new hit chokes the group).
};

inline constexpr VoiceSpec kVoices[] = {
    { "Kick", "Kick", Bank::linn, 36, 6, 0 },
    { "Snare", "Snare", Bank::linn, 38, 6, 0 },
    { "Sidestick", "Sidestick", Bank::linn, 37, 2, 0 },
    { "Rim", "Rim", Bank::linn, 34, 2, 0 },
    { "HatClosed", "Hat Closed", Bank::linn, 42, 2, 1 },
    { "HatOpen", "Hat Open", Bank::linn, 46, 2, 1 },
    { "Cabasa", "Cabasa", Bank::linn, 69, 2, 0 },
    { "Tambourine", "Tambourine", Bank::linn, 54, 2, 0 },
    { "CongaLo", "Conga Lo", Bank::linn, 64, 2, 0 },
    { "CongaHi", "Conga Hi", Bank::linn, 63, 2, 0 },
    { "Cowbell", "Cowbell", Bank::linn, 56, 2, 0 },
    { "Claps", "Claps", Bank::linn, 39, 2, 0 },
    { "TomLo", "Tom Lo", Bank::linn, 45, 2, 0 },
    { "TomMid", "Tom Mid", Bank::linn, 47, 2, 0 },
    { "TomHi", "Tom Hi", Bank::linn, 50, 2, 0 },
    { "Ride", "Ride", Bank::linn, 51, 2, 0 },
    { "Crash", "Crash", Bank::linn, 49, 2, 0 },
    { "MythTomA", "Myth Tom A", Bank::mythical, 72, 6, 0 },
    { "MythTomB", "Myth Tom B", Bank::mythical, 73, 6, 0 },
    { "EraKick", "Era Kick", Bank::mythical, 74, 6, 0 },
    { "EraSnare", "Era Snare", Bank::mythical, 75, 6, 0 },
    { "EraHat", "Era Hat", Bank::mythical, 76, 2, 0 },
    { "EraPerc", "Era Perc", Bank::mythical, 77, 2, 0 },
};

inline constexpr int kVoiceCount = (int) (sizeof (kVoices) / sizeof (kVoices[0]));
inline constexpr int kMaxSteps = 16;

constexpr int countBank (Bank bank)
{
    int n = 0;
    for (int i = 0; i < kVoiceCount; ++i)
        if (kVoices[i].bank == bank)
            ++n;
    return n;
}

inline constexpr int kLinnCount = countBank (Bank::linn);
inline constexpr int kMythCount = countBank (Bank::mythical);

inline int indexOfId (const char* id)
{
    for (int i = 0; i < kVoiceCount; ++i)
        if (std::strcmp (kVoices[i].id, id) == 0)
            return i;
    return -1;
}

inline int indexOfNote (int midiNote)
{
    for (int i = 0; i < kVoiceCount; ++i)
        if (kVoices[i].midiNote == midiNote)
            return i;
    return -1;
}

inline juce::String voiceParam (int index, const char* leaf)
{
    jassert (index >= 0 && index < kVoiceCount);
    return juce::String (kVoices[index].id).toLowerCase() + "_" + leaf;
}

inline float defaultLevelFor (int index)
{
    switch (index)
    {
        case 0: // Kick
        case 1: // Snare
            return 0.85f;
        case 4: // HatClosed
        case 5: // HatOpen
            return 0.42f;
        case 15: // Ride
        case 16: // Crash
            return 0.55f;
        default:
            return 0.70f;
    }
}

} // namespace dz
