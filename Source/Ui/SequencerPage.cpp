#include "SequencerPage.h"

namespace dz
{

struct SequencerPage::Row : public juce::Component
{
    juce::Label name;
    std::array<juce::ToggleButton, kMaxSteps> steps;

    void resized() override
    {
        auto area = getLocalBounds();
        name.setBounds (area.removeFromLeft (118));
        for (auto& step : steps)
            step.setBounds (area.removeFromLeft (28).reduced (1));
    }
};

SequencerPage::SequencerPage (DangerZoneAudioProcessor& processorIn)
    : processor (processorIn)
{
    play.onClick = [this]
    {
        processor.requestSequencerRestart();
        if (auto* param = processor.getApvts().getParameter ("playing"))
            param->setValueNotifyingHost (1.0f);
    };
    stop.onClick = [this]
    {
        if (auto* param = processor.getApvts().getParameter ("playing"))
            param->setValueNotifyingHost (0.0f);
    };
    addAndMakeVisible (play);
    addAndMakeVisible (stop);

    auto prep = [] (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 56, 18);
    };
    prep (tempo);
    prep (length);
    prep (swing);
    addAndMakeVisible (tempo);
    addAndMakeVisible (length);
    addAndMakeVisible (swing);

    for (auto* label : { &tempoLabel, &lengthLabel, &swingLabel, &playhead, &clock })
    {
        label->setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (*label);
    }
    tempoLabel.setText ("Tempo", juce::dontSendNotification);
    lengthLabel.setText ("Length", juce::dontSendNotification);
    swingLabel.setText ("Swing", juce::dontSendNotification);
    playhead.setText ("Step --", juce::dontSendNotification);
    clock.setText ("MIDI clock sync is stubbed", juce::dontSendNotification);

    tempoAttach = std::make_unique<SliderAttachment> (processor.getApvts(), "tempo", tempo);
    lengthAttach = std::make_unique<SliderAttachment> (processor.getApvts(), "patternLength", length);
    swingAttach = std::make_unique<SliderAttachment> (processor.getApvts(), "swing", swing);

    for (int voice = 0; voice < kVoiceCount; ++voice)
    {
        auto* row = rows.add (new Row());
        row->name.setText (kVoices[voice].label, juce::dontSendNotification);
        content.addAndMakeVisible (row);
        for (int step = 0; step < kMaxSteps; ++step)
        {
            row->steps[(size_t) step].setTooltip ("Step " + juce::String (step + 1));
            row->steps[(size_t) step].onClick = [this, voice, step, row]
            {
                processor.getSequencer().setStep (voice, step, row->steps[(size_t) step].getToggleState());
            };
        }
    }

    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);
    syncGrid();
    startTimerHz (30);
}

SequencerPage::~SequencerPage() = default;

void SequencerPage::syncGrid()
{
    for (int voice = 0; voice < rows.size(); ++voice)
    {
        auto* row = rows[voice];
        for (int step = 0; step < kMaxSteps; ++step)
        {
            const bool on = processor.getSequencer().getStep (voice, step);
            if (row->steps[(size_t) step].getToggleState() != on)
                row->steps[(size_t) step].setToggleState (on, juce::dontSendNotification);
        }
    }
    seenGeneration = processor.getSequencer().getGeneration();
}

void SequencerPage::timerCallback()
{
    if (processor.getSequencer().getGeneration() != seenGeneration)
        syncGrid();

    const int step = processor.getSequencer().getCurrentStep();
    playhead.setText (step < 0 ? "Step --" : "Step " + juce::String (step + 1), juce::dontSendNotification);
    clock.setText ("MIDI clock: stub (" + juce::String (processor.getMidiClockTicks())
                       + " ticks). Sync param is stored; the grid still uses internal tempo.",
                   juce::dontSendNotification);
}

void SequencerPage::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void SequencerPage::resized()
{
    auto area = getLocalBounds().reduced (8);
    auto top = area.removeFromTop (28);
    play.setBounds (top.removeFromLeft (72));
    top.removeFromLeft (6);
    stop.setBounds (top.removeFromLeft (72));
    top.removeFromLeft (12);
    playhead.setBounds (top.removeFromLeft (80));

    auto sliders = area.removeFromTop (28);
    tempoLabel.setBounds (sliders.removeFromLeft (52));
    tempo.setBounds (sliders.removeFromLeft (220));
    lengthLabel.setBounds (sliders.removeFromLeft (56));
    length.setBounds (sliders.removeFromLeft (160));
    swingLabel.setBounds (sliders.removeFromLeft (52));
    swing.setBounds (sliders.removeFromLeft (200));

    clock.setBounds (area.removeFromTop (22));
    viewport.setBounds (area);

    const int rowHeight = 26;
    const int width = juce::jmax (viewport.getWidth() - 16, 118 + kMaxSteps * 28);
    content.setSize (width, juce::jmax (1, rows.size() * rowHeight));
    for (int i = 0; i < rows.size(); ++i)
        rows[i]->setBounds (0, i * rowHeight, width, rowHeight);
}

} // namespace dz
