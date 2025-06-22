# QSynth

A high-performance multi-layered audio synthesizer written in C, featuring real-time audio processing with advanced instrument design capabilities.

## Features

- **Multi-layered synthesis** - Up to 4 tone layers per instrument with independent waveforms and detuning
- **Real-time audio processing** - Low-latency audio output with configurable buffer sizes  
- **Custom Instruments** - QSynth allows you to create custom instruments by defining their synthesis parameters
- **Double buffering** - Smooth audio playback without dropouts using ping-pong buffers
- **Per-voice buffering** - Individual buffer streams for each voice to prevent audio stuttering
- **Cross-platform** - Windows and Linux support with unified API
- **Advanced instruments** - Customizable ADSR envelopes, filters, and effects per voice
- **Polyphonic playback** - Up to 7 simultaneous voices with independent panning and velocity

## Quick Start

### Building

1. **Build the build system:**
   ```bash
   gcc build.c -o nob
   ```

2. **Build examples:**
   ```bash
   # Build basic synthesizer example
   ./nob basic_synth
   
   # Build instruments test
   ./nob instruments_test
   
   # Build for 64-bit
   ./nob basic_synth --x64
   
   # Build with debug symbols
   ./nob basic_synth --debug
   ```

3. **Run:**
   ```bash
   cd build
   ./basic_synth.exe
   ```

### Basic Usage

```c
#include "qsynth.h"

int main() {
    Synthesizer *synth;
    
    // Initialize synthesizer
    if (!synth_init(&synth, 44100.0, 2)) {
        printf("Failed to initialize synthesizer\n");
        return 1;
    }
    
    // Start audio playback
    synth_start(synth);
    
    // Play a note
    NoteCfg note = {
        .midi_note = 60,        // Middle C
        .duration_ms = 1000,    // 1 second
        .amplitude = 0.8,       // 80% volume
        .velocity = 1.0,        // Full velocity
        .pan = 0.5              // Center pan
    };
    
    synth_play_note(synth, INSTRUMENT_PIANO, &note);
    
    // Wait for note to finish
    Sleep(1500);
    
    // Cleanup
    synth_stop(synth);
    synth_cleanup(synth);
    
    return 0;
}
```

## API Reference

### Initialization & Cleanup

#### `bool synth_init(Synthesizer** synth_ptr, double sample_rate, int channels)`
Initializes the synthesizer with specified audio parameters.
- **synth_ptr**: Pointer to store the created synthesizer instance
- **sample_rate**: Audio sample rate (e.g., 44100.0 Hz)
- **channels**: Number of audio channels (typically 2 for stereo)
- **Returns**: `true` on success, `false` on failure

#### `void synth_cleanup(Synthesizer* synth)`
Cleans up and frees all synthesizer resources.

### Audio Control

#### `bool synth_start(Synthesizer* synth)`
Starts real-time audio processing and playback.
- **Returns**: `true` on success, `false` on failure

#### `void synth_stop(Synthesizer* synth)`
Stops audio processing and playback.

### Sound Generation

#### `int synth_play_note(Synthesizer* synth, InstrumentType instrument, NoteCfg *cfg)`
Plays a musical note with the specified instrument and configuration.
- **synth**: Synthesizer instance
- **instrument**: Instrument type (e.g., `INSTRUMENT_PIANO`, `INSTRUMENT_ORGAN`)
- **cfg**: Note configuration (pitch, duration, volume, etc.)
- **Returns**: Voice ID on success, negative value on error

### Configuration

#### `int synth_set_master_volume(Synthesizer *synth, double volume)`
Sets the global master volume for all audio output.
- **volume**: Volume level (0.0 to 1.0)
- **Returns**: 0 on success, negative value on error

### Error Handling

#### `QSynthError synth_get_last_error()`
Returns the last error code that occurred.

#### `const char* synth_get_error_string(QSynthError error)`
Converts an error code to a human-readable string.

### Note Configuration

The `NoteCfg` structure defines how a note should be played:

```c
typedef struct {
    int midi_note;      // MIDI note number (0-127, middle C = 60)
    int duration_ms;    // Duration in milliseconds
    double amplitude;   // Note volume (0.0 to 1.0)
    double velocity;    // Attack velocity (0.0 to 1.0)
    double pan;         // Stereo panning (0.0 = left, 1.0 = right, 0.5 = center)
} NoteCfg;
```

## Architecture

### Double Buffering System
QSynth uses a ping-pong buffer system where audio is rendered to one buffer while the audio device plays from another. This ensures smooth, uninterrupted audio playback even during CPU spikes.

### Per-Voice Buffer Streams
Each active voice maintains its own circular buffer that's filled by a background thread. This prevents audio dropouts when complex synthesis algorithms take longer than the audio callback deadline.

### Multi-Threading Design
- **Audio thread**: High-priority real-time thread for audio output
- **Voice generation threads**: Background threads that pre-generate audio samples
- **Main thread**: User interface and note triggering

## Error Codes

