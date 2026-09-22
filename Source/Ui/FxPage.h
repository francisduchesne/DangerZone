#pragma once

#include "PluginProcessor.h"

namespace dz
{

// Aux engines and feel. Per-voice fader, pan, tune, inserts, and sends live
// on the Mixer tab so each APVTS id has one control.
class FxPage : public juce::Component
{
public:
    explicit FxPage (DangerZoneAudioProcessor&);
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    static void styleSlider (juce::Slider& slider);

    DangerZoneAudioProcessor& processor;
    juce::Label hint;

    juce::GroupComponent moverGroup { {}, "Aux 1  ·  Mover (delay)" };
    juce::GroupComponent spatGroup { {}, "Aux 2  ·  Spatializer (two reverbs)" };
    juce::GroupComponent feelGroup { {}, "Feel" };
    juce::Slider moverTime, moverFeedback, moverReturn;
    juce::Slider spatSizeA, spatSizeB, spatLevelA, spatLevelB, spatReturn;
    juce::Slider timingJitter, pitchJitter;
    juce::ComboBox variantMode;
    juce::ToggleButton midiSync { "MIDI clock sync (stored, not following yet)" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAttachment> aMoverTime, aMoverFb, aMoverRet;
    std::unique_ptr<SliderAttachment> aSizeA, aSizeB, aLevA, aLevB, aSpatRet;
    std::unique_ptr<SliderAttachment> aTiming, aPitch;
    std::unique_ptr<ComboAttachment> aVariant;
    std::unique_ptr<ButtonAttachment> aMidi;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxPage)
};

} // namespace dz
