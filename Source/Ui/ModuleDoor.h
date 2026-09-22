#pragma once

#include <JuceHeader.h>

namespace dz
{

// Floating module panels parented to the Danger Zone editor.
// voice < 0 is the master bus. slot is 0 or 1.
// Closing a panel hides the editor only. Choosing None unloads the insert
// (the APVTS choice) and closes the panel. See docs/FVS_HOSTING.md.
//
// FVS Singles do not expose createModuleEditor() yet, so each panel paints
// a module-only face at the host module-hole aspect (452.6 x 790.5). There
// is no input or output strip. When a Single can return that component,
// parent it in the face and drop the stub painting. If only a full host
// editor exists, crop it to the module hole. Do not link FVS_Host.
class ModuleDoorHost : public juce::Component,
                       private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit ModuleDoorHost (juce::AudioProcessorValueTreeState&);
    ~ModuleDoorHost() override;

    void open (int voice, int slot);
    void resized() override;

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void applyModuleChange (const juce::String& parameterID, int moduleIndex);
    void requestClose (int voice, int slot);
    void closeNow (int voice, int slot);

    struct Door;
    juce::AudioProcessorValueTreeState& apvts;
    juce::OwnedArray<Door> doors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleDoorHost)
};

} // namespace dz
