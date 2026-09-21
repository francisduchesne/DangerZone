#include "Sequencer.h"

namespace dz
{

Sequencer::Sequencer()
{
    for (auto& bit : bits)
        bit.store (0);
}

void Sequencer::prepare (double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
}

void Sequencer::resetToStart()
{
    stepIndex = 0;
    samplesUntilNext = 0.0;
    currentStep.store (-1);
}

void Sequencer::setStep (int voice, int step, bool on)
{
    if (voice < 0 || voice >= kVoiceCount || step < 0 || step >= kMaxSteps)
        return;

    auto& cell = bits[(size_t) voice];
    const uint16_t mask = (uint16_t) (1u << step);
    const uint16_t cur = cell.load();
    cell.store (on ? (uint16_t) (cur | mask) : (uint16_t) (cur & (uint16_t) ~mask));
}

bool Sequencer::getStep (int voice, int step) const
{
    if (voice < 0 || voice >= kVoiceCount || step < 0 || step >= kMaxSteps)
        return false;
    return (bits[(size_t) voice].load() & (uint16_t) (1u << step)) != 0;
}

void Sequencer::setMask (int voice, uint16_t mask)
{
    if (voice >= 0 && voice < kVoiceCount)
        bits[(size_t) voice].store (mask);
}

uint16_t Sequencer::getMask (int voice) const
{
    if (voice < 0 || voice >= kVoiceCount)
        return 0;
    return bits[(size_t) voice].load();
}

void Sequencer::bumpGeneration()
{
    generation.fetch_add (1);
}

int Sequencer::collectHits (int numSamples,
                            double bpm,
                            int patternLength,
                            float swing01,
                            bool playing,
                            Hit* hits,
                            int maxHits)
{
    if (! playing || patternLength < 1 || bpm < 1.0 || numSamples <= 0)
        return 0;

    patternLength = juce::jlimit (1, kMaxSteps, patternLength);
    if (stepIndex >= patternLength)
        stepIndex = 0;

    // Swing 0 = straight 16ths. Swing 1 pushes the odd 16th toward a triplet.
    const double swing = (double) juce::jlimit (0.0f, 1.0f, swing01);
    const double sixteenth = (60.0 / bpm) * sampleRate / 4.0;
    const double pair = sixteenth * 2.0;
    const double longFraction = 0.5 + (1.0 / 6.0) * swing;
    const double evenDur = juce::jmax (1.0, pair * longFraction);
    const double oddDur = juce::jmax (1.0, pair - (pair * longFraction));

    int count = 0;
    double next = samplesUntilNext;

    while (next < (double) numSamples)
    {
        const int offset = juce::jlimit (0, numSamples - 1, (int) next);
        currentStep.store (stepIndex);

        if (count < maxHits)
        {
            const uint16_t stepBit = (uint16_t) (1u << stepIndex);
            for (int voice = 0; voice < kVoiceCount && count < maxHits; ++voice)
                if ((bits[(size_t) voice].load() & stepBit) != 0)
                    hits[count++] = { voice, offset };
        }

        const int fired = stepIndex;
        stepIndex = (stepIndex + 1) % patternLength;
        next += ((fired % 2) == 0) ? evenDur : oddDur;
    }

    samplesUntilNext = next - (double) numSamples;
    return count;
}

juce::ValueTree Sequencer::toValueTree() const
{
    juce::ValueTree tree ("PATTERN");
    for (int i = 0; i < kVoiceCount; ++i)
        tree.setProperty (kVoices[i].id, (int) bits[(size_t) i].load(), nullptr);
    return tree;
}

void Sequencer::fromValueTree (const juce::ValueTree& tree)
{
    if (! tree.isValid())
        return;

    for (int i = 0; i < kVoiceCount; ++i)
    {
        const int fallback = (int) bits[(size_t) i].load();
        const int stored = (int) tree.getProperty (kVoices[i].id, fallback);
        bits[(size_t) i].store ((uint16_t) stored);
    }
    bumpGeneration();
}

} // namespace dz