- `QSYNTH_ERROR_NONE` - No error
- `QSYNTH_ERROR_MEMALLOC` - Memory allocation failed  
- `QSYNTH_ERROR_DEVICE` - Audio device error
- `QSYNTH_ERROR_NOTECFG` - Invalid note configuration
- `QSYNTH_ERROR_UNINIT` - Synthesizer not initialized
- `QSYNTH_ERROR_VOICE_UNAVAILABLE` - No free voices available
- `QSYNTH_ERROR_UNSUPPORT` - Unsupported operation

## Build Options

- `--debug` - Build with debug symbols and logging
- `--x64` - Build for 64-bit architecture (default: 32-bit)
- `--release` - Build with optimizations (default)

## Creating Custom Instruments

QSynth allows you to create custom instruments by defining their synthesis parameters. This is a two-step process:

### Step 1: Add Instrument Type

Add your new instrument to the enum in `assets/instruments.h`:

```c
typedef enum {
    INST_LEAD_SQUARE = 0,
    INST_WARM_BASS,
    INST_ETHEREAL_PAD,
    INST_METALLIC_PLUCK,
    INST_WOBBLE_BASS,
    INST_BELL_LEAD,
    INST_DEEP_DRONE,
    INST_MY_CUSTOM_SYNTH,    // <- Add your instrument here
    
    INST_COUNT  // total number of instruments
} InstrumentType;
```

### Step 2: Define Instrument Parameters

Add the instrument definition in `src/assets/instruments.c`:

```c
static const InstrumentSignature instrument_signatures[INST_COUNT] = {
    // ... existing instruments ...
    
    // Your custom instrument
    [INST_MY_CUSTOM_SYNTH] = {
        .tone = {
            .layers = {
                {.type = WAVE_SQUARE},      // Layer 1: Square wave
                {.type = WAVE_SAWTOOTH},    // Layer 2: Sawtooth
                {.type = WAVE_SINE},        // Layer 3: Sine wave
                {.type = WAVE_TRIANGLE},    // Layer 4: Triangle
            },
            .detune = {0.0, 0.05, -12.0, 24.0},     // Detune in semitones
            .mix_levels = {0.6, 0.3, 0.2, 0.1},    // Volume mix for each layer
            .phase_diff = {0, 90, 180, 270},        // Phase offset in degrees
            .filter_opt = {
                .cutoff = 1200,                     // Filter cutoff frequency
                .filter_type = FILTER_LOWPASS,      // Filter type
                .resonance = 0.4                    // Filter resonance
            },
            .envelope_opt = ENVELOPE_OPT_PLUCK,     // Envelope preset
        },
        .name = "My Custom Synth",
        .category = "Custom",
        .description = "A custom synthesizer with rich harmonic content"
    },
};
```

### Instrument Parameters Explained

#### **Tone Layers (Up to 4)**
- **WAVE_SINE** - Pure sine wave (smooth, fundamental)
- **WAVE_SQUARE** - Square wave (hollow, reed-like)
- **WAVE_SAWTOOTH** - Sawtooth wave (bright, buzzy)
- **WAVE_TRIANGLE** - Triangle wave (soft, flute-like)

#### **Detune Array**
- Values in **semitones** relative to the base frequency
- `0.0` = no detune, `12.0` = one octave up, `-12.0` = one octave down
- Small values like `0.05` create chorus/beating effects

#### **Mix Levels**
- Volume for each layer (0.0 to 1.0)
- Should generally sum to around 1.0 or less to avoid clipping

#### **Phase Differences**
- Phase offset for each layer in **degrees** (0-360)
- Creates different harmonic relationships between layers

#### **Filter Options**
- **cutoff**: Frequency in Hz where filter takes effect
- **filter_type**: `FILTER_LOWPASS`, `FILTER_HIGHPASS`, `FILTER_BANDPASS`, `FILTER_NONE`
- **resonance**: Filter emphasis (0.0 to 1.0, higher = more pronounced)

#### **Envelope Presets**
- **ENVELOPE_OPT_LEAD** - Quick attack, sustained
- **ENVELOPE_OPT_PLUCK** - Quick attack, quick decay
- **ENVELOPE_OPT_PAD** - Slow attack, long sustain
- **ENVELOPE_OPT_BASS** - Medium attack, punchy

### Usage Example

```c
// Play your custom instrument
NoteCfg note = {
    .midi_note = 60,
    .duration_ms = 2000,
    .amplitude = 0.7,
    .velocity = 1.0,
    .pan = 0.5
};

synth_play_note(synth, INST_MY_CUSTOM_SYNTH, &note);
```

### Tips for Instrument Design

1. **Start Simple** - Begin with one or two layers, then add complexity
2. **Balance Mix Levels** - Ensure total doesn't exceed 1.0 to prevent distortion
3. **Use Detuning Creatively** - Small detunes (0.01-0.1) create thickness, large detunes (-12, +12) add harmonic content
4. **Filter Experimentation** - Try different cutoff frequencies and resonance values
5. **Test Across Octaves** - Make sure your instrument sounds good in different pitch ranges

After making changes, rebuild with `./nob your_example` and test your new instrument!

## Requirements

- **Windows**: MinGW-w64 or Visual Studio
- **Linux**: GCC with pthread support  
- **Dependencies**: pthread library (included in build)

