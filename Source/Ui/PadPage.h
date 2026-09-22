#pragma once

#include "PluginProcessor.h"

namespace dz
{

class PadPage : public juce::Component
{
public:
    PadPage (DangerZoneAudioProcessor& processor, Bank bank);
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void refreshLabels();

    DangerZoneAudioProcessor& processor;
    Bank bank;
    juce::OwnedArray<juce::TextButton> pads;
    std::vector<int> voices;
    juce::TextButton reload { "Reload samples" };
    juce::Label hint;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PadPage)
};

} // namespace dz
