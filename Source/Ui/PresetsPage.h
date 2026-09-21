#pragma once

#include "PluginProcessor.h"

namespace dz
{

class PresetsPage : public juce::Component,
                    private juce::Timer
{
public:
    explicit PresetsPage (DangerZoneAudioProcessor&);
    ~PresetsPage() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshList();

    DangerZoneAudioProcessor& processor;
    juce::ComboBox presets;
    juce::TextButton load { "Load" };
    juce::TextButton save { "Save user preset" };
    juce::TextEditor name;
    juce::Label status;
    juce::StringArray listed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsPage)
};

} // namespace dz
