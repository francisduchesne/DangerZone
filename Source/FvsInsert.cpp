#include "FvsInsert.h"

#include <JuceHeader.h>

#include <cmath>

namespace dz
{

void resetSlot (SlotState& state)
{
    state = {};
}

namespace
{

float lowpass (float& state, float x, float coeff)
{
    state = (1.0f - coeff) * x + coeff * state;
    return state;
}

void slap (SlotState& state, float& left, float& right, float amount, double sampleRate)
{
    constexpr int size = 256;
    const int delay = juce::jlimit (1, size - 1, (int) (sampleRate * (0.004 + 0.012 * (double) amount)));
    const int read = (state.slapPos - delay + size) % size;
    const float fb = 0.25f * amount;
    const float wetL = state.slapL[(size_t) read];
    const float wetR = state.slapR[(size_t) read];
    state.slapL[(size_t) state.slapPos] = left + wetL * fb;
    state.slapR[(size_t) state.slapPos] = right + wetR * fb;
    state.slapPos = (state.slapPos + 1) % size;
    left += wetL * amount;
    right += wetR * amount;
}

} // namespace

void processSlot (const SlotRequest& request, SlotState& state, float* left, float* right, int numSamples, double sampleRate)
{
    if (left == nullptr || right == nullptr || numSamples <= 0)
        return;
    if (request.bypass || request.module == (int) FvsModule::none || request.amount < 0.001f)
        return;

    const float amount = juce::jlimit (0.0f, 1.0f, request.amount);
    const float sr = (float) (sampleRate > 0.0 ? sampleRate : 44100.0);
    const float lpCoeff = std::exp (-2.0f * juce::MathConstants<float>::pi * 180.0f / sr);
    const auto module = (FvsModule) request.module;

    for (int i = 0; i < numSamples; ++i)
    {
        float l = left[i];
        float r = right[i];

        switch (module)
        {
            case FvsModule::equalizer:
            {
                const float lowL = lowpass (state.lpL, l, lpCoeff);
                const float lowR = lowpass (state.lpR, r, lpCoeff);
                l += lowL * amount;
                r += lowR * amount;
                break;
            }
            case FvsModule::amplifier:
            {
                const float drive = 1.0f + amount * 10.0f;
                const float norm = std::tanh (drive);
                l = std::tanh (l * drive) / norm;
                r = std::tanh (r * drive) / norm;
                break;
            }
            case FvsModule::shifter:
            {
                state.phase += juce::MathConstants<float>::twoPi * (2.0f + amount * 8.0f) / sr;
                if (state.phase > juce::MathConstants<float>::twoPi)
                    state.phase -= juce::MathConstants<float>::twoPi;
                const float mod = 0.5f + 0.5f * std::sin (state.phase);
                constexpr int size = 256;
                const int delay = juce::jlimit (1, size - 1, 2 + (int) (mod * amount * 40.0f));
                const int read = (state.slapPos - delay + size) % size;
                state.slapL[(size_t) state.slapPos] = l;
                state.slapR[(size_t) state.slapPos] = r;
                state.slapPos = (state.slapPos + 1) % size;
                const float wetL = state.slapL[(size_t) read];
                const float wetR = state.slapR[(size_t) read];
                l = l * (1.0f - amount * 0.5f) + wetL * (amount * 0.5f);
                r = r * (1.0f - amount * 0.5f) + wetR * (amount * 0.5f);
                break;
            }
            case FvsModule::widener:
            {
                const float mid = 0.5f * (l + r);
                const float side = 0.5f * (l - r);
                const float width = 1.0f + amount * 2.0f;
                l = mid + side * width;
                r = mid - side * width;
                break;
            }
            case FvsModule::pusher:
            {
                const float dL = l - state.prevL;
                const float dR = r - state.prevR;
                state.prevL = l;
                state.prevR = r;
                l += dL * amount * 1.5f;
                r += dR * amount * 1.5f;
                break;
            }
            case FvsModule::toughener:
            {
                auto compress = [amount] (float x, float& env)
                {
                    const float rect = std::abs (x);
                    const float coeff = rect > env ? 0.08f : 0.003f;
                    env += (rect - env) * coeff;
                    const float thresh = juce::jmap (amount, 0.95f, 0.18f);
                    float reduction = 1.0f;
                    if (env > thresh && env > 0.0001f)
                        reduction = thresh / env;
                    reduction = juce::jmap (amount, 1.0f, reduction);
                    return x * reduction;
                };
                l = compress (l, state.envL);
                r = compress (r, state.envR);
                break;
            }
            case FvsModule::mover:
                slap (state, l, r, amount, sampleRate);
                break;
            case FvsModule::spatializer:
            {
                const float lowL = lowpass (state.lpL, l, lpCoeff);
                const float lowR = lowpass (state.lpR, r, lpCoeff);
                const float cross = amount * 0.55f;
                const float outL = l + lowR * cross;
                const float outR = r + lowL * cross;
                l = outL;
                r = outR;
                break;
            }
            case FvsModule::none:
            case FvsModule::count:
            default:
                break;
        }

        left[i] = l;
        right[i] = r;
    }
}

} // namespace dz
