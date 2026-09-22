#include "MixerPage.h"

namespace dz
{

void MixerPage::styleSlider (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 44, 16);
}

void MixerPage::fillChoices (juce::ComboBox& box, juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId)
{
    if (auto* param = apvts.getParameter (paramId))
        box.addItemList (param->getAllValueStrings(), 1);
}

MixerPage::Columns::Columns()
{
    for (auto* label : { &level, &pan, &tune, &mute, &solo, &ins1, &bypass1, &amt1,
                         &ins2, &bypass2, &amt2, &aux1, &aux2 })
    {
        label->setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (*label);
    }
}

void MixerPage::Columns::resized()
{
    auto area = getLocalBounds();
    auto top = area.removeFromTop (kLineH);
    top.removeFromLeft (kNameW);
    level.setBounds (top.removeFromLeft (kLevelW).reduced (2, 0));
    pan.setBounds (top.removeFromLeft (kPanW).reduced (2, 0));
    tune.setBounds (top.removeFromLeft (kTuneW).reduced (2, 0));
    mute.setBounds (top.removeFromLeft (kToggleW).reduced (2, 0));
    solo.setBounds (top.removeFromLeft (kToggleW).reduced (2, 0));

    auto bottom = area.removeFromTop (kLineH);
    bottom.removeFromLeft (kNameW);
    ins1.setBounds (bottom.removeFromLeft (kComboW).reduced (2, 0));
    bypass1.setBounds (bottom.removeFromLeft (kBypassW).reduced (1, 0));
    amt1.setBounds (bottom.removeFromLeft (kAmountW).reduced (2, 0));
    ins2.setBounds (bottom.removeFromLeft (kComboW).reduced (2, 0));
    bypass2.setBounds (bottom.removeFromLeft (kBypassW).reduced (1, 0));
    amt2.setBounds (bottom.removeFromLeft (kAmountW).reduced (2, 0));
    aux1.setBounds (bottom.removeFromLeft (kAuxW).reduced (2, 0));
    aux2.setBounds (bottom.removeFromLeft (kAuxW).reduced (2, 0));
}

void MixerPage::Strip::paint (juce::Graphics& g)
{
    g.setColour (findColour (juce::ComboBox::outlineColourId).withAlpha (0.35f));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, (float) getWidth());
}

void MixerPage::Strip::resized()
{
    auto area = getLocalBounds();
    auto top = area.removeFromTop (kLineH);
    name.setBounds (top.removeFromLeft (kNameW).reduced (2, 0));
    level.setBounds (top.removeFromLeft (kLevelW).reduced (1, 1));
    pan.setBounds (top.removeFromLeft (kPanW).reduced (1, 1));
    tune.setBounds (top.removeFromLeft (kTuneW).reduced (1, 1));
    mute.setBounds (top.removeFromLeft (kToggleW).reduced (2, 2));
    solo.setBounds (top.removeFromLeft (kToggleW).reduced (2, 2));

    auto bottom = area.removeFromTop (kLineH);
    bottom.removeFromLeft (kNameW);
    ins1.setBounds (bottom.removeFromLeft (kComboW).reduced (2, 3));
    bypass1.setBounds (bottom.removeFromLeft (kBypassW).reduced (2, 3));
    amt1.setBounds (bottom.removeFromLeft (kAmountW).reduced (1, 1));
    ins2.setBounds (bottom.removeFromLeft (kComboW).reduced (2, 3));
    bypass2.setBounds (bottom.removeFromLeft (kBypassW).reduced (2, 3));
    amt2.setBounds (bottom.removeFromLeft (kAmountW).reduced (1, 1));
    aux1.setBounds (bottom.removeFromLeft (kAuxW).reduced (1, 1));
    aux2.setBounds (bottom.removeFromLeft (kAuxW).reduced (1, 1));
}

