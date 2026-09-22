#include "PluginProcessor.h"

#include "Mixer.h"
#include "Parameters.h"
#include "PluginEditor.h"
#include "PresetManager.h"

DangerZoneAudioProcessor::DangerZoneAudioProcessor()
    : juce::AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", dz::createParameterLayout())
{
    bindParams();
    engine.load (dz::findSamplesRoot());
    dz::applyStockPattern (sequencer);
    presetName = dz::kPresetStock;
}

void DangerZoneAudioProcessor::bindParams()
{
    auto grab = [this] (const juce::String& id) -> std::atomic<float>*
    {
        auto* value = apvts.getRawParameterValue (id);
        jassert (value != nullptr);
        return value;
    };

    tempo = grab ("tempo");
    patternLength = grab ("patternLength");
    swing = grab ("swing");
    playing = grab ("playing");
    variantMode = grab ("variantMode");
    timingJitterMs = grab ("timingJitterMs");
    pitchJitterCents = grab ("pitchJitterCents");
    masterLevel = grab ("masterLevel");
    masterIns1 = grab ("masterIns1");
    masterIns1Bypass = grab ("masterIns1Bypass");
    masterIns1Amount = grab ("masterIns1Amount");
    masterIns2 = grab ("masterIns2");
    masterIns2Bypass = grab ("masterIns2Bypass");
    masterIns2Amount = grab ("masterIns2Amount");
    moverTimeMs = grab ("moverTimeMs");
    moverFeedback = grab ("moverFeedback");
    moverReturn = grab ("moverReturn");
    spatSizeA = grab ("spatSizeA");
    spatSizeB = grab ("spatSizeB");
    spatLevelA = grab ("spatLevelA");
    spatLevelB = grab ("spatLevelB");
    spatReturn = grab ("spatReturn");

    for (int i = 0; i < dz::kVoiceCount; ++i)
    {
        auto& slot = voiceParams[(size_t) i];
        slot.level = grab (dz::voiceParam (i, "level"));
        slot.pan = grab (dz::voiceParam (i, "pan"));
        slot.mute = grab (dz::voiceParam (i, "mute"));
        slot.solo = grab (dz::voiceParam (i, "solo"));
        slot.tune = grab (dz::voiceParam (i, "tune"));
        slot.aux1 = grab (dz::voiceParam (i, "aux1"));
        slot.aux2 = grab (dz::voiceParam (i, "aux2"));
        slot.ins1 = grab (dz::voiceParam (i, "ins1"));
        slot.ins1Bypass = grab (dz::voiceParam (i, "ins1Bypass"));
        slot.ins1Amount = grab (dz::voiceParam (i, "ins1Amount"));
        slot.ins2 = grab (dz::voiceParam (i, "ins2"));
        slot.ins2Bypass = grab (dz::voiceParam (i, "ins2Bypass"));
        slot.ins2Amount = grab (dz::voiceParam (i, "ins2Amount"));
    }
}

void DangerZoneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    capacity = juce::jmax (samplesPerBlock, 512);
    for (auto& bus : voiceBus)
        bus.setSize (2, capacity);
    aux1.setSize (2, capacity);
    aux2.setSize (2, capacity);
    engine.setDeviceSampleRate (currentRate);
    sequencer.prepare (currentRate);
    fx.prepare (currentRate, capacity);
    for (auto& slot : slots)
        dz::resetSlot (slot);
    wasPlaying = false;
}

void DangerZoneAudioProcessor::releaseResources()
{
    fx.reset();
}

bool DangerZoneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::disabled();
}

void DangerZoneAudioProcessor::ensureCapacity (int numSamples)
{
    if (numSamples <= capacity)
        return;
    capacity = numSamples;
    for (auto& bus : voiceBus)
        bus.setSize (2, capacity, false, false, true);
    aux1.setSize (2, capacity, false, false, true);
    aux2.setSize (2, capacity, false, false, true);
    fx.ensureBlock (capacity);
}

dz::SlotRequest DangerZoneAudioProcessor::slotRequest (std::atomic<float>* module,
                                                       std::atomic<float>* bypass,
                                                       std::atomic<float>* amount) const
{
    dz::SlotRequest request;
    if (module != nullptr)
        request.module = juce::roundToInt (module->load());
    if (bypass != nullptr)
        request.bypass = bypass->load() >= 0.5f;
    if (amount != nullptr)
        request.amount = amount->load();
    return request;
}

