# LinnDrum research notes

Danger Zone is modelled on the **LinnDrum** (Linn Electronics, 1982–1985), the orange/black machine. People often call that unit the LM-2. The LM-2 name is a nickname. The earlier machine is the LM-1 (1980). Sample the LinnDrum, not an LM-1, unless a take is intentionally an era-page sound.

Sources used for this note: LinnDrum owner's manual (behaviour, not quoted), Electronics & Music Maker, February 1983, the Wikipedia LinnDrum summary, the Kenton MIDI retrofit sheet (historical note map only), and the MAME `linndrum` driver comments for how the voice cores were wired. Nothing here is a ROM dump or a copy of the manual.

## Voices to keep

Program these as separate plugin voices. MIDI notes are a modern GM-friendly map so a DAW drum editor works. They are **not** the Kenton retrofit map (that retrofit used 36 = bass, 37 = snare, and so on, which fights General MIDI).

| Plugin voice | On the LinnDrum | MIDI note | Placeholders now |
| --- | --- | --- | --- |
| Kick | Bass | 36 | 6 variants |
| Snare | Snare. Tuning knob on the panel | 38 | 6 variants, pitch layers later |
| Sidestick | Sidestick (cross-stick) | 37 | 2 |
| Rim | Rimshot. Often grouped with the snare/sidestick core on the hardware; kept as its own slot so it can be sampled on its own | 34 | 2 |
| Hat Closed | Hi-hat, fast VCA decay | 42 | 2 |
| Hat Open | Same hat sample on the hardware, slow VCA decay. Separate slot here, and it chokes with the closed hat | 46 | 2 |
| Cabasa | Cabasa | 69 | 2 |
| Tambourine | Tambourine | 54 | 2 |
| Conga Lo | Low conga. Tuning shared with the tom/conga clock | 64 | 2 |
| Conga Hi | High conga | 63 | 2 |
| Cowbell | Cowbell. Also a trigger-out source on the hardware | 56 | 2 |
| Claps | Handclaps | 39 | 2 |
| Tom Lo / Mid / Hi | Three toms, tunable | 45 / 47 / 50 | 2 each |
| Ride | Ride | 51 | 2 |
| Crash | Crash | 49 | 2 |

The panel is often counted as 15 mix channels because open and closed hat are one voice core (one VCA, two decays) and rim/sidestick sit on the snare core. The plugin exposes 17 triggers so each articulation can have its own multi-samples. That matches how the pattern was actually programmed: closed hat and open hat are different buttons.

Target later, once the orange machine is sampled: about **six variants per hit**, and snare pitch layers at several knob settings. The LM-1 clave is not a LinnDrum voice. If it is wanted, it belongs on the mythical / era page, not in the Linn bank.

Hardware dynamics (not separate samples on the original for every step): snare had three loudness levels; bass, hat, cabasa, tambourine, and ride had two; clap and cowbell were fixed. The plugin maps MIDI velocity to gain for now. TODO: real velocity layers, separate from the anti-machine-gun round robin.

## Sequencer ideas worth keeping

- 16th-note grid. Error-correct on the hardware quantized a performance onto 1/8, 1/8 triplet, 1/16, 1/16 triplet, 1/32, or 1/32 triplet. The plugin grid is straight 16ths with a pattern length of 1–16. Triplet grids are a later TODO.
- Shuffle. The panel had straight timing plus six feels, A–F. A is straight 16ths. B and C are the feels people actually used. The plugin uses one swing percent: 0% is straight, and 100% pushes the odd 16th toward a triplet. TODO: measure A–F on the orange machine and offer those six steps if the percent is not enough.
- The manual's Before / After controls let each overdub keep its own feel, or rewrite the feel of what was already recorded. Not in this scaffold. One swing for the whole pattern is enough until the real feels are measured.
- Tempo lived on the panel (the display also shared the trigger-out setting). The plugin tempo is an APVTS parameter, 40–240 BPM, used only by the internal clock until MIDI clock follow exists.

## What the DAW replaces — skip it

- Cassette storage of patterns and songs.
- Song mode (a list of patterns). The DAW arrangement is the song. A pattern chain inside the plugin can wait.
- DIN sync, tape sync (48 pulses per quarter on the sync jack), and the trigger-out jack (1/8 through 1/64-triplet, or "every cowbell"). MIDI clock *in* is the replacement, and it is only a counted stub today (`midiClockSync` is stored, the grid does not follow it).
- The click / metronome output and the programming beep.
- Battery-backed memory limits (56 user patterns, 42 presets, 49 songs on the hardware). Presets are files. The host chunk stores the current kit and pattern.
- Rear-panel individual jacks (one per drum, plus the click) and the mix faders that did not affect those jacks. The plugin mixer is the stereo mix. Extra output buses are a TODO, not a requirement for the first instrument.
- The control-voltage jacks that retuned snare/sidestick and the tom/conga group. Per-voice tune (and later snare pitch layers) covers that.

## Hardware limits we deliberately drop

MAME's notes on the voice cores, shortened:

- Several sounds shared a multiplexed DAC. Bass, cabasa, tambourine, clap, cowbell, hat, ride, and crash could not each play every loudness at once, and some could not overlap a second copy of themselves the way a sampler can.
- Snare and sidestick shared one core. You did not get a full overlap of both, and the end user did not get every loudness the DAC could do.
- Toms and congas shared clocks. Tuning one moved the group. They could not all speak with fully independent pitch at the same instant.
- Open and closed hat are one sample and one VCA. Closing the hat shortens the decay; it also stops the open hat.

The plugin keeps **hat choke** only. Kick, snare, toms, and congas overlap, each with its own tune. Round-robin variants are allowed to overlap so a fast roll is not one sample restarting (the machine gun). A new hat hit fades the previous hat in that choke group.

Samples on the machine were 8-bit, roughly 28–35 kHz, with a CEM3320 lowpass on the bass and simple RC filters elsewhere. Do not bitcrush the plugin by default. Sample the real outputs, several takes, and the clock drift shows up as small differences between takes. That is why one sample retriggered sounds like a gun and six variants do not. Jitter in the engine is only a stand-in until those takes exist.

## Mythical / era page

Not part of the LinnDrum ROM. Slots reserved so later sessions can hold other era machines, or "mythical" toms that never shipped in 1982, without crowding the Linn bank:

Myth Tom A, Myth Tom B, Era Kick, Era Snare, Era Hat, Era Perc.

MIDI notes 72–77. No placeholder audio yet. The engine already looks in `Resources/Samples/<Voice>/` and `Resources/Samples/Mythical/<Voice>/`.
