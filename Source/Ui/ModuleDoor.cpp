#include "ModuleDoor.h"

#include "FvsModules.h"
#include "VoiceBank.h"

namespace dz
{
namespace
{
// FVS host layout defaults (FvsHostLayout). The hole is the module face.
// Input and output rects are the chrome this panel does not show.
constexpr float kModuleHoleW = 452.6f;
constexpr float kModuleHoleH = 790.5f;
constexpr int kTitleH = 28;
constexpr int kFaceH = 504;
constexpr int kFaceW = (int) (kFaceH * (kModuleHoleW / kModuleHoleH) + 0.5f);

juce::String insertParamId (int voice, int slot)
{
    const char* leaf = slot == 0 ? "ins1" : "ins2";
    if (voice < 0)
        return slot == 0 ? "masterIns1" : "masterIns2";
    return voiceParam (voice, leaf);
}

bool parseInsertParam (const juce::String& id, int& voice, int& slot)
{
    if (id == "masterIns1")
    {
        voice = -1;
        slot = 0;
        return true;
    }
    if (id == "masterIns2")
    {
        voice = -1;
        slot = 1;
        return true;
    }

    for (int i = 0; i < kVoiceCount; ++i)
    {
        if (id == voiceParam (i, "ins1"))
        {
            voice = i;
            slot = 0;
            return true;
        }
        if (id == voiceParam (i, "ins2"))
        {
            voice = i;
            slot = 1;
            return true;
        }
    }

    return false;
}

juce::String doorTitle (int voice, int slot)
{
    const juce::String who = voice < 0 ? "Master" : juce::String (kVoices[voice].label);
    return who + "  ·  Insert " + juce::String (slot + 1);
}

const char* moduleName (int index)
{
    if (index <= 0 || index >= (int) FvsModule::count)
        return kFvsModuleNames[0];
    return kFvsModuleNames[index];
}
} // namespace

struct ModuleDoorHost::Door : public juce::Component
{
    Door (ModuleDoorHost& ownerIn, int voiceIn, int slotIn, int moduleIn)
        : owner (ownerIn), voice (voiceIn), slot (slotIn), module (moduleIn), face (*this)
    {
        setOpaque (true);
        constrainer.setSizeLimits (kFaceW, kTitleH + kFaceH, kFaceW, kTitleH + kFaceH);
        constrainer.setMinimumOnscreenAmounts (kTitleH + kFaceH, 48, kTitleH, 48);

        face.moduleName = moduleName (moduleIn);
        close.setButtonText ("X");
        close.setTooltip ("Close the panel. The insert stays loaded.");
        close.onClick = [this] { owner.requestClose (voice, slot); };
        addAndMakeVisible (close);
        addAndMakeVisible (face);
    }

    void show (int moduleIn)
    {
        module = moduleIn;
        face.moduleName = moduleName (module);
        face.repaint();
        repaint();
    }

    void keepOnScreen()
    {
        constrainer.checkComponentBounds (this);
    }

    void paint (juce::Graphics& g) override
    {
        auto bar = getLocalBounds().removeFromTop (kTitleH);
        g.setColour (findColour (juce::TextButton::buttonColourId));
        g.fillRect (bar);
        g.setColour (findColour (juce::Label::textColourId));
        g.setFont (juce::Font (juce::FontOptions (14.0f).withStyle ("Bold")));
        g.drawFittedText (doorTitle (voice, slot),
                          bar.withTrimmedRight (close.getWidth() + 6).reduced (8, 0),
                          juce::Justification::centredLeft,
                          1);
        g.setColour (findColour (juce::ComboBox::outlineColourId));
        g.drawRect (getLocalBounds(), 1);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto bar = area.removeFromTop (kTitleH);
        close.setBounds (bar.removeFromRight (28).reduced (3));
        face.setBounds (area.reduced (1, 0).withTrimmedBottom (1));
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        setMouseCursor (e.y < kTitleH ? juce::MouseCursor::DraggingHandCursor
                                      : juce::MouseCursor::NormalCursor);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        toFront (false);
        if (e.y >= kTitleH || close.getBounds().contains (e.getPosition()))
            return;
        dragging = true;
        dragger.startDraggingComponent (this, e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragging)
            dragger.dragComponent (this, e, &constrainer);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        dragging = false;
    }

    struct Face : public juce::Component
    {
        Door& door;
        juce::String moduleName;

        explicit Face (Door& doorIn)
            : door (doorIn)
        {
            setOpaque (true);
        }

