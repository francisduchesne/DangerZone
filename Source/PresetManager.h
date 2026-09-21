#pragma once

#include <JuceHeader.h>

namespace dz
{

class Sequencer;

inline constexpr const char* kPresetStock = "Stock Linn";
inline constexpr const char* kPresetProcessed = "Processed";

bool isFactoryPreset (const juce::String& name);
juce::File userPresetDirectory();
juce::StringArray userPresetNames();

void applyStockPattern (Sequencer& sequencer);
void applyProcessedPattern (Sequencer& sequencer);
void resetParameters (juce::AudioProcessor& processor);
void setRealValue (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, float realValue);

juce::ValueTree captureState (juce::AudioProcessorValueTreeState& apvts,
                              const Sequencer& sequencer,
                              const juce::String& presetName);
// Returns the preset name stored in the tree. Pattern generation is bumped.
juce::String restoreState (juce::AudioProcessorValueTreeState& apvts,
                           Sequencer& sequencer,
                           const juce::ValueTree& state);

bool saveUserPresetFile (juce::AudioProcessorValueTreeState& apvts,
                         const Sequencer& sequencer,
                         const juce::String& presetName,
                         juce::String& error);
bool loadUserPresetFile (juce::AudioProcessorValueTreeState& apvts,
                         Sequencer& sequencer,
                         const juce::String& presetName,
                         juce::String& error,
                         juce::String& loadedName);

} // namespace dz
