# Danger Zone mixer graph

Francis's mixer, as built in this scaffold. Every control below is a real APVTS parameter and a real point in the audio graph. Fashion Victim module bodies are stubs, so the strip can host FVS modules later without a new topology. This instrument does not link `FVS_Host`.

## Product decision

Every LinnDrum pad has its own channel strip:

- Fader
- Pan
- **2 insert slots.** Each slot chooses a Fashion Victim Single effect. The exact menu is still TBD, so the picker is a stub (None plus a provisional list of Single names).
- **2 aux sends**, pre-filled:
  - Aux 1 → **Mover** (delay)
  - Aux 2 → **Spatializer** (reverb). Spatializer has **two reverb engines**, both on this aux return.

The **master bus** has FVS insert access, starting with **2 insert slots** and the same stub picker.

Mythical / era voices use this same strip, so they do not grow a second mixer later. Send levels start at zero. What is pre-filled is the destination, not the send amount: raising Aux 1 always feeds Mover, raising Aux 2 always feeds both Spatializer engines.

## Flow

```
Voice engine (one-shot multi-sample)
        |
        v
 Insert slot 1 ---- stub FVS picker (exact menu TBD) + bypass + amount
        |
        v
 Insert slot 2 ---- same picker
        |
        v
 Fader (level) ---- post-fader, constant-power pan
        |
        +---------------------------+---------------------------+
        |                           |                           |
        v                           v                           v
   Master dry sum            Aux 1 send                  Aux 2 send
                                    |                           |
                                    v                           v
                              Mover (delay)              Spatializer
                              time / feedback            engine A (reverb)
                                                         engine B (reverb)
                                    |                           |
                                    v                           v
                              Mover return               Spatializer return
        |                           |                           |
        +---------------------------+---------------------------+
        |
        v
 Master insert 1 ---- FVS picker + bypass + amount
        |
        v
 Master insert 2 ---- same picker
        |
        v
 Master level ---- hard limit
        |
        v
 Stereo output
```

Mute and solo sit on the strip, before the dry sum and before both sends. Solo is exclusive: if any solo is on, only soloed strips pass. Sends are post-fader and post-pan, so a fader at zero does not feed the auxes.

## What is fixed vs chosen

| Point | Choice | Default |
| --- | --- | --- |
| Voice insert 1 and 2 | Stub picker. Provisional names: None, Equalizer, Amplifier, Shifter, Widener, Pusher, Toughener, Mover, Spatializer. Exact FVS menu TBD | None |
| Master insert 1 and 2 | Same stub picker | None |
| Aux 1 | Pre-filled destination: Mover delay. Send is a level, not a second picker | send 0, return 0.55, 180 ms |
| Aux 2 | Pre-filled destination: Spatializer. Two reverb engines on the return | send 0; engine A and engine B each have size and level; return 0.55 |

The provisional insert names exist so the choice parameter and the hosting switch (`FvsModule`) are real. Replacing that list later is a menu change, not a new strip. Aux 1 and Aux 2 stay wired to Mover and Spatializer in this scaffold.

## Two different "Mover" / "Spatializer" paths

The picker can also select Mover or Spatializer *inside an insert slot*. That is a short stub on that channel (a slap, or a crossfeed). It is not the aux engine.

- Aux 1 Mover: stereo delay in `FxChain`, fed only by aux 1 sends, returned by `moverReturn`.
- Aux 2 Spatializer: two `juce::dsp::Reverb` engines in `FxChain`. Engine A is the smaller room (`spatSizeA` / `spatLevelA`). Engine B is the larger room (`spatSizeB` / `spatLevelB`). Their sum is returned by `spatReturn`.

## APVTS ids

Per voice, `id` is the sample-folder name in lower case (`kick`, `hatclosed`, `mythtoma`, …):

- `id_level`, `id_pan`, `id_mute`, `id_solo`, `id_tune`
- `id_aux1`, `id_aux2`
- `id_ins1`, `id_ins1Bypass`, `id_ins1Amount`
- `id_ins2`, `id_ins2Bypass`, `id_ins2Amount`

Master and aux engines:

- `masterLevel`
- `masterIns1`, `masterIns1Bypass`, `masterIns1Amount`
- `masterIns2`, `masterIns2Bypass`, `masterIns2Amount`
- `moverTimeMs`, `moverFeedback`, `moverReturn`
- `spatSizeA`, `spatSizeB`, `spatLevelA`, `spatLevelB`, `spatReturn`

Insert amount `0` is a bypass even if a module is selected. The stub DSP starts to colour the signal as amount rises. Bypass forces a passthrough.

## Where it lives

- `Source/Mixer.cpp` — fader, pan, mute, solo, send gains
- `Source/FvsInsert.cpp` — insert-slot stub DSP and the hosting boundary
- `Source/FxChain.cpp` — aux Mover and the two-engine Spatializer
- `Source/PluginProcessor.cpp` — the graph order above
- Mixer tab — strips, insert pickers, master inserts
- FX tab — Mover, both Spatializer engines, insert amount, tune, jitter

## TODO

- Lock the insert menu to the real Fashion Victim Single list when that list is chosen. Keep two slots per Linn voice and two slots on the master.
- Replace `processSlot` with the real FVS module for the picked id. Keep the parameter ids.
- Replace `FxChain::processMover` and `processSpatializer` with real FVS Mover and FVS Spatializer. Keep two reverb engines on the Aux 2 return.
- Host that DSP behind `FvsModule` / `FxChain` (or a library load). Leave the Single-host shell out of this instrument.
