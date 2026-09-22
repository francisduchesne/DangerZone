# Danger Zone — product brief

Danger Zone is an Arcades virtual instrument: a LinnDrum-style drum machine branded for Fashion Victim, built from the orange/black LinnDrum Francis owns. This repo is the instrument itself (VST3 + Standalone). It is not an FVS_Host effect module.

## Identity

| | |
| --- | --- |
| Name | Danger Zone |
| Company | Arcades |
| Bundle | `com.arcades.dangerzone` |
| Manufacturer code | `Arcd` |
| Plugin code | `Dzln` |
| Type | Instrument, MIDI in, stereo out |
| Formats | VST3, Standalone |
| Category | Instrument / Drum |

## What this scaffold already does

- Linn voice bank: Kick, Snare, Sidestick, Rim, Hat Closed, Hat Open, Cabasa, Tambourine, Conga Lo, Conga Hi, Cowbell, Claps, Tom Lo, Tom Mid, Tom Hi, Ride, Crash.
- Mythical / era bank (empty until samples are dropped): Myth Tom A, Myth Tom B, Era Kick, Era Snare, Era Hat, Era Perc.
- Multi-sample playback from `Resources/Samples/<Voice>/`. Round-robin or random, so one file is not retriggered as a machine gun. Optional pitch-layer files. Timing and pitch jitter.
- Closed and open hat share a choke group.
- 16-step sequencer, internal tempo, swing, pattern length, play / stop. MIDI notes play the same voices. MIDI clock is counted and stored, not followed yet.
- Mixer graph in [MIXER.md](MIXER.md), matching the strip below. FVS bodies are stubs. The graph and the parameters are real.
- Factory presets **Stock Linn** and **Processed**, plus user XML presets and full APVTS + pattern state in the host chunk.
- Stock JUCE widgets. No custom art. Retro-wave / orange-black Fashion Victim look is documentation and naming only for now.

Placeholder WAVs in `Resources/Samples/` are procedural stand-ins so the engine and the sequencer make sound. They are not recordings of the Linn.

## Mixer

Every LinnDrum pad has a channel strip. Mythical slots use the same strip.

- Fader and pan (plus mute and solo on the scaffold strip).
- Two insert slots. Each one can select any Fashion Victim Single effect. The exact menu is TBD, so the picker is stubbed with a provisional list (see [MIXER.md](MIXER.md)). Open shows the module face in a floating panel inside this editor. The FVS Host input and output strips stay out. See [FVS_HOSTING.md](FVS_HOSTING.md).
- Two aux sends, pre-filled: Aux 1 is Mover (delay). Aux 2 is Spatializer (reverb) with two reverb engines on that return.
- The master bus has two FVS insert slots, same stub picker.

Real FVS plugin hosting waits. The strip, the aux returns, and the master inserts are the placeholders that hosting will fill.

## What stays out on purpose

- No dependency on `FVS_Host` or any Fashion Victim Single module project.
- No cassette, song chain, click track, or DIN-sync hardware. The DAW arranges songs. See [LINNDRUM_RESEARCH.md](LINNDRUM_RESEARCH.md).
- The hardware could not sound every voice at once (shared voice cores, one hat VCA). The plugin does not copy that limit, except hat choke, which is musical.
- 8-bit / 28 kHz grit is not simulated. When the real Linn is sampled, that grit is in the takes.

## Roadmap

1. Multi-sample the orange/black Linn. About six variants per hit so round-robin kills the machine gun. Snare (and later toms / congas) at several pitch settings, named `snare_p0_v1.wav`, `snare_pm3_v1.wav`, and so on. Keep the folder names already in `VoiceBank.h`.
2. Drop mythical / era takes into `Resources/Samples/MythTomA/` (or `Resources/Samples/Mythical/<Voice>/`) and hit Reload samples.
3. Fill the insert picker with the final Fashion Victim Single menu, then replace the insert stubs and the aux Mover / Spatializer stubs with those modules. Two Spatializer reverb engines stay on Aux 2. Topology and parameter ids stay.
4. Follow MIDI clock (and later the host playhead) when `midiClockSync` is on. The hook is `MidiClockSync`.
5. Orange / black retro-wave LookAndFeel. Still no copied Linn panel art unless it is original.
6. Optional individual outputs, matching the hardware jacks, as extra buses. Not required for the stereo mixer.

## Presets

- **Stock Linn** — dry inserts, zero sends, a straight pattern: kick on the beat, snare on the backbeat, closed hat on the 8ths.
- **Processed** — snare amplifier insert, a little Mover on the kick, Spatializer on snare / hat / crash, a master widener insert, light swing, a short tom fill.

User presets are written as XML under the Arcades / Danger Zone / Presets folder in the OS app-data directory. Factory names cannot be overwritten.
