#pragma once

#include "PluginProcessor.h"

namespace dz
{

class MixerPage : public juce::Component
{
public:
    explicit MixerPage (DangerZoneAudioProcessor&);
    ~MixerPage() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    struct Strip : public juce::Component
    {
        juce::Label name;
        juce::Slider level, pan, aux1, aux2;
        juce::ComboBox ins1, ins2;
        juce::ToggleButton bypass1 { "B" }, bypass2 { "B" }, mute { "M" }, solo { "S" };

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
        using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
        std::unique_ptr<SliderAttachment> aLevel, aPan, aAux1, aAux2;
        std::unique_ptr<ComboAttachment> aIns1, aIns2;
        std::unique_ptr<ButtonAttachment> aBypass1, aBypass2, aMute, aSolo;

        void resized() override
        {
            auto area = getLocalBounds();
            name.setBounds (area.removeFromLeft (108));
            level.setBounds (area.removeFromLeft (150));
            pan.setBounds (area.removeFromLeft (120));
            aux1.setBounds (area.removeFromLeft (110));
            aux2.setBounds (area.removeFromLeft (110));
            ins1.setBounds (area.removeFromLeft (116).reduced (0, 3));
            bypass1.setBounds (area.removeFromLeft (28).reduced (1));
            ins2.setBounds (area.removeFromLeft (116).reduced (0, 3));
            bypass2.setBounds (area.removeFromLeft (28).reduced (1));
            mute.setBounds (area.removeFromLeft (32).reduced (1));
            solo.setBounds (area.removeFromLeft (32).reduced (1));
        }
    };

    static void styleSlider (juce::Slider& slider);
    static void fillChoices (juce::ComboBox& box, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId);

    DangerZoneAudioProcessor& processor;
    juce::Label masterLabel, header;
    juce::Slider masterLevel;
    juce::ComboBox masterIns1, masterIns2;
    juce::ToggleButton masterBypass1 { "B" }, masterBypass2 { "B" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aMaster;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> aMasterIns1, aMasterIns2;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aMasterBypass1, aMasterBypass2;

    juce::Component content;
    juce::OwnedArray<Strip> strips;
    juce::Viewport viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerPage)
};

} // namespace dz
