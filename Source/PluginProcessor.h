#pragma once

#include <JuceHeader.h>

#include "FvsInsert.h"
#include "FxChain.h"
#include "MidiClockSync.h"
#include "PresetManager.h"
#include "Sequencer.h"
#include "VoiceEngine.h"

class DangerZoneAudioProcessor : public juce::AudioProcessor
{
public:
    DangerZoneAudioProcessor();
    ~DangerZoneAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Danger Zone"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getApvts() { return apvts; }
    dz::Sequencer& getSequencer() { return sequencer; }
    dz::VoiceEngine& getEngine() { return engine; }

    void triggerVoiceFromUi (int voice);
    void requestSequencerRestart();
    void reloadSamples();
    juce::String getSamplesStatus() const;
    int getMidiClockTicks() const { return clock.getTickCount(); }

    void applyFactoryPreset (const juce::String& name);
    juce::StringArray listPresetNames() const;
    bool saveUserPreset (const juce::String& name, juce::String& error);
    bool loadPresetByName (const juce::String& name, juce::String& error);
    juce::String getPresetName() const;

    juce::AudioProcessorValueTreeState apvts;

private:
    struct VoicePtrs
    {
        std::atomic<float>* level = nullptr;
        std::atomic<float>* pan = nullptr;
        std::atomic<float>* mute = nullptr;
        std::atomic<float>* solo = nullptr;
        std::atomic<float>* tune = nullptr;
        std::atomic<float>* aux1 = nullptr;
        std::atomic<float>* aux2 = nullptr;
        std::atomic<float>* ins1 = nullptr;
        std::atomic<float>* ins1Bypass = nullptr;
        std::atomic<float>* ins1Amount = nullptr;
        std::atomic<float>* ins2 = nullptr;
        std::atomic<float>* ins2Bypass = nullptr;
        std::atomic<float>* ins2Amount = nullptr;
    };

    void bindParams();
    void ensureCapacity (int numSamples);
    void triggerVoice (int voice, float velocity, int sampleOffset);
    dz::SlotRequest slotRequest (std::atomic<float>* module,
                                 std::atomic<float>* bypass,
                                 std::atomic<float>* amount) const;

    dz::VoiceEngine engine;
    dz::Sequencer sequencer;
    dz::FxChain fx;
    dz::MidiClockSync clock;

    std::array<dz::SlotState, dz::kVoiceCount * 2 + 2> slots {};
    std::array<juce::AudioBuffer<float>, dz::kVoiceCount> voiceBus;
    juce::AudioBuffer<float> aux1;
    juce::AudioBuffer<float> aux2;
    std::array<VoicePtrs, dz::kVoiceCount> voiceParams {};

    std::atomic<float>* tempo = nullptr;
    std::atomic<float>* patternLength = nullptr;
    std::atomic<float>* swing = nullptr;
    std::atomic<float>* playing = nullptr;
    std::atomic<float>* variantMode = nullptr;
    std::atomic<float>* timingJitterMs = nullptr;
    std::atomic<float>* pitchJitterCents = nullptr;
    std::atomic<float>* masterLevel = nullptr;
    std::atomic<float>* masterIns1 = nullptr;
    std::atomic<float>* masterIns1Bypass = nullptr;
    std::atomic<float>* masterIns1Amount = nullptr;
    std::atomic<float>* masterIns2 = nullptr;
    std::atomic<float>* masterIns2Bypass = nullptr;
    std::atomic<float>* masterIns2Amount = nullptr;
    std::atomic<float>* moverTimeMs = nullptr;
    std::atomic<float>* moverFeedback = nullptr;
    std::atomic<float>* moverReturn = nullptr;
    std::atomic<float>* spatSizeA = nullptr;
    std::atomic<float>* spatSizeB = nullptr;
    std::atomic<float>* spatLevelA = nullptr;
    std::atomic<float>* spatLevelB = nullptr;
    std::atomic<float>* spatReturn = nullptr;

    int capacity = 0;
    double currentRate = 44100.0;
    bool wasPlaying = false;
    std::atomic<bool> restartRequested { false };

    juce::AbstractFifo uiFifo { 128 };
    std::array<int, 128> uiSlots {};

    juce::CriticalSection nameLock;
    juce::String presetName { dz::kPresetStock };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DangerZoneAudioProcessor)
};
