#pragma once

#include "PluginProcessor.h"

namespace dz
{

// One channel strip per voice (Linn and Mythical). Column widths are shared
// with the header so the captions stay aligned with the controls.
class MixerPage : public juce::Component
{
public:
    explicit MixerPage (DangerZoneAudioProcessor&);
    ~MixerPage() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

    static constexpr int kNameW = 112;
    static constexpr int kLevelW = 150;
    static constexpr int kPanW = 124;
    static constexpr int kTuneW = 108;
    static constexpr int kToggleW = 34;
    static constexpr int kComboW = 116;
    static constexpr int kBypassW = 30;
    static constexpr int kAmountW = 104;
    static constexpr int kAuxW = 128;
    static constexpr int kLineH = 26;
    static constexpr int kStripH = 54;
    static constexpr int kSectionH = 22;

    static int stripWidth()
    {
        const int top = kNameW + kLevelW + kPanW + kTuneW + kToggleW + kToggleW;
        const int bottom = kNameW + kComboW + kBypassW + kAmountW + kComboW + kBypassW + kAmountW + kAuxW + kAuxW;
        return juce::jmax (top, bottom);
    }

private:
    struct Strip : public juce::Component
    {
        int voice = 0;
        Bank bank = Bank::linn;

        juce::Label name;
        juce::Slider level, pan, tune, aux1, aux2, amt1, amt2;
        juce::ComboBox ins1, ins2;
        juce::ToggleButton bypass1 { "B" }, bypass2 { "B" }, mute { "M" }, solo { "S" };

        using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
        using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
        using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
        std::unique_ptr<SliderAttachment> aLevel, aPan, aTune, aAux1, aAux2, aAmt1, aAmt2;
        std::unique_ptr<ComboAttachment> aIns1, aIns2;
        std::unique_ptr<ButtonAttachment> aBypass1, aBypass2, aMute, aSolo;

        void paint (juce::Graphics& g) override;
        void resized() override;
    };

    struct Columns : public juce::Component
    {
        juce::Label level { {}, "Level" };
        juce::Label pan { {}, "Pan" };
        juce::Label tune { {}, "Tune" };
        juce::Label mute { {}, "M" };
        juce::Label solo { {}, "S" };
        juce::Label ins1 { {}, "Insert 1" };
        juce::Label bypass1 { {}, "B" };
        juce::Label amt1 { {}, "Amount" };
        juce::Label ins2 { {}, "Insert 2" };
        juce::Label bypass2 { {}, "B" };
        juce::Label amt2 { {}, "Amount" };
        juce::Label aux1 { {}, "Aux 1 Mover" };
        juce::Label aux2 { {}, "Aux 2 Spatializer" };

        Columns();
        void resized() override;
    };

    static void styleSlider (juce::Slider& slider);
    static void fillChoices (juce::ComboBox& box, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId);

    DangerZoneAudioProcessor& processor;
    juce::Label masterLabel { {}, "Master" };
    juce::Slider masterLevel, masterAmt1, masterAmt2;
    juce::ComboBox masterIns1, masterIns2;
    juce::ToggleButton masterBypass1 { "B" }, masterBypass2 { "B" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aMaster, aMasterAmt1, aMasterAmt2;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> aMasterIns1, aMasterIns2;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> aMasterBypass1, aMasterBypass2;

    juce::Component content;
    Columns columns;
    juce::Label linnHeading { {}, "LinnDrum" };
    juce::Label mythHeading { {}, "Mythical / era" };
    juce::OwnedArray<Strip> strips;
    juce::Viewport viewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerPage)
};

} // namespace dz