MixerPage::MixerPage (DangerZoneAudioProcessor& processorIn)
    : processor (processorIn)
{
    const auto insertTip = juce::String ("FVS insert. Menu is a stub; the exact Single list is TBD.");
    auto& apvts = processor.getApvts();

    addAndMakeVisible (masterLabel);
    styleSlider (masterLevel);
    styleSlider (masterAmt1);
    styleSlider (masterAmt2);
    masterLevel.setTooltip ("Master level");
    masterAmt1.setTooltip ("Master insert 1 amount. Zero stays dry until you raise it.");
    masterAmt2.setTooltip ("Master insert 2 amount. Zero stays dry until you raise it.");
    fillChoices (masterIns1, apvts, "masterIns1");
    fillChoices (masterIns2, apvts, "masterIns2");
    masterIns1.setTooltip (insertTip);
    masterIns2.setTooltip (insertTip);
    masterBypass1.setTooltip ("Bypass master insert 1");
    masterBypass2.setTooltip ("Bypass master insert 2");
    addAndMakeVisible (masterLevel);
    addAndMakeVisible (masterIns1);
    addAndMakeVisible (masterBypass1);
    addAndMakeVisible (masterAmt1);
    addAndMakeVisible (masterIns2);
    addAndMakeVisible (masterBypass2);
    addAndMakeVisible (masterAmt2);

    aMaster = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "masterLevel", masterLevel);
    aMasterIns1 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "masterIns1", masterIns1);
    aMasterBypass1 = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "masterIns1Bypass", masterBypass1);
    aMasterAmt1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "masterIns1Amount", masterAmt1);
    aMasterIns2 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "masterIns2", masterIns2);
    aMasterBypass2 = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "masterIns2Bypass", masterBypass2);
    aMasterAmt2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, "masterIns2Amount", masterAmt2);

    for (auto* heading : { &linnHeading, &mythHeading })
    {
        heading->setJustificationType (juce::Justification::centredLeft);
        heading->setFont (juce::Font (juce::FontOptions (15.0f).withStyle ("Bold")));
        content.addAndMakeVisible (*heading);
    }
    content.addAndMakeVisible (columns);

    for (int voice = 0; voice < kVoiceCount; ++voice)
    {
        auto* strip = strips.add (new Strip());
        strip->voice = voice;
        strip->bank = kVoices[voice].bank;
        strip->name.setText (kVoices[voice].label, juce::dontSendNotification);
        strip->name.setJustificationType (juce::Justification::centredLeft);

        for (auto* slider : { &strip->level, &strip->pan, &strip->tune, &strip->aux1, &strip->aux2, &strip->amt1, &strip->amt2 })
            styleSlider (*slider);

        strip->level.setTooltip ("Fader");
        strip->pan.setTooltip ("Constant-power pan");
        strip->tune.setTooltip ("Tune in semitones");
        strip->aux1.setTooltip ("Aux 1 send. Destination is Mover (delay), not a picker. Default 0.");
        strip->aux2.setTooltip ("Aux 2 send. Destination is Spatializer (two reverb engines), not a picker. Default 0.");
        strip->amt1.setTooltip ("Insert 1 amount. Zero stays dry until you raise it.");
        strip->amt2.setTooltip ("Insert 2 amount. Zero stays dry until you raise it.");
        strip->bypass1.setTooltip ("Bypass insert 1");
        strip->bypass2.setTooltip ("Bypass insert 2");
        strip->mute.setTooltip ("Mute");
        strip->solo.setTooltip ("Solo. If any solo is on, only soloed strips pass.");

        fillChoices (strip->ins1, apvts, voiceParam (voice, "ins1"));
        fillChoices (strip->ins2, apvts, voiceParam (voice, "ins2"));
        strip->ins1.setTooltip (insertTip);
        strip->ins2.setTooltip (insertTip);
        content.addAndMakeVisible (strip);

        strip->aLevel = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "level"), strip->level);
        strip->aPan = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "pan"), strip->pan);
        strip->aTune = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "tune"), strip->tune);
        strip->aAux1 = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "aux1"), strip->aux1);
        strip->aAux2 = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "aux2"), strip->aux2);
        strip->aAmt1 = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "ins1Amount"), strip->amt1);
        strip->aAmt2 = std::make_unique<Strip::SliderAttachment> (apvts, voiceParam (voice, "ins2Amount"), strip->amt2);
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
    masterLevel.setBounds (master.removeFromLeft (200).reduced (2, 4));
    masterIns1.setBounds (master.removeFromLeft (130).reduced (2, 4));
    masterBypass1.setBounds (master.removeFromLeft (32).reduced (2, 4));
    masterAmt1.setBounds (master.removeFromLeft (120).reduced (2, 4));
    masterIns2.setBounds (master.removeFromLeft (130).reduced (2, 4));
    masterBypass2.setBounds (master.removeFromLeft (32).reduced (2, 4));
    masterAmt2.setBounds (master.removeFromLeft (120).reduced (2, 4));

    viewport.setBounds (area);

    const int width = juce::jmax (stripWidth(), viewport.getMaximumVisibleWidth());
    int y = 0;
    columns.setBounds (0, y, width, kLineH * 2);
    y += kLineH * 2;

    bool placedLinn = false;
    bool placedMyth = false;
    for (auto* strip : strips)
    {
        if (strip->bank == Bank::linn && ! placedLinn)
        {
            linnHeading.setBounds (4, y, width - 8, kSectionH);
            y += kSectionH;
            placedLinn = true;
        }
        if (strip->bank == Bank::mythical && ! placedMyth)
        {
            mythHeading.setBounds (4, y, width - 8, kSectionH);
            y += kSectionH;
            placedMyth = true;
        }
        strip->setBounds (0, y, width, kStripH);
        y += kStripH;
    }

    content.setSize (width, juce::jmax (1, y));
}

} // namespace dz
