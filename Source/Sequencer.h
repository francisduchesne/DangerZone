#pragma once

#include "VoiceBank.h"

#include <array>
#include <atomic>
#include <cstdint>

namespace dz
{

// 16-step pattern. Bits live in atomics so the audio thread and the grid
// share them without a lock. MIDI clock follow is intentionally not here;
// see MidiClockSync.
class Sequencer
{
public:
    Sequencer();

    void prepare (double sampleRate);
    void resetToStart();

    void setStep (int voice, int step, bool on);
    bool getStep (int voice, int step) const;
    void setMask (int voice, uint16_t mask);
    uint16_t getMask (int voice) const;
    void bumpGeneration();

    int getCurrentStep() const { return currentStep.load(); }
    int getGeneration() const { return generation.load(); }

    struct Hit
    {
        int voice = 0;
        int offset = 0;
    };

    int collectHits (int numSamples,
                     double bpm,
                     int patternLength,
                     float swing01,
                     bool playing,
                     Hit* hits,
                     int maxHits);

    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree& tree);

private:
    std::array<std::atomic<uint16_t>, kVoiceCount> bits;
    std::atomic<int> currentStep { -1 };
    std::atomic<int> generation { 0 };
    int stepIndex = 0;
    double samplesUntilNext = 0.0;
    double sampleRate = 44100.0;
};

} // namespace dz
