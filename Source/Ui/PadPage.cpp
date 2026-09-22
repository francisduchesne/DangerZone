#include "PadPage.h"

namespace dz
{

PadPage::PadPage (DangerZoneAudioProcessor& processorIn, Bank bankIn)
    : processor (processorIn),
      bank (bankIn)
{
    for (int i = 0; i < kVoiceCount; ++i)
    {
        if (kVoices[i].bank != bank)
            continue;
        voices.push_back (i);
        auto* pad = pads.add (new juce::TextButton());
        addAndMakeVisible (pad);
        const int voice = i;
        pad->onClick = [this, voice] { processor.triggerVoiceFromUi (voice); };
    }

    reload.onClick = [this]
    {
        processor.reloadSamples();
        refreshLabels();
    };
    addAndMakeVisible (reload);

    hint.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (hint);
    refreshLabels();
}

void PadPage::refreshLabels()
{
    for (int i = 0; i < pads.size(); ++i)
    {
        const int voice = voices[(size_t) i];
        const int loaded = processor.getEngine().getVariantCount (voice);
        const int target = kVoices[voice].targetVariants;
        pads[i]->setButtonText (juce::String (kVoices[voice].label) + "\n"
                                + juce::String (loaded) + " / " + juce::String (target) + " var");
    }

    if (bank == Bank::mythical)
        hint.setText ("Mythical / era slots. Empty until WAVs land in Resources/Samples/<Voice>/ "
                      "or Resources/Samples/Mythical/<Voice>/. TODO: real era-machine multi-samples.",
                      juce::dontSendNotification);
    else
        hint.setText ("Click a pad to audition. Counts are loaded variants / target takes. "
                      "TODO: replace placeholders with the orange/black Linn (~6 hits, snare pitch layers).",
                      juce::dontSendNotification);
}

void PadPage::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PadPage::resized()
{
    auto area = getLocalBounds().reduced (8);
    auto bottom = area.removeFromBottom (72);
    reload.setBounds (bottom.removeFromTop (28).removeFromLeft (160));
    hint.setBounds (bottom.reduced (0, 4));

    const int columns = 6;
    const int width = 150;
    const int height = 52;
    for (int i = 0; i < pads.size(); ++i)
    {
        const int column = i % columns;
        const int row = i / columns;
        pads[i]->setBounds (area.getX() + column * (width + 8),
                            area.getY() + row * (height + 8),
                            width, height);
    }
}

} // namespace dz
