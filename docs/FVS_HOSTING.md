# Fashion Victim modules inside Danger Zone

Danger Zone hosts a Fashion Victim Single **inside its own editor**. Opening an insert does not open a separate OS plugin window, and it does not show the FVS Host input/output chrome.

The DSP behind a slot is still the stub in `FvsInsert::processSlot`. The panel itself is real: you can open it, drag it, and close it in Standalone.

## Little door

Each pad strip and the master bus has two insert slots. Pick a module (anything other than None), then press **Open** on that slot.

- The panel shows **only the module face** — the middle effect area. There is no host input strip and no host output strip beside it.
- It is a child of the Danger Zone editor, drawn above the pages. Drag the title bar to move it. **X** closes it. It cannot be dragged fully off the editor.
- Several slots can be open at once. One panel per slot. Opening the same slot again brings that panel forward.
- **Open does nothing while the slot is None.**
- **Closing the panel only hides it.** Bypass, amount, and the selected module stay as they are. The insert keeps processing.
- **Choosing None unloads that insert** and closes its panel if it was open. Clearing the slot is the unload. Closing the panel is not.

## How the window is parented

`ModuleDoorHost` (`Source/Ui/ModuleDoor.cpp`) fills the editor above the footer and ignores clicks that are not on a panel, so the mixer underneath still works.

Each panel is a `juce::Component` child. It uses `ComponentDragger` and `ComponentBoundsConstrainer`. It is not a desktop `DocumentWindow` and it is not added with `addToDesktop()`, so the operating system does not get a second plugin window.

The face is sized to the FVS host **module hole**, about **452.6 × 790.5**, scaled to fit this editor (the stub face is 289 × 504 under a 28 px title). Those figures are the defaults in `FvsHostLayout`:

| Region | Rect (x, y, w, h) | Shown here |
| --- | --- | --- |
| Module hole | 119.7, 14.25, 452.6, 790.5 | Yes — this is the panel |
| Input strip | 35.5, 205, 93, 493 | No |
| Output strip | 564.5, 205, 93, 493 | No |

## What FVS still has to expose

Singles ship a full host editor today. Nothing in FVS Host provides a module-only component. There is no `createModuleEditor()`.

When a Single can do that, Danger Zone should parent it in this panel:

```cpp
std::unique_ptr<juce::Component> createModuleEditor();
```

That component is the effect UI only. No input strip, no output strip, no host shell.

Until then, the panel is a stub face at the module-hole aspect. If a later build only has the full host component, crop and center it to the module hole and still do not show the I/O strips. Do not link `FVS_Host` into Danger Zone to get that chrome. The insert DSP boundary stays `FvsModule` / `FvsInsert`.
