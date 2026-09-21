#include "PluginEditor.h"

#include "Ui/FxPage.h"
#include "Ui/MixerPage.h"
#include "Ui/PadPage.h"
#include "Ui/PresetsPage.h"
#include "Ui/SequencerPage.h"

DangerZoneAudioProcessorEditor::DangerZoneAudioProcessorEditor (DangerZoneAudioProcessor& processorIn)
    : juce::AudioProcessorEditor (&processorIn),
      processor (processorIn)
{
    auto stock = juce::Colours::transparentBlack;
    tabs.addTab ("Linn", stock, new dz::PadPage (processor, dz::Bank::linn), true);
    tabs.addTab ("Mythical", stock, new dz::PadPage (processor, dz::Bank::mythical), true);
    tabs.addTab ("Sequencer", stock, new dz::SequencerPage (processor), true);
    tabs.addTab ("Mixer", stock, new dz::MixerPage (processor), true);
    tabs.addTab ("FX", stock, new dz::FxPage (processor), true);
    tabs.addTab ("Presets", stock, new dz::PresetsPage (processor), true);
    tabs.setTabBarDepth (30);
    tabs.setOutline (0);
    addAndMakeVisible (tabs);

    footer.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (footer);

    setSize (1180, 780);
    setResizable (true, true);
    setResizeLimits (980, 640, 1900, 1400);
    startTimerHz (2);
}

void DangerZoneAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void DangerZoneAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    footer.setBounds (area.removeFromBottom (22).reduced (8, 2));
    tabs.setBounds (area);
}

void DangerZoneAudioProcessorEditor::timerCallback()
{
    footer.setText ("Danger Zone  |  Arcades  |  " + processor.getPresetName()
                        + "  |  " + processor.getSamplesStatus(),
                    juce::dontSendNotification);
}