void DangerZoneAudioProcessor::triggerVoice (int voice, float velocity, int sampleOffset)
{
    if (voice < 0 || voice >= dz::kVoiceCount)
        return;
    const float tune = voiceParams[(size_t) voice].tune != nullptr ? voiceParams[(size_t) voice].tune->load() : 0.0f;
    const float jitPitch = pitchJitterCents != nullptr ? pitchJitterCents->load() : 0.0f;
    const float jitTime = timingJitterMs != nullptr ? timingJitterMs->load() : 0.0f;
    const bool random = variantMode != nullptr && variantMode->load() >= 0.5f;
    engine.trigger (voice, velocity, sampleOffset, tune, jitPitch, jitTime, random);
}

void DangerZoneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2 || numSamples <= 0)
        return;

    ensureCapacity (numSamples);
    buffer.clear();
    aux1.clear (0, numSamples);
    aux2.clear (0, numSamples);
    for (auto& bus : voiceBus)
        bus.clear (0, numSamples);

    {
        int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
        const int ready = uiFifo.getNumReady();
        if (ready > 0)
        {
            uiFifo.prepareToRead (ready, start1, size1, start2, size2);
            for (int i = 0; i < size1; ++i)
                triggerVoice (uiSlots[(size_t) (start1 + i)], 1.0f, 0);
            for (int i = 0; i < size2; ++i)
                triggerVoice (uiSlots[(size_t) (start2 + i)], 1.0f, 0);
            uiFifo.finishedRead (size1 + size2);
        }
    }

    for (const auto metadata : midi)
    {
        const auto message = metadata.getMessage();
        const int offset = juce::jlimit (0, numSamples - 1, metadata.samplePosition);
        if (message.isNoteOn())
        {
            const int voice = dz::indexOfNote (message.getNoteNumber());
            if (voice >= 0)
                triggerVoice (voice, message.getFloatVelocity(), offset);
        }
        else if (message.isAllNotesOff() || message.isAllSoundOff())
        {
            engine.panic();
        }
        else if (message.isMidiClock() || message.isMidiStart() || message.isMidiStop() || message.isMidiContinue())
        {
            clock.handle (message);
        }
    }

    const bool isPlaying = playing != nullptr && playing->load() >= 0.5f;
    if (restartRequested.exchange (false) || (isPlaying && ! wasPlaying))
        sequencer.resetToStart();
    wasPlaying = isPlaying;

    const double bpm = tempo != nullptr ? (double) tempo->load() : 120.0;
    const int length = patternLength != nullptr ? juce::roundToInt (patternLength->load()) : dz::kMaxSteps;
    const float swing01 = swing != nullptr ? swing->load() / 100.0f : 0.0f;

    dz::Sequencer::Hit hits[256];
    const int hitCount = sequencer.collectHits (numSamples, bpm, length, swing01, isPlaying, hits, 256);
    for (int i = 0; i < hitCount; ++i)
        triggerVoice (hits[i].voice, 0.95f, hits[i].offset);

    engine.render (voiceBus.data(), dz::kVoiceCount, numSamples);

    for (int voice = 0; voice < dz::kVoiceCount; ++voice)
    {
        auto* left = voiceBus[(size_t) voice].getWritePointer (0);
        auto* right = voiceBus[(size_t) voice].getWritePointer (1);
        const auto& ptrs = voiceParams[(size_t) voice];
        dz::processSlot (slotRequest (ptrs.ins1, ptrs.ins1Bypass, ptrs.ins1Amount),
                         slots[(size_t) (voice * 2)],
                         left, right, numSamples, currentRate);
        dz::processSlot (slotRequest (ptrs.ins2, ptrs.ins2Bypass, ptrs.ins2Amount),
                         slots[(size_t) (voice * 2 + 1)],
                         left, right, numSamples, currentRate);
    }

    bool anySolo = false;
    for (int voice = 0; voice < dz::kVoiceCount; ++voice)
        if (voiceParams[(size_t) voice].solo != nullptr && voiceParams[(size_t) voice].solo->load() >= 0.5f)
            anySolo = true;

    for (int voice = 0; voice < dz::kVoiceCount; ++voice)
    {
        const auto& ptrs = voiceParams[(size_t) voice];
        dz::ChannelStrip strip;
        strip.level = ptrs.level != nullptr ? ptrs.level->load() : 0.0f;
        strip.pan = ptrs.pan != nullptr ? ptrs.pan->load() : 0.0f;
        strip.mute = ptrs.mute != nullptr && ptrs.mute->load() >= 0.5f;
        strip.solo = ptrs.solo != nullptr && ptrs.solo->load() >= 0.5f;
        strip.aux1 = ptrs.aux1 != nullptr ? ptrs.aux1->load() : 0.0f;
        strip.aux2 = ptrs.aux2 != nullptr ? ptrs.aux2->load() : 0.0f;
        const auto gain = dz::computeStrip (strip, anySolo);
        if (! gain.audible)
            continue;

        buffer.addFrom (0, 0, voiceBus[(size_t) voice], 0, 0, numSamples, gain.left);
        buffer.addFrom (1, 0, voiceBus[(size_t) voice], 1, 0, numSamples, gain.right);
        aux1.addFrom (0, 0, voiceBus[(size_t) voice], 0, 0, numSamples, gain.left * gain.aux1);
        aux1.addFrom (1, 0, voiceBus[(size_t) voice], 1, 0, numSamples, gain.right * gain.aux1);
        aux2.addFrom (0, 0, voiceBus[(size_t) voice], 0, 0, numSamples, gain.left * gain.aux2);
        aux2.addFrom (1, 0, voiceBus[(size_t) voice], 1, 0, numSamples, gain.right * gain.aux2);
    }

    fx.processMover (aux1, numSamples,
                     moverTimeMs != nullptr ? moverTimeMs->load() : 180.0f,
                     moverFeedback != nullptr ? moverFeedback->load() : 0.3f);
    dz::SpatializerSettings spat;
    spat.sizeA = spatSizeA != nullptr ? spatSizeA->load() : 0.32f;
    spat.sizeB = spatSizeB != nullptr ? spatSizeB->load() : 0.74f;
    spat.levelA = spatLevelA != nullptr ? spatLevelA->load() : 0.7f;
    spat.levelB = spatLevelB != nullptr ? spatLevelB->load() : 0.45f;
    fx.processSpatializer (aux2, numSamples, spat);

    const float moverGain = moverReturn != nullptr ? moverReturn->load() : 0.0f;
    const float spatGain = spatReturn != nullptr ? spatReturn->load() : 0.0f;
    buffer.addFrom (0, 0, aux1, 0, 0, numSamples, moverGain);
    buffer.addFrom (1, 0, aux1, 1, 0, numSamples, moverGain);
    buffer.addFrom (0, 0, aux2, 0, 0, numSamples, spatGain);
    buffer.addFrom (1, 0, aux2, 1, 0, numSamples, spatGain);

    dz::processSlot (slotRequest (masterIns1, masterIns1Bypass, masterIns1Amount),
                     slots[(size_t) (dz::kVoiceCount * 2)],
                     buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples, currentRate);
    dz::processSlot (slotRequest (masterIns2, masterIns2Bypass, masterIns2Amount),
                     slots[(size_t) (dz::kVoiceCount * 2 + 1)],
                     buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples, currentRate);

    buffer.applyGain (masterLevel != nullptr ? masterLevel->load() : 0.85f);
    for (int channel = 0; channel < 2; ++channel)
    {
        auto* data = buffer.getWritePointer (channel);
        for (int i = 0; i < numSamples; ++i)
            data[i] = juce::jlimit (-1.0f, 1.0f, data[i]);
    }
}

