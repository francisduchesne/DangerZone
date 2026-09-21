# Danger Zone

Arcade-branded LinnDrum-style drum instrument by Arcades. VST3 and Standalone. Not an FVS_Host module.

Fashion Victim processing is named in the mixer (insert slots, Mover on aux 1, two-engine Spatializer on aux 2) and implemented as stubs. The graph and the parameters are real. See [docs/MIXER.md](docs/MIXER.md), [docs/PRODUCT_BRIEF.md](docs/PRODUCT_BRIEF.md), and [docs/LINNDRUM_RESEARCH.md](docs/LINNDRUM_RESEARCH.md).

## Identity

- Product: Danger Zone
- Company: Arcades
- Bundle: `com.arcades.dangerzone`
- Manufacturer code: `Arcd`
- Plugin code: `Dzln`
- Synth, MIDI in, stereo out
- Formats: VST3 + Standalone
- Category: Instrument / Drum

Editor pages, stock JUCE widgets, no custom art: **Linn | Mythical | Sequencer | Mixer | FX | Presets**.

## Build (Windows)

Requirements: CMake 3.22 or newer, a C++20 compiler, git. JUCE 8.0.8 is fetched at configure time (not vendored).

Visual Studio 2026 (toolset 18) is what this machine uses:

```bat
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release --target DangerZone_VST3 DangerZone_Standalone
```

Other generators work if they can compile C++20. Configure from a Visual Studio developer environment if the generator is Ninja and `cl` is not already on `PATH`.

Release artefacts:

- `build/DangerZone_artefacts/Release/VST3/Danger Zone.vst3`
- `build/DangerZone_artefacts/Release/Standalone/Danger Zone.exe`

The build copies `Resources/Samples` next to each binary, under `Resources/Samples`. Copy the whole `.vst3` bundle if you move the plugin. `COPY_PLUGIN_AFTER_BUILD` is off, so CMake does not need a system VST3 folder.

If CMake or a C++ toolchain is missing, stop there and install them. Do not block on a second machine. JUCE itself only needs network (or a CMake `FETCHCONTENT_SOURCE_DIR_JUCE` override) during configure.

## Samples

Drop WAVs here (this layout is already in the repo for the procedural placeholders):

```
Resources/Samples/<Voice>/<voice>_v1.wav
Resources/Samples/Snare/snare_v1.wav
Resources/Samples/Snare/snare_p0_v1.wav    optional pitch layer, 0 semitones
Resources/Samples/Snare/snare_pm3_v1.wav   optional pitch layer, -3 semitones
Resources/Samples/Mythical/<Voice>/        alternate folder for the era page
```

Voice folder names: `Kick`, `Snare`, `Sidestick`, `Rim`, `HatClosed`, `HatOpen`, `Cabasa`, `Tambourine`, `CongaLo`, `CongaHi`, `Cowbell`, `Claps`, `TomLo`, `TomMid`, `TomHi`, `Ride`, `Crash`, `MythTomA`, `MythTomB`, `EraKick`, `EraSnare`, `EraHat`, `EraPerc`.

The engine searches next to the executable or VST3 binary (`Resources/Samples`), then walks parent folders for a repo checkout. **Reload samples** on the Linn or Mythical page picks up new files without a rebuild.

Round-robin is the default (`variantMode`). Random is the other choice. Either way a repeated hit walks the variants instead of restarting one file. Timing jitter and pitch jitter are on the FX page.

## Play it

1. Open the Standalone, or load the VST3.
2. Press **Play** on the Sequencer page. Stock Linn is a dry pattern: kick on the beat, snare on the backbeat, closed hat on the 8ths.
3. Pads on Linn audition a voice. Mythical pads stay quiet until those folders have WAVs.
4. Mixer: fader, pan, mute, solo, aux 1 (Mover), aux 2 (Spatializer), two insert pickers per voice, two insert pickers on the master.
5. FX: Mover time / feedback / return, Spatializer engine A and engine B, insert amount for the selected voice.
6. Presets: **Stock Linn**, **Processed**, and Save user preset.

MIDI notes use the map in [docs/LINNDRUM_RESEARCH.md](docs/LINNDRUM_RESEARCH.md) (kick = 36, snare = 38, closed hat = 42, …).

## Layout

```
Source/PluginProcessor.*    graph, MIDI, state
Source/PluginEditor.*       tabs
Source/VoiceEngine.*        multi-sample playback
Source/Sequencer.*          16-step grid
Source/Mixer.*              fader / pan / mute / solo / send gains
Source/FvsInsert.*          insert-slot stubs (hosting boundary)
Source/FxChain.*            aux Mover + two-engine Spatializer
Source/PresetManager.*      Stock Linn, Processed, user XML
Source/MidiClockSync.h      clock hook, not yet followed
Source/Ui/                  Linn, Mythical, Sequencer, Mixer, FX, Presets
```

## TODO

- Real multi-samples of the orange/black Linn. About six variants per hit. Snare pitch layers. Placeholder WAVs are only here so the engine has something to play.
- Mythical / era takes in the empty slots.
- Replace `FvsInsert::processSlot` and `FxChain` with real Fashion Victim DSP. Do not depend on FVS_Host. Keep the parameter ids in [docs/MIXER.md](docs/MIXER.md).
- MIDI clock follow when `midiClockSync` is enabled. Host playhead follow after that.
- Orange/black retro-wave LookAndFeel. No custom art in this scaffold.
- Measure Linn shuffle A–F if a single swing percent is not enough.
- Optional per-voice output buses.
