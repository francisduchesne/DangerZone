# Danger Zone mixer graph

The mixer is a real signal path with real APVTS parameters. Fashion Victim module *bodies* are stubs. The instrument does not link `FVS_Host`.

## Flow

```
Voice engine (one-shot multi-sample)
        |
        v
 Insert slot 1 ---- FVS picker (None or a named module) + bypass + amount
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
| Voice insert 1 and 2 | Picker: None, Equalizer, Amplifier, Shifter, Widener, Pusher, Toughener, Mover, Spatializer | None |
| Master insert 1 and 2 | Same picker | None |
| Aux 1 destination | Fixed. Always the Mover delay | send 0, return 0.55, 180 ms |
| Aux 2 destination | Fixed. Always the Spatializer | send 0, both engines mixed by their levels |

Aux destinations are not pickers. Turning up `aux1` always hits Mover. Turning up `aux2` always hits Spatializer.

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

- Replace `processSlot` with the real Fashion Victim module for the picked id. Keep the parameter ids.
- Replace `FxChain::processMover` and `processSpatializer` with real FVS Mover and FVS Spatializer. Keep two reverb engines on the Spatializer return.
- Do not turn Danger Zone into an `FVS_Host` module to do that. Host the DSP behind `FvsModule` / `FxChain`, or load the module as a library, without pulling the Single-host shell into this instrument.
