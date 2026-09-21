#include "Mixer.h"

#include <JuceHeader.h>

#include <cmath>

namespace dz
{

StripGain computeStrip (const ChannelStrip& strip, bool anySolo) noexcept
{
    StripGain gain;
    if (strip.mute)
        return gain;
    if (anySolo && ! strip.solo)
        return gain;

    const float pan = juce::jlimit (-1.0f, 1.0f, strip.pan);
    const float angle = (pan + 1.0f) * 0.5f * juce::MathConstants<float>::halfPi;
    const float level = juce::jlimit (0.0f, 1.0f, strip.level);
    gain.audible = level > 0.0001f || strip.aux1 > 0.0001f || strip.aux2 > 0.0001f;
    gain.left = std::cos (angle) * level;
    gain.right = std::sin (angle) * level;
    gain.aux1 = juce::jlimit (0.0f, 1.0f, strip.aux1);
    gain.aux2 = juce::jlimit (0.0f, 1.0f, strip.aux2);
    return gain;
}

} // namespace dz