void DangerZoneAudioProcessor::triggerVoiceFromUi (int voice)
{
    if (voice < 0 || voice >= dz::kVoiceCount)
        return;
    int start1 = 0, size1 = 0, start2 = 0, size2 = 0;
    uiFifo.prepareToWrite (1, start1, size1, start2, size2);
    if (size1 > 0)
        uiSlots[(size_t) start1] = voice;
    else if (size2 > 0)
        uiSlots[(size_t) start2] = voice;
    uiFifo.finishedWrite (size1 + size2);
}

void DangerZoneAudioProcessor::requestSequencerRestart()
{
    restartRequested.store (true);
}

void DangerZoneAudioProcessor::reloadSamples()
{
    suspendProcessing (true);
    engine.panic();
    engine.load (dz::findSamplesRoot());
    engine.setDeviceSampleRate (currentRate);
    suspendProcessing (false);
}

juce::String DangerZoneAudioProcessor::getSamplesStatus() const
{
    return engine.getLoadReport();
}

void DangerZoneAudioProcessor::applyFactoryPreset (const juce::String& name)
{
    dz::resetParameters (*this);
    if (name == dz::kPresetProcessed)
    {
        const int kick = dz::indexOfId ("Kick");
        const int snare = dz::indexOfId ("Snare");
        const int hat = dz::indexOfId ("HatClosed");
        const int crash = dz::indexOfId ("Crash");
        dz::setRealValue (apvts, "swing", 18.0f);
        dz::setRealValue (apvts, dz::voiceParam (snare, "ins1"), (float) dz::FvsModule::amplifier);
        dz::setRealValue (apvts, dz::voiceParam (snare, "ins1Amount"), 0.45f);
        dz::setRealValue (apvts, dz::voiceParam (kick, "aux1"), 0.22f);
        dz::setRealValue (apvts, dz::voiceParam (snare, "aux2"), 0.40f);
        dz::setRealValue (apvts, dz::voiceParam (hat, "aux2"), 0.16f);
        dz::setRealValue (apvts, dz::voiceParam (crash, "aux2"), 0.28f);
        dz::setRealValue (apvts, "moverTimeMs", 240.0f);
        dz::setRealValue (apvts, "moverFeedback", 0.32f);
        dz::setRealValue (apvts, "moverReturn", 0.48f);
        dz::setRealValue (apvts, "spatSizeA", 0.30f);
        dz::setRealValue (apvts, "spatSizeB", 0.80f);
        dz::setRealValue (apvts, "spatLevelA", 0.60f);
        dz::setRealValue (apvts, "spatLevelB", 0.42f);
        dz::setRealValue (apvts, "spatReturn", 0.52f);
        dz::setRealValue (apvts, "masterIns1", (float) dz::FvsModule::widener);
        dz::setRealValue (apvts, "masterIns1Amount", 0.28f);
        dz::applyProcessedPattern (sequencer);
    }
    else
    {
        dz::applyStockPattern (sequencer);
    }

    const juce::ScopedLock lock (nameLock);
    presetName = name == dz::kPresetProcessed ? dz::kPresetProcessed : dz::kPresetStock;
}

