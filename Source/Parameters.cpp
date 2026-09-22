#include "Parameters.h"

#include "FvsModules.h"
#include "VoiceBank.h"

namespace dz
{
namespace
{

juce::StringArray moduleChoices()
{
    juce::StringArray names;
    for (const char* name : kFvsModuleNames)
        names.add (name);
    return names;
}

void addFloat (juce::AudioProcessorValueTreeState::ParameterLayout& layout,
               const juce::String& id,
               const juce::String& name,
               float min,
               float max,
               float def,
               float interval = 0.0f)
{
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { id, 1 },
        name,
        juce::NormalisableRange<float> (min, max, interval),
        def));
}

void addBool (juce::AudioProcessorValueTreeState::ParameterLayout& layout,
              const juce::String& id,
              const juce::String& name,
              bool def)
{
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 }, name, def));
}

void addChoice (juce::AudioProcessorValueTreeState::ParameterLayout& layout,
                const juce::String& id,
                const juce::String& name,
                const juce::StringArray& choices,
                int def)
{
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { id, 1 }, name, choices, def));
}

void addVoiceStrip (juce::AudioProcessorValueTreeState::ParameterLayout& layout, int index)
{
    const auto label = juce::String (kVoices[index].label);
    const auto modules = moduleChoices();

    addFloat (layout, voiceParam (index, "level"), label + " Level", 0.0f, 1.0f, defaultLevelFor (index));
    addFloat (layout, voiceParam (index, "pan"), label + " Pan", -1.0f, 1.0f, 0.0f);
    addBool (layout, voiceParam (index, "mute"), label + " Mute", false);
    addBool (layout, voiceParam (index, "solo"), label + " Solo", false);
    addFloat (layout, voiceParam (index, "tune"), label + " Tune", -12.0f, 12.0f, 0.0f, 0.01f);
    addFloat (layout, voiceParam (index, "aux1"), label + " Aux 1 (Mover)", 0.0f, 1.0f, 0.0f);
    addFloat (layout, voiceParam (index, "aux2"), label + " Aux 2 (Spatializer)", 0.0f, 1.0f, 0.0f);

    addChoice (layout, voiceParam (index, "ins1"), label + " Insert 1", modules, 0);
    addBool (layout, voiceParam (index, "ins1Bypass"), label + " Insert 1 Bypass", false);
    addFloat (layout, voiceParam (index, "ins1Amount"), label + " Insert 1 Amount", 0.0f, 1.0f, 0.0f);

    addChoice (layout, voiceParam (index, "ins2"), label + " Insert 2", modules, 0);
    addBool (layout, voiceParam (index, "ins2Bypass"), label + " Insert 2 Bypass", false);
    addFloat (layout, voiceParam (index, "ins2Amount"), label + " Insert 2 Amount", 0.0f, 1.0f, 0.0f);
}

} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    addFloat (layout, "tempo", "Tempo", 40.0f, 240.0f, 120.0f, 0.1f);
    layout.add (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { "patternLength", 1 }, "Pattern Length", 1, kMaxSteps, kMaxSteps));
    addFloat (layout, "swing", "Swing", 0.0f, 100.0f, 0.0f, 0.1f);
    addBool (layout, "playing", "Playing", false);
    addChoice (layout, "variantMode", "Variant Mode", juce::StringArray { "Round Robin", "Random" }, 0);
    addFloat (layout, "timingJitterMs", "Timing Jitter", 0.0f, 20.0f, 0.0f, 0.1f);
    addFloat (layout, "pitchJitterCents", "Pitch Jitter", 0.0f, 50.0f, 0.0f, 0.1f);
    addBool (layout, "midiClockSync", "MIDI Clock Sync", false);

    addFloat (layout, "masterLevel", "Master Level", 0.0f, 1.0f, 0.85f);
    const auto modules = moduleChoices();
    addChoice (layout, "masterIns1", "Master Insert 1", modules, 0);
    addBool (layout, "masterIns1Bypass", "Master Insert 1 Bypass", false);
    addFloat (layout, "masterIns1Amount", "Master Insert 1 Amount", 0.0f, 1.0f, 0.0f);
    addChoice (layout, "masterIns2", "Master Insert 2", modules, 0);
    addBool (layout, "masterIns2Bypass", "Master Insert 2 Bypass", false);
    addFloat (layout, "masterIns2Amount", "Master Insert 2 Amount", 0.0f, 1.0f, 0.0f);

    // Aux 1 return is the Mover delay. Aux 2 return is the Spatializer (two reverbs).
    addFloat (layout, "moverTimeMs", "Mover Time", 1.0f, 1000.0f, 180.0f, 0.1f);
    addFloat (layout, "moverFeedback", "Mover Feedback", 0.0f, 0.85f, 0.30f);
    addFloat (layout, "moverReturn", "Mover Return", 0.0f, 1.0f, 0.55f);

    addFloat (layout, "spatSizeA", "Spatializer A Size", 0.0f, 1.0f, 0.32f);
    addFloat (layout, "spatSizeB", "Spatializer B Size", 0.0f, 1.0f, 0.74f);
    addFloat (layout, "spatLevelA", "Spatializer A Level", 0.0f, 1.0f, 0.70f);
    addFloat (layout, "spatLevelB", "Spatializer B Level", 0.0f, 1.0f, 0.45f);
    addFloat (layout, "spatReturn", "Spatializer Return", 0.0f, 1.0f, 0.55f);

    for (int i = 0; i < kVoiceCount; ++i)
        addVoiceStrip (layout, i);

    return layout;
}

} // namespace dz
