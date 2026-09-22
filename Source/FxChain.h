#pragma once

#include <JuceHeader.h>

namespace dz
{

struct SpatializerSettings
{
    float sizeA = 0.32f;
    float sizeB = 0.74f;
    float levelA = 0.70f;
    float levelB = 0.45f;
};

// Aux engines. Not insert slots.
// Aux 1 -> Mover (delay stub).
// Aux 2 -> Spatializer with two reverb engines (stubs).
// TODO(fvs): swap these bodies for the real FVS Mover and FVS Spatializer.
// The return levels and the per-voice send params stay as they are.
class FxChain
{
public:
    void prepare (double sampleRate, int maxBlock);
    void reset();
    void ensureBlock (int numSamples);

    void processMover (juce::AudioBuffer<float>& aux, int numSamples, float timeMs, float feedback);
    void processSpatializer (juce::AudioBuffer<float>& aux, int numSamples, const SpatializerSettings& settings);

private:
    double sampleRate = 44100.0;
    int capacity = 0;
    int writeIndex = 0;
    std::vector<float> delayL;
    std::vector<float> delayR;
    juce::AudioBuffer<float> scratchA;
    juce::AudioBuffer<float> scratchB;
    juce::dsp::Reverb engineA;
    juce::dsp::Reverb engineB;
};

} // namespace dz
