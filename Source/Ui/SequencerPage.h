#pragma once

#include "PluginProcessor.h"

namespace dz
{

class SequencerPage : public juce::Component,
                      private juce::Timer
{
public:
    explicit SequencerPage (DangerZoneAudioProcessor&);
    ~SequencerPage() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    struct Row;

    void timerCallback() override;
    void syncGrid();

    DangerZoneAudioProcessor& processor;
    juce::TextButton play { "Play" };
    juce::TextButton stop { "Stop" };
    juce::Slider tempo, length, swing;
    juce::Label tempoLabel, lengthLabel, swingLabel, playhead, clock;
    juce::Component content;
    juce::OwnedArray<Row> rows;
    juce::Viewport viewport;
    int seenGeneration = -1;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> tempoAttach, lengthAttach, swingAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequencerPage)
};

} // namespace dz
