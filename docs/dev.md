# Synthesizer Architecture Design Document

## Overview

This document describes a hierarchical synthesizer architecture built from simple, composable building blocks. Each layer adds musical meaning and functionality to create a complete music synthesis system.

## Architecture Hierarchy

```
Wave (Basic Waveform)
    ↓
ToneBlock (Sound Characteristics)
    ↓
Tone (Musical Notes)
    ↓
Voice (Musical Performance)
    ↓
Song (Complete Composition)
```

---

## 1. Wave - The Foundation

**Purpose**: The most basic form of audio signal - pure mathematical waveforms.

**Definition**: A repeating pattern that creates pitch through frequency and timbre through shape.

### Properties
- **Shape**: Mathematical function (sine, square, sawtooth, triangle, noise)
- **Pure Mathematics**: No musical meaning yet

### Examples
```
Sine Wave:     ∿∿∿∿∿∿∿∿     (Pure, smooth)
Square Wave:   ▄▀▄▀▄▀▄▀     (Harsh, digital)
Sawtooth:      /|/|/|/|     (Bright, rich harmonics)
Triangle:      /\/\/\/\     (Mellow, hollow)
Noise:         ░▓░▓░▓░▓     (Random, percussive)
```

### Use Cases
- **Sine**: Pure tones, sub-bass, bells
- **Square**: Retro games, electronic leads
- **Sawtooth**: Analog synths, bass lines
- **Triangle**: Soft pads, flutes
- **Noise**: Drums, percussion, effects

---

## 2. ToneBlock - Sound Characteristics

**Purpose**: Adds musical and timbral characteristics to a raw wave.

**Definition**: A wave enhanced with all the properties that define how it sounds.

### Properties
- **Wave**: Base waveform (from Wave layer)
- **Pitch**: Musical frequency (MIDI note or Hz)
- **Filter**: Frequency shaping (low-pass, high-pass, etc.)
- **Phase**: Position in wave cycle
- **Envelope**: Volume changes over time (ADSR)
- **Amplitude**: Overall volume level

### Structure
```
ToneBlock {
    Wave base_wave;
    double pitch;           // MIDI note or frequency
    Filter filter;          // Frequency modification
    double phase;           // Wave position
    Envelope envelope;      // ADSR parameters
    double amplitude;       // Volume level
}
```

### Examples
```
Bass ToneBlock:
- Wave: Sawtooth
- Pitch: C2 (65.4 Hz)
- Filter: Low-pass at 200Hz
- Envelope: Fast attack, medium decay
- Amplitude: 0.8

Lead ToneBlock:
- Wave: Square
- Pitch: C5 (523.3 Hz)
- Filter: High-pass at 300Hz
- Envelope: Medium attack, long sustain
- Amplitude: 0.6

Pad ToneBlock:
- Wave: Triangle
- Pitch: C4 (261.6 Hz)
- Filter: Band-pass at 1000Hz
- Envelope: Slow attack, long release
- Amplitude: 0.4
```

### Use Cases
- **Instrument definition**: Define how an instrument sounds
- **Sound design**: Create specific timbres
- **Preset system**: Reusable sound configurations

---

## 3. Tone - Musical Notes

**Purpose**: Groups multiple ToneBlocks to create rich, layered sounds.

**Definition**: A collection of ToneBlocks played together to form a single musical note.

### Properties
- **ToneBlocks**: Array of sound layers
- **Root Pitch**: Base musical note
- **Harmonics**: Additional frequencies for richness
- **Layering**: Multiple timbres combined

### Structure
```
Tone {
    ToneBlock[] layers;     // Multiple sound layers
    int root_note;          // Base MIDI note
    double detune[];        // Pitch offsets for layers
    double mix_levels[];    // Volume balance between layers
}
```

### Examples
```
Rich Piano Tone:
- Layer 1: Sine wave (fundamental)
- Layer 2: Triangle wave (+12 semitones, quieter)
- Layer 3: Noise burst (attack transient)

Fat Synth Bass:
- Layer 1: Sawtooth (fundamental)
- Layer 2: Sawtooth (-12 semitones, sub-octave)
- Layer 3: Square wave (slight detune, +0.05 semitones)

Atmospheric Pad:
- Layer 1: Triangle (fundamental)
- Layer 2: Sine (+7 semitones, fifth)
- Layer 3: Triangle (+12 semitones, octave)
- Layer 4: Sine (+19 semitones, major third)
```

