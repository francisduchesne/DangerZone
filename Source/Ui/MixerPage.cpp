#include "MixerPage.h"

namespace dz
{

void MixerPage::styleSlider (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 16);
}

void MixerPage::fillChoices (juce::ComboBox& box, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
{
    if (auto* param = apvts.getParameter (paramId))
        box.addItemList (param->getAllValueStrings(), 1);
}

MixerPage::MixerPage (DangerZoneAudioProcessor& processorIn)
    : processor (processorIn)
{
    masterLabel.setText ("Master", juce::dontSendNotification);
    header.setText ("Voice     Level      Pan       Aux1 Mover    Aux2 Spatializer    Insert 1    Insert 2    M  S",
                    juce::dontSendNotification);
    addAndMakeVisible (masterLabel);
    addAndMakeVisible (header);

    styleSlider (masterLevel);
    addAndMakeVisible (masterLevel);
    const auto insertTip = juce::String ("FVS insert. Menu is a stub; the exact Single list is TBD.");
    fillChoices (masterIns1, processor.getApvts(), "masterIns1");
    fillChoices (masterIns2, processor.getApvts(), "masterIns2");
    masterIns1.setTooltip (insertTip);
    masterIns2.setTooltip (insertTip);
    addAndMakeVisible (masterIns1);
    addAndMakeVisible (masterIns2);
    addAndMakeVisible (masterBypass1);
    addAndMakeVisible (masterBypass2);

    auto& apvts = processor.getApvts();
    aMaster = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "masterLevel", masterLevel);
    aMasterIns1 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "masterIns1", masterIns1);
    aMasterIns2 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "masterIns2", masterIns2);
    aMasterBypass1 = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "masterIns1Bypass", masterBypass1);
    aMasterBypass2 = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "masterIns2Bypass", masterBypass2);

    for (int voice = 0; voice < kVoiceCount; ++voice)
    {
        auto* strip = strips.add (new Strip());
        strip->name.setText (kVoices[voice].label, juce::dontSendNotification);
        styleSlider (strip->level);
        styleSlider (strip->pan);
        styleSlider (strip->aux1);
        styleSlider (strip->aux2);
        strip->aux1.setTooltip ("Aux 1 send. Pre-filled destination: Mover (delay).");
        strip->aux2.setTooltip ("Aux 2 send. Pre-filled destination: Spatializer, two reverb engines.");
        fillChoices (strip->ins1, apvts, voiceParam (voice, "ins1"));
        fillChoices (strip->ins2, apvts, voiceParam (voice, "ins2"));
        strip->ins1.setTooltip (insertTip);
        strip->ins2.setTooltip (insertTip);
        content.addAndMakeVisible (strip);

        strip->aLevel = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "level"), strip->level);
        strip->aPan = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "pan"), strip->pan);
        strip->aAux1 = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "aux1"), strip->aux1);
        strip->aAux2 = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "aux2"), strip->aux2);
        strip->aIns1 = std::make_unique<Strip::ComboAttachment> (apvts, voiceParam (voice, "ins1"), strip->ins1);
        strip->aIns2 = std::make_unique<Strip::ComboAttachment> (apvts, voiceParam (voice, "ins2"), strip->ins2);
        strip->aBypass1 = std::make_unique<Strip::ButtonAttachment> (apvts, voiceParam (voice, "ins1Bypass"), strip->bypass1);
        strip->aBypass2 = std::make_unique<Strip::ButtonAttachment> (apvts, voiceParam (voice, "ins2Bypass"), strip->bypass2);
        strip->aMute = std::make_unique<Strip::ButtonAttachment> (apvts, voiceParam (voice, "mute"), strip->mute);
        strip->aSolo = std::make_unique<Strip::ButtonAttachment> (apvts, voiceParam (voice, "solo"), strip->solo);
    }

    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, true);
    addAndMakeVisible (viewport);
}

void MixerPage::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MixerPage::resized()
{
    auto area = getLocalBounds().reduced (8);
    auto master = area.removeFromTop (32);
    masterLabel.setBounds (master.removeFromLeft (64));
    masterLevel.setBounds (master.removeFromLeft (220));
    masterIns1.setBounds (master.removeFromLeft (140).reduced (4, 4));
    masterBypass1.setBounds (master.removeFromLeft (32).reduced (2));
    masterIns2.setBounds (master.removeFromLeft (140).reduced (4, 4));
    masterBypass2.setBounds (master.removeFromLeft (32).reduced (2));

    header.setBounds (area.removeFromTop (18));
    viewport.setBounds (area);

    const int rowHeight = 28;
    const int width = juce::jmax (980, viewport.getWidth() - 16);
    content.setSize (width, juce::jmax (1, strips.size() * rowHeight));
    for (int i = 0; i < strips.size(); ++i)
        strips[i]->setBounds (0, i * rowHeight, width, rowHeight);
}

} // namespace dz
