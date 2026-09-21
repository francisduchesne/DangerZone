#pragma once

#include "PluginProcessor.h"

class DangerZoneAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit DangerZoneAudioProcessorEditor (DangerZoneAudioProcessor&);
    ~DangerZoneAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    DangerZoneAudioProcessor& processor;
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    juce::Label footer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DangerZoneAudioProcessorEditor)
};
