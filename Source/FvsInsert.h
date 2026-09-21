#pragma once

#include "FvsModules.h"

#include <array>

namespace dz
{

// Hosting boundary for one FVS insert slot.
// TODO(fvs): replace processSlot with the real Fashion Victim module that
// matches FvsModule. Do not link FVS_Host into this instrument. The picker
// id and the amount/bypass params stay; only the DSP body changes.
//
// "Mover" and "Spatializer" chosen here are short insert stubs. The aux
// Mover delay and the two-engine Spatializer live in FxChain, not in this slot.
struct SlotRequest
{
    int module = (int) FvsModule::none;
    float amount = 0.0f;
    bool bypass = false;
};

struct SlotState
{
    float lpL = 0.0f;
    float lpR = 0.0f;
    float envL = 0.0f;
    float envR = 0.0f;
    float prevL = 0.0f;
    float prevR = 0.0f;
    float phase = 0.0f;
    int slapPos = 0;
    std::array<float, 256> slapL {};
    std::array<float, 256> slapR {};
};

void resetSlot (SlotState& state);
void processSlot (const SlotRequest& request, SlotState& state, float* left, float* right, int numSamples, double sampleRate);

} // namespace dz
