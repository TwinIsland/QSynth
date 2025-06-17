# QSynth

A high-performance multi-layered audio synthesizer written in C, featuring real-time audio processing with advanced instrument design capabilities.

## ✨ Features

- **Multi-layer Synthesis**: Combine multiple waveforms with independent detuning and mixing
- **Real-time Audio Processing**: Low-latency audio output with double-buffering
- **Advanced Filtering**: Biquad filters (Low-pass, High-pass, Band-pass) with adjustable resonance
- **ADSR Envelope**: Attack, Decay, Sustain, Release envelope shaping
- **Flexible Instrument Design**: Define custom instruments with multiple oscillator layers
- **Voice Management**: Polyphonic playback with configurable voice limits
- **Cross-platform Ready**: Modular audio backend (Windows implemented, Linux planned)

## 🎵 Instrument Definition

Create rich, layered instruments with ease:

```c
static const InstrumentSignature instrument_signatures[INST_COUNT] = {
    [INST_LEAD_SQUARE] = {
        .tone = {
            .layers = {
                // Main square wave layer
                { .base_wave = WAVE_SQUARE },
                // Sub-octave layer for fatness  
                { .base_wave = WAVE_SQUARE },
                // Slight detune layer for width
                { .base_wave = WAVE_SQUARE },
                // High harmonic layer for brightness
                { .base_wave = WAVE_TRIANGLE },
            },
            .detune = {0.0, -12.0, +0.05, +12.0},     // Semitones & cents
            .mix_levels = {1.0, 0.4, 0.3, 0.2},       // Layer volumes
            .phase_diff = {0, 0, 0, 0},               // Phase offsets
            .filter_opt = {
                .cutoff = 1000, 
                .filter_type = FILTER_HIGHPASS, 
                .resonance = 0.77
            },
            .envelope_opt = {
                .attack_ratio = 0.01,    // 1% attack
                .decay_ratio = 0.29,     // 29% decay  
                .release_ratio = 0.7,    // 70% release
                .sustain_level = 0.5     // 50% sustain
            },
        },
        .name = "Lead Square",
        .category = "Lead",
        .description = "Rich square wave lead with sub-bass, detuning, and harmonic layers"
    },
    // Add more instruments here...
};
```

## 🚀 Quick Start

### Prerequisites
- **Windows**: MinGW-w64 or Visual Studio
- **Linux**: GCC with ALSA dev libraries *(coming soon)*

### Building

```bash
# Windows (MinGW)
gcc -Wall -Wextra -std=c99 -O2 \
    -Iinclude -Idevice -Iassets \
    ./device/winaudio.c -lwinmm \
    ./src/**/*.c \
    ./examples/basic_synth.c \
    -o ./build/basic_synth.exe

# Run the example
./build/basic_synth.exe
```

### Basic Usage

```c
#include <stdio.h>
#include <windows.h>
#include "qsynth.h"

int main() {
    // Initialize synthesizer
    Synthesizer *synth;
    if (!synth_init(&synth, 44100.0, 2)) {
        printf("Failed to initialize synthesizer\n");
        return 1;
    }

    // Start audio playback
    if (!synth_start(synth)) {
        printf("Error: %s\n", synth_get_error_string(synth_get_last_error()));
        return 1;
    }

    // Play a simple melody (C major scale)
    int melody[] = {60, 64, 67, 72, 67, 64, 60}; // MIDI note numbers
    
    for (int i = 0; i < 7; i++) {
        NoteCfg cfg = {
            .midi_note = melody[i],
            .amplitude = 0.8,
            .duration = 2.0,        // seconds
            .velocity = 1.0,
            .pan = 0.5              // center pan
        };

        int voice_id = synth_play_note(synth, INST_LEAD_SQUARE, &cfg);
        if (voice_id == -1) {
            printf("Error playing note: %s\n", 
                   synth_get_error_string(synth_get_last_error()));
        }

        Sleep(1000); // Wait 1 second between notes
    }

    // Cleanup
    synth_stop(synth);
    synth_cleanup(synth);
    return 0;
}
```

## 🎛️ API Reference

### Core Functions

| Function | Description |
|----------|-------------|
| `synth_init(synth, sample_rate, channels)` | Initialize synthesizer |
| `synth_start(synth)` | Start audio playback |
| `synth_stop(synth)` | Stop audio playback |
| `synth_cleanup(synth)` | Free resources |
| `synth_play_note(synth, instrument, config)` | Play a note |
| `synth_set_master_volume(synth, volume)` | Set master volume (0.0-1.0) |

### Note Configuration

```c
typedef struct {
    int midi_note;      // MIDI note number (0-127)
    double amplitude;   // Note amplitude (0.0-1.0)
    double duration;    // Duration in seconds
    double velocity;    // Velocity (0.0-1.0)
    double pan;         // Stereo pan (0.0=left, 0.5=center, 1.0=right)
} NoteCfg;
```

## 🎯 Roadmap

- [ ] **Linux Audio Support** (ALSA/PulseAudio)
- [ ] **macOS Audio Support** (Core Audio)
- [ ] **MIDI Input** support
- [ ] **Effect Chain** (Reverb, Delay, Chorus)
- [ ] **Real-time Parameter Control**
- [ ] **Preset Management System**
- [ ] **Audio File Export** (WAV/FLAC)

## 🏗️ Architecture

```
QSynth/
├── src/
│   ├── core.c              # Main synthesizer logic
│   ├── voice.c             # Voice management
│   ├── instruments.c       # Instrument definitions
│   └── filters/
│       └── biquad.c        # Digital filtering
├── device/
│   ├── winaudio.c          # Windows audio backend
│   └── linuxaudio.c        # Linux audio backend (planned)
├── include/
│   └── qsynth.h            # Public API
└── examples/
    └── basic_synth.c       # Usage examples
```

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

---

*QSynth - Real-time audio synthesis made simple* 🎵