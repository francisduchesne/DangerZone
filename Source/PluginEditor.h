#pragma once

#include "PluginProcessor.h"
#include "Ui/ModuleDoor.h"

class DangerZoneAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit DangerZoneAudioProcessorEditor (DangerZoneAudioProcessor&);
    ~DangerZoneAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

    // voice < 0 opens a master insert. Closing the panel does not unload DSP.
    void openInsertDoor (int voice, int slot);

private:
    void timerCallback() override;

    DangerZoneAudioProcessor& processor;
    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    dz::ModuleDoorHost moduleDoors;
    juce::Label footer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DangerZoneAudioProcessorEditor)
};
