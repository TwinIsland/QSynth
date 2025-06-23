#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "instruments.h"

typedef struct Synthesizer Synthesizer;

#define MAX_TONE_LAYERS 4

#define AUDIO_FRAME_PER_READ 4410 // 100ms buffer size for audio device

#define VOICE_BUFFER_SIZE 8192
#define VOICE_BUFFER_REFILL_THRESHOLD 0.5
#define REFILL_CHUNK_SIZE 8192
#define MAX_VOICE_ACTIVE 16

typedef enum
{
    NOTE_CONTROL_DURATION, // Use duration_ms
    NOTE_CONTROL_MANUAL    // Use start_note/end_note API
} NoteControlMode;

typedef struct
{
    int midi_note;
    int duration_ms;
    double amplitude;
    double pan;
} NoteCfg;

typedef enum
{
    QSYNTH_ERROR_NONE = 0,
    QSYNTH_ERROR_MEMALLOC,
    QSYNTH_ERROR_DEVICE,
    QSYNTH_ERROR_NOTECFG,
    QSYNTH_ERROR_UNINIT,
    QSYNTH_ERROR_VOICE_UNAVAILABLE,
    QSYNTH_ERROR_CONFIG,
    QSYNTH_ERROR_WORKER,
    QSYNTH_ERROR_UNSUPPORT,
} QSynthError;

typedef struct {
    double frame_per_read;
    int voice_buffer;
    int max_voice;
    int voice_active;
    int latency_ms;
    int sample_processed;
    const int16_t *recent_samples;
    int recent_sample_size;
} QSynthStat;

// qsynth init/deinit
bool synth_init(Synthesizer **synth_ptr, double sample_rate, int channels);
void synth_cleanup(Synthesizer *synth);

// qsynth streaming
bool synth_start(Synthesizer *synth);
void synth_stop(Synthesizer *synth);

// qsynth basic sound interface
int synth_play_note(Synthesizer *synth, InstrumentType instrument, NoteControlMode control_mode, NoteCfg *cfg);
void synth_end_note(Synthesizer *synth, int voice_id);

// qsynth global setting
int synth_set_master_volume(Synthesizer *synth, double volume);

// qsynth error handling
QSynthError synth_get_last_error();
const char *synth_get_error_string(QSynthError error);

// qsynth utils
QSynthStat synth_get_stat(Synthesizer *synth);

// qsynth static data
void synth_print_stat(Synthesizer *synth);