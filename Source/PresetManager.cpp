#include "PresetManager.h"

#include "Sequencer.h"
#include "VoiceBank.h"

namespace dz
{
namespace
{

void clearPattern (Sequencer& sequencer)
{
    for (int i = 0; i < kVoiceCount; ++i)
        sequencer.setMask (i, 0);
}

void setVoiceMask (Sequencer& sequencer, const char* id, uint16_t mask)
{
    const int index = indexOfId (id);
    if (index >= 0)
        sequencer.setMask (index, mask);
}

} // namespace

bool isFactoryPreset (const juce::String& name)
{
    return name == kPresetStock || name == kPresetProcessed;
}

juce::File userPresetDirectory()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("Arcades")
                   .getChildFile ("Danger Zone")
                   .getChildFile ("Presets");
    dir.createDirectory();
    return dir;
}

juce::StringArray userPresetNames()
{
    juce::StringArray names;
    for (const auto& file : userPresetDirectory().findChildFiles (juce::File::findFiles, false, "*.xml"))
        names.add (file.getFileNameWithoutExtension());
    names.sort (true);
    return names;
}

void applyStockPattern (Sequencer& sequencer)
{
    clearPattern (sequencer);
    setVoiceMask (sequencer, "Kick", 0x1111);      // 1, 5, 9, 13
    setVoiceMask (sequencer, "Snare", 0x1010);     // 5, 13
    setVoiceMask (sequencer, "HatClosed", 0x5555); // straight 8ths
    sequencer.bumpGeneration();
}

void applyProcessedPattern (Sequencer& sequencer)
{
    clearPattern (sequencer);
    setVoiceMask (sequencer, "Kick", 0x0101);      // 1 and 9
    setVoiceMask (sequencer, "Snare", 0x1010);     // 5, 13
    setVoiceMask (sequencer, "HatClosed", 0x5555);
    setVoiceMask (sequencer, "HatOpen", 0x4444);   // off-8ths
    setVoiceMask (sequencer, "Claps", 0x1010);
    setVoiceMask (sequencer, "TomHi", 0xC000);     // last two steps
    sequencer.bumpGeneration();
}

void resetParameters (juce::AudioProcessor& processor)
{
    for (auto* param : processor.getParameters())
        if (param != nullptr)
            param->setValueNotifyingHost (param->getDefaultValue());
}

void setRealValue (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, float realValue)
{
    if (auto* param = apvts.getParameter (id))
        param->setValueNotifyingHost (param->convertTo0to1 (realValue));
}

juce::ValueTree captureState (juce::AudioProcessorValueTreeState& apvts,
                              const Sequencer& sequencer,
                              const juce::String& presetName)
{
    juce::ValueTree root ("DangerZone");
    root.setProperty ("version", 1, nullptr);
    root.setProperty ("presetName", presetName, nullptr);
    root.appendChild (apvts.copyState(), nullptr);
    root.appendChild (sequencer.toValueTree(), nullptr);
    return root;
}

juce::String restoreState (juce::AudioProcessorValueTreeState& apvts,
                           Sequencer& sequencer,
                           const juce::ValueTree& state)
{
    if (! state.isValid())
        return {};

    const auto params = state.getChildWithName ("PARAMS");
    if (params.isValid())
        apvts.replaceState (params);

    sequencer.fromValueTree (state.getChildWithName ("PATTERN"));
    return state.getProperty ("presetName", kPresetStock).toString();
}

bool saveUserPresetFile (juce::AudioProcessorValueTreeState& apvts,
                         const Sequencer& sequencer,
                         const juce::String& presetName,
                         juce::String& error)
{
    const auto trimmed = presetName.trim();
    if (trimmed.isEmpty())
    {
        error = "Preset name is empty.";
        return false;
    }
    if (isFactoryPreset (trimmed))
    {
        error = "Factory names are reserved.";
        return false;
    }

    const auto file = userPresetDirectory().getChildFile (juce::File::createLegalFileName (trimmed) + ".xml");
    const auto tree = captureState (apvts, sequencer, trimmed);
    const std::unique_ptr<juce::XmlElement> xml (tree.createXml());
    if (xml == nullptr || ! xml->writeTo (file))
    {
        error = "Could not write " + file.getFullPathName();
        return false;
    }
    return true;
}

bool loadUserPresetFile (juce::AudioProcessorValueTreeState& apvts,
                         Sequencer& sequencer,
                         const juce::String& presetName,
                         juce::String& error,
                         juce::String& loadedName)
{
    const auto file = userPresetDirectory().getChildFile (juce::File::createLegalFileName (presetName.trim()) + ".xml");
    if (! file.existsAsFile())
    {
        error = "No preset file named \"" + presetName + "\".";
        return false;
    }

    const std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));
    if (xml == nullptr)
    {
        error = "Preset XML could not be read.";
        return false;
    }

    loadedName = restoreState (apvts, sequencer, juce::ValueTree::fromXml (*xml));
    if (loadedName.isEmpty())
        loadedName = presetName.trim();
    return true;
}

} // namespace dz