juce::StringArray DangerZoneAudioProcessor::listPresetNames() const
{
    juce::StringArray names;
    names.add (dz::kPresetStock);
    names.add (dz::kPresetProcessed);
    for (const auto& user : dz::userPresetNames())
        names.add (user);
    return names;
}

bool DangerZoneAudioProcessor::saveUserPreset (const juce::String& name, juce::String& error)
{
    juce::String stored;
    {
        const juce::ScopedLock lock (nameLock);
        stored = name.trim();
    }
    if (! dz::saveUserPresetFile (apvts, sequencer, stored, error))
        return false;
    const juce::ScopedLock lock (nameLock);
    presetName = stored;
    return true;
}

bool DangerZoneAudioProcessor::loadPresetByName (const juce::String& name, juce::String& error)
{
    if (dz::isFactoryPreset (name))
    {
        applyFactoryPreset (name);
        return true;
    }

    juce::String loaded;
    if (! dz::loadUserPresetFile (apvts, sequencer, name, error, loaded))
        return false;
    const juce::ScopedLock lock (nameLock);
    presetName = loaded.isNotEmpty() ? loaded : name.trim();
    return true;
}

juce::String DangerZoneAudioProcessor::getPresetName() const
{
    const juce::ScopedLock lock (nameLock);
    return presetName;
}

void DangerZoneAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::String name;
    {
        const juce::ScopedLock lock (nameLock);
        name = presetName;
    }
    const auto tree = dz::captureState (apvts, sequencer, name);
    juce::MemoryOutputStream stream (destData, false);
    tree.writeToStream (stream);
}

void DangerZoneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return;
    const auto tree = juce::ValueTree::readFromData (data, (size_t) sizeInBytes);
    const auto loaded = dz::restoreState (apvts, sequencer, tree);
    if (loaded.isNotEmpty())
    {
        const juce::ScopedLock lock (nameLock);
        presetName = loaded;
    }
}

juce::AudioProcessorEditor* DangerZoneAudioProcessor::createEditor()
{
    return new DangerZoneAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DangerZoneAudioProcessor();
}
