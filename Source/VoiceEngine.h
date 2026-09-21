#pragma once

#include "VoiceBank.h"

namespace dz
{

// Multi-sample one-shot engine.
// Files: Resources/Samples/<VoiceId>/<voice>_vN.wav
// Optional snare (any voice) pitch layers: <voice>_p0_vN.wav, <voice>_pm3_vN.wav
// TODO(samples): replace procedural placeholders with ~6 takes per hit from the
// orange/black Linn. Snare needs real pitch-layer folders/files. Do not loop
// one take — that is the machine-gun this engine is here to avoid.
class VoiceEngine
{
public:
    void setDeviceSampleRate (double sampleRate);
    void load (const juce::File& samplesRoot);
    void panic();

    // Audio thread. tuneSemis is the voice tune. Jitter is applied per hit.
    void trigger (int voice,
                  float velocity,
                  int sampleOffset,
                  float tuneSemis,
                  float pitchJitterCents,
                  float timingJitterMs,
                  bool randomVariants);

    void render (juce::AudioBuffer<float>* voiceBuses, int numVoices, int numSamples);

    int getVariantCount (int voice) const;
    juce::String getSamplesRoot() const { return rootPath; }
    juce::String getLoadReport() const { return loadReport; }

private:
    struct Variant
    {
        juce::AudioBuffer<float> audio;
        double sourceRate = 44100.0;
        int sortKey = 0;
    };

    struct Pool
    {
        int semis = 0;
        bool implicit = true;
        int rr = 0;
        int last = -1;
        std::vector<Variant> vars;
    };

    struct VoiceSamples
    {
        std::vector<Pool> pools;
    };

    struct Active
    {
        bool on = false;
        int voice = -1;
        const float* left = nullptr;
        const float* right = nullptr;
        int length = 0;
        double pos = 0.0;
        double increment = 1.0;
        int delay = 0;
        float velocity = 1.0f;
        float fade = 0.0f;
        float release = 1.0f;
        bool releasing = false;
        int age = 0;
    };

    static constexpr int kPoolSize = 48;

    Pool* pickPool (VoiceSamples& samples, float tuneSemis);
    int pickVariant (Pool& pool, bool random);
    uint32_t nextRand();
    float nextUnit();

    std::array<VoiceSamples, kVoiceCount> voices;
    std::array<Active, kPoolSize> active {};
    std::array<int, kVoiceCount> variantCounts {};
    double deviceRate = 44100.0;
    uint32_t rng = 0xA341316Cu;
    juce::String rootPath;
    juce::String loadReport;
    juce::AudioFormatManager formats;
    bool formatsReady = false;
};

juce::File findSamplesRoot();

} // namespace dz
