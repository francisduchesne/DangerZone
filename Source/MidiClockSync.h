#pragma once

#include <JuceHeader.h>

namespace dz
{

// MIDI clock hook. The step engine still runs on the internal tempo.
// TODO(midi-clock): when midiClockSync is on, derive BPM from 0xF8
// (24 pulses per quarter) and restart the sequencer on 0xFA / continue on 0xFB.
// Host AudioPlayHead follow is a separate later step. Do not fake sync by
// snapping the internal clock until that math is actually in collectHits.
class MidiClockSync
{
public:
    void handle (const juce::MidiMessage& message)
    {
        if (message.isMidiClock())
            ticks.fetch_add (1);
        else if (message.isMidiStart())
            startSeen.store (true);
        else if (message.isMidiStop())
            startSeen.store (false);
        else if (message.isMidiContinue())
            startSeen.store (true);
    }

    int getTickCount() const { return ticks.load(); }
    bool sawStart() const { return startSeen.load(); }

private:
    std::atomic<int> ticks { 0 };
    std::atomic<bool> startSeen { false };
};

} // namespace dz
