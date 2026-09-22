#pragma once

namespace dz
{

// One Linn (or mythical) channel after the insert slots.
// Aux 1 feeds Mover. Aux 2 feeds Spatializer. See docs/MIXER.md.
struct ChannelStrip
{
    float level = 0.7f;
    float pan = 0.0f;
    bool mute = false;
    bool solo = false;
    float aux1 = 0.0f;
    float aux2 = 0.0f;
};

struct StripGain
{
    bool audible = false;
    float left = 0.0f;
    float right = 0.0f;
    float aux1 = 0.0f;
    float aux2 = 0.0f;
};

StripGain computeStrip (const ChannelStrip& strip, bool anySolo) noexcept;

} // namespace dz
