#include "VoiceEngine.h"

#include <algorithm>
#include <cmath>

namespace dz
{
namespace
{

struct ParsedName
{
    bool ok = false;
    bool layered = false;
    int variant = 1;
    int semis = 0;
};

ParsedName parseSampleName (const juce::String& stem)
{
    ParsedName parsed;
    const auto lower = stem.toLowerCase();
    const int vPos = lower.lastIndexOf ("_v");
    if (vPos < 0)
        return parsed;

    parsed.variant = juce::jmax (1, lower.substring (vPos + 2).getIntValue());
    const int pPos = lower.indexOf ("_p");
    if (pPos >= 0 && pPos < vPos)
    {
        parsed.layered = true;
        auto rest = lower.substring (pPos + 2, vPos);
        int sign = 1;
        if (rest.startsWithChar ('m'))
        {
            sign = -1;
            rest = rest.substring (1);
        }
        parsed.semis = sign * rest.getIntValue();
    }
    parsed.ok = true;
    return parsed;
}

} // namespace

juce::File findSamplesRoot()
{
    juce::Array<juce::File> seeds;
    seeds.add (juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory());
    seeds.add (juce::File::getSpecialLocation (juce::File::currentApplicationFile).getParentDirectory());

    for (auto seed : seeds)
    {
        auto cursor = seed;
        for (int depth = 0; depth < 8 && cursor.getFullPathName().isNotEmpty(); ++depth)
        {
            const auto direct = cursor.getChildFile ("Resources").getChildFile ("Samples");
            if (direct.getChildFile ("Kick").isDirectory())
                return direct;

            const auto sibling = cursor.getSiblingFile ("Resources").getChildFile ("Samples");
            if (sibling.getChildFile ("Kick").isDirectory())
                return sibling;

            cursor = cursor.getParentDirectory();
        }
    }

    return {};
}

void VoiceEngine::setDeviceSampleRate (double sampleRate)
{
    if (sampleRate > 0.0)
        deviceRate = sampleRate;
}

uint32_t VoiceEngine::nextRand()
{
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    if (rng == 0)
        rng = 0xA341316Cu;
    return rng;
}

float VoiceEngine::nextUnit()
{
    return (float) (nextRand() & 0xFFFFFFu) / (float) 0x1000000u;
}

void VoiceEngine::load (const juce::File& samplesRoot)
{
    if (! formatsReady)
    {
        formats.registerBasicFormats();
        formatsReady = true;
    }

    for (auto& voice : voices)
        voice.pools.clear();
    variantCounts.fill (0);
    panic();

    rootPath = samplesRoot.getFullPathName();
    loadReport.clear();

    if (! samplesRoot.isDirectory())
    {
        loadReport = "Samples folder not found. Drop WAVs in Resources/Samples/<Voice>/ (see README).";
        return;
    }

    int files = 0;
    int voiced = 0;

    for (int i = 0; i < kVoiceCount; ++i)
    {
        auto dir = samplesRoot.getChildFile (kVoices[i].id);
        if (! dir.isDirectory() && kVoices[i].bank == Bank::mythical)
            dir = samplesRoot.getChildFile ("Mythical").getChildFile (kVoices[i].id);
        if (! dir.isDirectory())
            continue;

        juce::Array<juce::File> wavs;
        dir.findChildFiles (wavs, juce::File::findFiles, false, "*.wav");

        Pool base;
        base.semis = 0;
        base.implicit = true;
        std::vector<Pool> layers;

        for (const auto& file : wavs)
        {
            const auto parsed = parseSampleName (file.getFileNameWithoutExtension());
            if (! parsed.ok)
                continue;

            std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
            if (reader == nullptr || reader->lengthInSamples <= 0)
                continue;

            Variant variant;
            variant.sortKey = parsed.variant;
            variant.sourceRate = reader->sampleRate > 0.0 ? reader->sampleRate : 44100.0;
            const int channels = juce::jlimit (1, 2, (int) reader->numChannels);
            const int length = (int) juce::jmin ((juce::int64) 8 * 1024 * 1024, reader->lengthInSamples);
            variant.audio.setSize (channels, length);
            reader->read (&variant.audio, 0, length, 0, true, true);

            if (parsed.layered)
            {
                Pool* dest = nullptr;
                for (auto& layer : layers)
                    if (layer.semis == parsed.semis)
                        dest = &layer;
                if (dest == nullptr)
                {
                    Pool created;
                    created.semis = parsed.semis;
                    created.implicit = false;
                    layers.push_back (std::move (created));
                    dest = &layers.back();
                }
                dest->vars.push_back (std::move (variant));
            }
            else
            {
                base.vars.push_back (std::move (variant));
            }
            ++files;
        }

        auto sortPool = [] (Pool& pool)
        {
            std::sort (pool.vars.begin(), pool.vars.end(),
                       [] (const Variant& a, const Variant& b) { return a.sortKey < b.sortKey; });
        };
        sortPool (base);
        for (auto& layer : layers)
            sortPool (layer);

        if (! base.vars.empty())
            voices[(size_t) i].pools.push_back (std::move (base));
        for (auto& layer : layers)
            if (! layer.vars.empty())
                voices[(size_t) i].pools.push_back (std::move (layer));

        int count = 0;
        for (const auto& pool : voices[(size_t) i].pools)
            count += (int) pool.vars.size();
        variantCounts[(size_t) i] = count;
        if (count > 0)
            ++voiced;
    }

    loadReport = "Loaded " + juce::String (files) + " WAVs for " + juce::String (voiced)
                 + " / " + juce::String (kVoiceCount) + " voices from " + rootPath
                 + ". TODO: replace with real Linn multi-samples.";
}

void VoiceEngine::panic()
{
    for (auto& hit : active)
        hit.on = false;
}

VoiceEngine::Pool* VoiceEngine::pickPool (VoiceSamples& samples, float tuneSemis)
{
    Pool* best = nullptr;
    float bestDist = 1.0e9f;
    for (auto& pool : samples.pools)
    {
        if (pool.vars.empty())
            continue;
        const float dist = std::abs ((float) pool.semis - tuneSemis);
        const bool closer = best == nullptr || dist < bestDist - 0.001f;
        const bool tiePreferExplicit = best != nullptr && std::abs (dist - bestDist) <= 0.001f
                                        && best->implicit && ! pool.implicit;
        if (closer || tiePreferExplicit)
        {
            best = &pool;
            bestDist = dist;
        }
    }
    return best;
}

int VoiceEngine::pickVariant (Pool& pool, bool random)
{
    const int n = (int) pool.vars.size();
    if (n <= 0)
        return -1;
    if (n == 1)
        return 0;

    int index;
    if (random)
    {
        index = (int) (nextRand() % (uint32_t) n);
        if (index == pool.last)
            index = (index + 1) % n;
    }
    else
    {
        index = pool.rr % n;
        pool.rr = (pool.rr + 1) % n;
    }
    pool.last = index;
    return index;
}

void VoiceEngine::trigger (int voice,
                           float velocity,
                           int sampleOffset,
                           float tuneSemis,
                           float pitchJitterCents,
                           float timingJitterMs,
                           bool randomVariants)
{
    if (voice < 0 || voice >= kVoiceCount)
        return;

    auto& samples = voices[(size_t) voice];
    const float cents = (nextUnit() * 2.0f - 1.0f) * juce::jlimit (0.0f, 50.0f, pitchJitterCents);
    const float playedSemis = tuneSemis + cents / 100.0f;
    auto* pool = pickPool (samples, playedSemis);
    if (pool == nullptr)
        return;

    const int which = pickVariant (*pool, randomVariants);
    if (which < 0)
        return;

    if (kVoices[voice].chokeGroup != 0)
    {
        for (auto& hit : active)
            if (hit.on && hit.voice >= 0 && kVoices[hit.voice].chokeGroup == kVoices[voice].chokeGroup)
                hit.releasing = true;
    }

    int slot = -1;
    for (int i = 0; i < kPoolSize; ++i)
        if (! active[(size_t) i].on)
        {
            slot = i;
            break;
        }
    if (slot < 0)
    {
        int oldest = 0;
        double oldestPos = -1.0;
        for (int i = 0; i < kPoolSize; ++i)
        {
            if (active[(size_t) i].pos > oldestPos)
            {
                oldestPos = active[(size_t) i].pos;
                oldest = i;
            }
        }
        slot = oldest;
    }

    const auto& variant = pool->vars[(size_t) which];
    auto& hit = active[(size_t) slot];
    hit.on = true;
    hit.voice = voice;
    hit.left = variant.audio.getReadPointer (0);
    hit.right = variant.audio.getNumChannels() > 1 ? variant.audio.getReadPointer (1) : nullptr;
    hit.length = variant.audio.getNumSamples();
    hit.pos = 0.0;
    const double ratio = std::pow (2.0, (double) (playedSemis - (float) pool->semis) / 12.0);
    const double src = variant.sourceRate > 0.0 ? variant.sourceRate : deviceRate;
    hit.increment = ratio * src / (deviceRate > 0.0 ? deviceRate : 44100.0);
    const float jitterMs = juce::jmax (0.0f, timingJitterMs);
    const int jitterSamples = (int) (nextUnit() * jitterMs * 0.001f * (float) deviceRate);
    hit.delay = juce::jmax (0, sampleOffset) + jitterSamples;
    hit.velocity = juce::jlimit (0.0f, 1.0f, velocity) * 0.75f;
    hit.fade = 0.0f;
    hit.release = 1.0f;
    hit.releasing = false;
    hit.age = 0;
}

void VoiceEngine::render (juce::AudioBuffer<float>* voiceBuses, int numVoices, int numSamples)
{
    if (voiceBuses == nullptr || numSamples <= 0)
        return;

    for (auto& hit : active)
    {
        if (! hit.on || hit.left == nullptr || hit.voice < 0 || hit.voice >= numVoices)
            continue;

        auto* leftOut = voiceBuses[hit.voice].getWritePointer (0);
        auto* rightOut = voiceBuses[hit.voice].getNumChannels() > 1
                             ? voiceBuses[hit.voice].getWritePointer (1)
                             : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            if (! hit.on)
                break;
            if (hit.delay > 0)
            {
                --hit.delay;
                continue;
            }
            if (hit.pos >= (double) hit.length)
            {
                hit.on = false;
                break;
            }

            const int i0 = (int) hit.pos;
            const int i1 = juce::jmin (i0 + 1, hit.length - 1);
            const float frac = (float) (hit.pos - (double) i0);
            const float s0 = hit.left[i0];
            const float s1 = hit.left[i1];
            float sampleL = s0 + (s1 - s0) * frac;
            float sampleR = sampleL;
            if (hit.right != nullptr)
            {
                const float r0 = hit.right[i0];
                const float r1 = hit.right[i1];
                sampleR = r0 + (r1 - r0) * frac;
            }

            float gain = hit.velocity * hit.release * juce::jmin (1.0f, hit.fade);
            hit.fade += 0.125f;
            ++hit.age;
            if (hit.releasing)
            {
                hit.release -= 1.0f / 128.0f;
                if (hit.release <= 0.0f)
                {
                    hit.on = false;
                    break;
                }
            }

            const int remain = hit.length - i0;
            if (remain < 16)
                gain *= (float) remain / 16.0f;

            leftOut[i] += sampleL * gain;
            if (rightOut != nullptr)
                rightOut[i] += sampleR * gain;
            hit.pos += hit.increment;
        }
    }
}

int VoiceEngine::getVariantCount (int voice) const
{
    if (voice < 0 || voice >= kVoiceCount)
        return 0;
    return variantCounts[(size_t) voice];
}

} // namespace dz