### Use Cases
- **Rich instruments**: Multi-layered sounds
- **Harmonic content**: Add overtones and undertones
- **Detuning effects**: Slight pitch variations for width
- **Attack layers**: Different sounds for note start vs. sustain

---

## 4. Voice - Musical Performance

**Purpose**: Adds temporal and performance aspects to a Tone.

**Definition**: A Tone with timing, dynamics, and performance characteristics.

### Properties
- **Tone**: The sound definition
- **Envelope**: Performance-level volume shaping
- **Duration**: How long the note plays
- **Velocity**: Performance dynamics (how hard it's played)
- **Articulation**: Performance style (legato, staccato, etc.)
- **Expression**: Real-time modulation

### Structure
```
Voice {
    Tone sound;             // What to play
    Envelope performance;   // Performance-level envelope
    double duration;        // Note length
    double velocity;        // Playing strength (0.0-1.0)
    double start_time;      // When to start
    double pan;             // Stereo position
    Expression modulation;  // Real-time changes
}
```

### Examples
```
Gentle Piano Voice:
- Tone: Rich Piano (3 layers)
- Duration: 2.0 seconds
- Velocity: 0.4 (soft touch)
- Envelope: Medium attack, natural decay
- Pan: Center

Aggressive Lead Voice:
- Tone: Distorted Sawtooth
- Duration: 0.5 seconds
- Velocity: 0.9 (hard playing)
- Envelope: Instant attack, quick release
- Pan: Slightly left

Sustained Pad Voice:
- Tone: Atmospheric Pad (4 layers)
- Duration: 8.0 seconds
- Velocity: 0.6 (medium)
- Envelope: Very slow attack, long release
- Pan: Wide stereo
```

### Use Cases
- **Musical expression**: Dynamics and articulation
- **Timing control**: Note duration and placement
- **Performance realism**: Velocity sensitivity
- **Spatial placement**: Stereo positioning

---

## 5. Song - Complete Composition

**Purpose**: Organizes Voices into a complete musical piece.

**Definition**: A collection of Voices arranged in time to create music.

### Properties
- **Voices**: Array of musical performances
- **Timeline**: When each voice plays
- **Tempo**: Speed of the music
- **Structure**: Verse, chorus, bridge, etc.
- **Global effects**: Reverb, compression, etc.

### Structure
```
Song {
    Voice[] voices;         // All musical parts
    double[] start_times;   // When each voice begins
    double tempo;           // Beats per minute
    TimeSignature meter;    // 4/4, 3/4, etc.
    GlobalEffects fx;       // Song-wide processing
    string title;           // Song name
}
```

### Organization Types

#### Sequential (Melody)
```
Time:  0s    1s    2s    3s    4s
Voice: [C4]  [D4]  [E4]  [F4]  [G4]
```

#### Simultaneous (Chord)
```
Time:  0s-2s
Voice: [C4] Bass
       [E4] Harmony
       [G4] Melody
```

#### Complex Arrangement
```
Time:     0s    4s    8s    12s   16s
Bass:     [──────────────────────────]
Drums:          [──────────────────]
Lead:                 [────────]
Pad:      [──────────]       [────]
```

### Examples
```
Simple Song Structure:
- Intro: Atmospheric pad (4 bars)
- Verse: Bass + drums + melody (16 bars)
- Chorus: Full arrangement (8 bars)
- Verse: Repeat with variation (16 bars)
- Chorus: Repeat (8 bars)
- Outro: Fade with pad (4 bars)

Electronic Track:
- Build-up: Gradual addition of layers
- Drop: All elements together
- Break: Minimal elements
- Build-up: Second rise
- Drop: Full energy again
- Outro: Gradual removal of layers
```

### Use Cases
- **Complete compositions**: Full songs and tracks
- **Arrangements**: Multiple instrument parts
- **Song structure**: Verse/chorus organization
- **Timing coordination**: Synchronized playback