        void paint (juce::Graphics& g) override
        {
            const auto text = findColour (juce::Label::textColourId);
            const auto outline = findColour (juce::ComboBox::outlineColourId);
            g.fillAll (findColour (juce::ResizableWindow::backgroundColourId));

            auto inner = getLocalBounds().reduced (16);
            g.setColour (outline);
            g.drawRect (inner, 1);

            g.setColour (text);
            g.setFont (juce::Font (juce::FontOptions (22.0f).withStyle ("Bold")));
            g.drawFittedText (moduleName, inner.removeFromTop (56), juce::Justification::centred, 1);

            g.setFont (juce::Font (juce::FontOptions (15.0f)));
            g.drawFittedText ("Module face", inner.removeFromTop (28), juce::Justification::centred, 1);

            g.setFont (juce::Font (juce::FontOptions (13.0f)));
            g.drawFittedText ("No host input or output strip.\n"
                              "Closing this panel keeps the insert loaded.",
                              inner.reduced (12, 8),
                              juce::Justification::centred,
                              4);
        }

        void mouseDown (const juce::MouseEvent&) override
        {
            door.toFront (false);
        }
    };

    ModuleDoorHost& owner;
    int voice = 0;
    int slot = 0;
    int module = 0;
    bool dragging = false;
    Face face;
    juce::TextButton close;
    juce::ComponentDragger dragger;
    juce::ComponentBoundsConstrainer constrainer;
};

void ModuleDoorHost::requestClose (int voice, int slot)
{
    juce::Component::SafePointer<ModuleDoorHost> safe (this);
    juce::MessageManager::callAsync ([safe, voice, slot]
    {
        if (safe != nullptr)
            safe->closeNow (voice, slot);
    });
}

ModuleDoorHost::ModuleDoorHost (juce::AudioProcessorValueTreeState& state)
    : apvts (state)
{
    setInterceptsMouseClicks (false, true);
    apvts.addParameterListener ("masterIns1", this);
    apvts.addParameterListener ("masterIns2", this);
    for (int i = 0; i < kVoiceCount; ++i)
    {
        apvts.addParameterListener (voiceParam (i, "ins1"), this);
        apvts.addParameterListener (voiceParam (i, "ins2"), this);
    }
}

ModuleDoorHost::~ModuleDoorHost()
{
    apvts.removeParameterListener ("masterIns1", this);
    apvts.removeParameterListener ("masterIns2", this);
    for (int i = 0; i < kVoiceCount; ++i)
    {
        apvts.removeParameterListener (voiceParam (i, "ins1"), this);
        apvts.removeParameterListener (voiceParam (i, "ins2"), this);
    }
}

void ModuleDoorHost::open (int voice, int slot)
{
    const auto id = insertParamId (voice, slot);
    const auto* raw = apvts.getRawParameterValue (id);
    const int index = raw != nullptr ? juce::roundToInt (raw->load()) : 0;
    if (index <= 0)
        return;

    for (auto* door : doors)
    {
        if (door->voice == voice && door->slot == slot)
        {
            door->show (index);
            door->toFront (false);
            return;
        }
    }

    auto* door = doors.add (new Door (*this, voice, slot, index));
    addAndMakeVisible (door);
    door->setSize (kFaceW, kTitleH + kFaceH);
    const int n = (doors.size() - 1) % 8;
    door->setTopLeftPosition (24 + n * 28, 36 + n * 28);
    door->keepOnScreen();
    door->toFront (false);
}

void ModuleDoorHost::resized()
{
    for (auto* door : doors)
        door->keepOnScreen();
}

void ModuleDoorHost::parameterChanged (const juce::String& parameterID, float newValue)
{
    const int index = juce::roundToInt (newValue);
    const auto id = parameterID;
    juce::Component::SafePointer<ModuleDoorHost> safe (this);
    juce::MessageManager::callAsync ([safe, id, index]
    {
        if (safe != nullptr)
            safe->applyModuleChange (id, index);
    });
}

void ModuleDoorHost::applyModuleChange (const juce::String& parameterID, int moduleIndex)
{
    int voice = 0;
    int slot = 0;
    if (! parseInsertParam (parameterID, voice, slot))
        return;

    Door* found = nullptr;
    for (auto* door : doors)
    {
        if (door->voice == voice && door->slot == slot)
        {
            found = door;
            break;
        }
    }

    if (found == nullptr)
        return;

    if (moduleIndex <= 0)
        closeNow (voice, slot);
    else
        found->show (moduleIndex);
}

void ModuleDoorHost::closeNow (int voice, int slot)
{
    for (int i = doors.size(); --i >= 0;)
        if (doors[i]->voice == voice && doors[i]->slot == slot)
            doors.remove (i);
}

} // namespace dz
