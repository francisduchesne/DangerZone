#include "FxPage.h"

namespace dz
{

void FxPage::styleSlider (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 18);
}

FxPage::FxPage (DangerZoneAudioProcessor& processorIn)
    : processor (processorIn)
{
    voiceLabel.setText ("Voice", juce::dontSendNotification);
    tuneLabel.setText ("Tune", juce::dontSendNotification);
    amt1Label.setText ("Insert 1 amount", juce::dontSendNotification);
    amt2Label.setText ("Insert 2 amount", juce::dontSendNotification);
    for (auto* label : { &voiceLabel, &tuneLabel, &amt1Label, &amt2Label })
        addAndMakeVisible (*label);

    for (int i = 0; i < kVoiceCount; ++i)
        voiceBox.addItem (kVoices[i].label, i + 1);
    voiceBox.setSelectedId (1, juce::dontSendNotification);
    voiceBox.onChange = [this]
    {
        const int id = voiceBox.getSelectedId();
        if (id > 0)
            showVoice (id - 1);
    };
    addAndMakeVisible (voiceBox);

    for (auto* slider : { &tune, &ins1Amount, &ins2Amount, &moverTime, &moverFeedback, &moverReturn,
                          &spatSizeA, &spatSizeB, &spatLevelA, &spatLevelB, &spatReturn,
                          &timingJitter, &pitchJitter })
    {
        styleSlider (*slider);
        addAndMakeVisible (*slider);
    }
    moverTime.setTooltip ("Mover delay time");
    moverTime.setTextValueSuffix (" ms");
    moverFeedback.setTooltip ("Mover feedback");
    moverReturn.setTooltip ("Mover return into the master");
    spatSizeA.setTooltip ("Spatializer engine A size");
    spatSizeB.setTooltip ("Spatializer engine B size");
    spatLevelA.setTooltip ("Spatializer engine A level");
    spatLevelB.setTooltip ("Spatializer engine B level");
    spatReturn.setTooltip ("Spatializer return into the master");
    timingJitter.setTooltip ("Random delay on each hit");
    timingJitter.setTextValueSuffix (" ms");
    pitchJitter.setTooltip ("Random pitch on each hit");
    pitchJitter.setTextValueSuffix (" ct");

    addAndMakeVisible (moverGroup);
    addAndMakeVisible (spatGroup);
    addAndMakeVisible (feelGroup);

    if (auto* param = processor.getApvts().getParameter ("variantMode"))
        variantMode.addItemList (param->getAllValueStrings(), 1);
    addAndMakeVisible (variantMode);
    addAndMakeVisible (midiSync);

    auto& apvts = processor.getApvts();
    aMoverTime = std::make_unique<SliderAttachment> (apvts, "moverTimeMs", moverTime);
    aMoverFb = std::make_unique<SliderAttachment> (apvts, "moverFeedback", moverFeedback);
    aMoverRet = std::make_unique<SliderAttachment> (apvts, "moverReturn", moverReturn);
    aSizeA = std::make_unique<SliderAttachment> (apvts, "spatSizeA", spatSizeA);
    aSizeB = std::make_unique<SliderAttachment> (apvts, "spatSizeB", spatSizeB);
    aLevA = std::make_unique<SliderAttachment> (apvts, "spatLevelA", spatLevelA);
    aLevB = std::make_unique<SliderAttachment> (apvts, "spatLevelB", spatLevelB);
    aSpatRet = std::make_unique<SliderAttachment> (apvts, "spatReturn", spatReturn);
    aTiming = std::make_unique<SliderAttachment> (apvts, "timingJitterMs", timingJitter);
    aPitch = std::make_unique<SliderAttachment> (apvts, "pitchJitterCents", pitchJitter);
    aVariant = std::make_unique<ComboAttachment> (apvts, "variantMode", variantMode);
    aMidi = std::make_unique<ButtonAttachment> (apvts, "midiClockSync", midiSync);

    showVoice (0);
}

void FxPage::showVoice (int voiceIndex)
{
    if (voiceIndex < 0 || voiceIndex >= kVoiceCount)
        return;

    aTune.reset();
    aAmt1.reset();
    aAmt2.reset();

    auto& apvts = processor.getApvts();
    aTune = std::make_unique<SliderAttachment> (apvts, voiceParam (voiceIndex, "tune"), tune);
    aAmt1 = std::make_unique<SliderAttachment> (apvts, voiceParam (voiceIndex, "ins1Amount"), ins1Amount);
    aAmt2 = std::make_unique<SliderAttachment> (apvts, voiceParam (voiceIndex, "ins2Amount"), ins2Amount);
}

void FxPage::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void FxPage::resized()
{
    auto area = getLocalBounds().reduced (10);
    auto voiceRow = area.removeFromTop (28);
    voiceLabel.setBounds (voiceRow.removeFromLeft (48));
    voiceBox.setBounds (voiceRow.removeFromLeft (180));

    auto tuneRow = area.removeFromTop (28);
    tuneLabel.setBounds (tuneRow.removeFromLeft (48));
    tune.setBounds (tuneRow.removeFromLeft (280));
    auto amtRow = area.removeFromTop (28);
    amt1Label.setBounds (amtRow.removeFromLeft (120));
    ins1Amount.setBounds (amtRow.removeFromLeft (320));
    auto amtRow2 = area.removeFromTop (28);
    amt2Label.setBounds (amtRow2.removeFromLeft (120));
    ins2Amount.setBounds (amtRow2.removeFromLeft (320));

    area.removeFromTop (8);
    auto moverArea = area.removeFromTop (150);
    moverGroup.setBounds (moverArea);
    moverArea = moverArea.reduced (12, 22);
    moverTime.setBounds (moverArea.removeFromTop (28).removeFromLeft (420));
    moverFeedback.setBounds (moverArea.removeFromTop (28).removeFromLeft (420));
    moverReturn.setBounds (moverArea.removeFromTop (28).removeFromLeft (420));

    area.removeFromTop (8);
    auto spatArea = area.removeFromTop (210);
    spatGroup.setBounds (spatArea);
    spatArea = spatArea.reduced (12, 22);
    spatSizeA.setBounds (spatArea.removeFromTop (28).removeFromLeft (420));
    spatLevelA.setBounds (spatArea.removeFromTop (28).removeFromLeft (420));
    spatSizeB.setBounds (spatArea.removeFromTop (28).removeFromLeft (420));
    spatLevelB.setBounds (spatArea.removeFromTop (28).removeFromLeft (420));
    spatReturn.setBounds (spatArea.removeFromTop (28).removeFromLeft (420));

    area.removeFromTop (8);
    feelGroup.setBounds (area);
    auto feel = area.reduced (12, 22);
    variantMode.setBounds (feel.removeFromTop (28).removeFromLeft (220));
    timingJitter.setBounds (feel.removeFromTop (28).removeFromLeft (420));
    pitchJitter.setBounds (feel.removeFromTop (28).removeFromLeft (420));
    midiSync.setBounds (feel.removeFromTop (28).removeFromLeft (420));
}

} // namespace dz
