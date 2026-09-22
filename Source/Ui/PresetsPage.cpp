#include "PresetsPage.h"

namespace dz
{

PresetsPage::PresetsPage (DangerZoneAudioProcessor& processorIn)
    : processor (processorIn)
{
    name.setTextToShowWhenEmpty ("User preset name", juce::Colours::grey);
    addAndMakeVisible (presets);
    addAndMakeVisible (load);
    addAndMakeVisible (save);
    addAndMakeVisible (name);
    status.setJustificationType (juce::Justification::topLeft);
    addAndMakeVisible (status);

    load.onClick = [this]
    {
        const auto selected = presets.getText();
        juce::String error;
        if (processor.loadPresetByName (selected, error))
            status.setText ("Loaded " + processor.getPresetName(), juce::dontSendNotification);
        else
            status.setText (error, juce::dontSendNotification);
    };
    save.onClick = [this]
    {
        juce::String error;
        if (processor.saveUserPreset (name.getText(), error))
        {
            status.setText ("Saved " + processor.getPresetName() + " to the Arcades/Danger Zone/Presets folder.",
                            juce::dontSendNotification);
            refreshList();
        }
        else
        {
            status.setText (error, juce::dontSendNotification);
        }
    };

    refreshList();
    startTimerHz (2);
}

void PresetsPage::refreshList()
{
    const auto current = presets.getText();
    presets.clear (juce::dontSendNotification);
    listed = processor.listPresetNames();
    for (int i = 0; i < listed.size(); ++i)
        presets.addItem (listed[i], i + 1);
    if (listed.contains (current))
        presets.setText (current, juce::dontSendNotification);
    else if (listed.size() > 0)
        presets.setSelectedItemIndex (0, juce::dontSendNotification);
}

void PresetsPage::timerCallback()
{
    const auto live = processor.getPresetName();
    if (status.getText().isEmpty())
        status.setText ("Current: " + live + ". Factory: Stock Linn (dry) and Processed (aux + inserts).",
                        juce::dontSendNotification);
}

void PresetsPage::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PresetsPage::resized()
{
    auto area = getLocalBounds().reduced (12);
    presets.setBounds (area.removeFromTop (28).removeFromLeft (320));
    area.removeFromTop (8);
    load.setBounds (area.removeFromTop (28).removeFromLeft (120));
    area.removeFromTop (16);
    name.setBounds (area.removeFromTop (28).removeFromLeft (320));
    area.removeFromTop (8);
    save.setBounds (area.removeFromTop (28).removeFromLeft (180));
    area.removeFromTop (16);
    status.setBounds (area.removeFromTop (80));
}

} // namespace dz
