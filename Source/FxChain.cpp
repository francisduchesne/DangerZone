#include "FxChain.h"

namespace dz
{

void FxChain::prepare (double newSampleRate, int maxBlock)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    const int delaySamples = juce::jmax (8, (int) (sampleRate * 2.0));
    delayL.assign ((size_t) delaySamples, 0.0f);
    delayR.assign ((size_t) delaySamples, 0.0f);
    writeIndex = 0;

    capacity = juce::jmax (maxBlock, 1);
    scratchA.setSize (2, capacity);
    scratchB.setSize (2, capacity);
    scratchA.clear();
    scratchB.clear();

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) capacity;
    spec.numChannels = 2;
    engineA.prepare (spec);
    engineB.prepare (spec);
    engineA.reset();
    engineB.reset();
}

void FxChain::reset()
{
    std::fill (delayL.begin(), delayL.end(), 0.0f);
    std::fill (delayR.begin(), delayR.end(), 0.0f);
    writeIndex = 0;
    scratchA.clear();
    scratchB.clear();
    engineA.reset();
    engineB.reset();
}

void FxChain::ensureBlock (int numSamples)
{
    if (numSamples <= capacity)
        return;
    prepare (sampleRate, numSamples);
}

void FxChain::processMover (juce::AudioBuffer<float>& aux, int numSamples, float timeMs, float feedback)
{
    if (delayL.empty() || numSamples <= 0 || aux.getNumChannels() < 1)
        return;

    const int size = (int) delayL.size();
    const int delaySamples = juce::jlimit (1, size - 1, (int) (sampleRate * (double) timeMs * 0.001));
    const float fb = juce::jlimit (0.0f, 0.85f, feedback);
    auto* left = aux.getWritePointer (0);
    auto* right = aux.getNumChannels() > 1 ? aux.getWritePointer (1) : left;

    for (int i = 0; i < numSamples; ++i)
    {
        int read = writeIndex - delaySamples;
        if (read < 0)
            read += size;
        const float wetL = delayL[(size_t) read];
        const float wetR = delayR[(size_t) read];
        delayL[(size_t) writeIndex] = left[i] + wetL * fb;
        delayR[(size_t) writeIndex] = right[i] + wetR * fb;
        left[i] = wetL;
        right[i] = wetR;
        writeIndex = (writeIndex + 1) % size;
    }
}

void FxChain::processSpatializer (juce::AudioBuffer<float>& aux, int numSamples, const SpatializerSettings& settings)
{
    if (numSamples <= 0 || aux.getNumChannels() < 1)
        return;

    ensureBlock (numSamples);

    auto setEngine = [] (juce::dsp::Reverb& engine, float size)
    {
        juce::dsp::Reverb::Parameters params;
        params.roomSize = juce::jlimit (0.0f, 1.0f, 0.12f + 0.84f * size);
        params.damping = 0.45f;
        params.wetLevel = 1.0f;
        params.dryLevel = 0.0f;
        params.width = 1.0f;
        params.freezeMode = 0.0f;
        engine.setParameters (params);
    };
    setEngine (engineA, settings.sizeA);
    setEngine (engineB, settings.sizeB);

    const int channels = juce::jmin (2, aux.getNumChannels());
    for (int ch = 0; ch < channels; ++ch)
    {
        scratchA.copyFrom (ch, 0, aux, ch, 0, numSamples);
        scratchB.copyFrom (ch, 0, aux, ch, 0, numSamples);
    }
    if (channels == 1)
    {
        scratchA.copyFrom (1, 0, scratchA, 0, 0, numSamples);
        scratchB.copyFrom (1, 0, scratchB, 0, 0, numSamples);
    }

    auto run = [numSamples] (juce::dsp::Reverb& engine, juce::AudioBuffer<float>& buffer)
    {
        juce::dsp::AudioBlock<float> block (buffer.getArrayOfWritePointers(), 2, (size_t) numSamples);
        juce::dsp::ProcessContextReplacing<float> context (block);
        engine.process (context);
    };
    run (engineA, scratchA);
    run (engineB, scratchB);

    const float levelA = juce::jlimit (0.0f, 1.0f, settings.levelA);
    const float levelB = juce::jlimit (0.0f, 1.0f, settings.levelB);
    for (int ch = 0; ch < channels; ++ch)
    {
        auto* out = aux.getWritePointer (ch);
        const auto* a = scratchA.getReadPointer (ch);
        const auto* b = scratchB.getReadPointer (ch);
        for (int i = 0; i < numSamples; ++i)
            out[i] = a[i] * levelA + b[i] * levelB;
    }
}

} // namespace dz
