#pragma once

#include <stdbool.h>

#include "instruments.h"

typedef struct Synthesizer Synthesizer;

#define MAX_TONE_LAYERS 4
#define AUDIO_BUFFER_SIZE 4410
#define MAX_VOICE_ACTIVE 5


typedef struct {
    int midi_note;
    double duration;
    double amplitude;
    double velocity;
    double pan;
} NoteCfg;

typedef enum {
    QSYNTH_ERROR_NONE = 0,
    QSYNTH_ERROR_MEMALLOC,
    QSYNTH_ERROR_DEVICE,
    QSYNTH_ERROR_NOTECFG,
    QSYNTH_ERROR_UNINIT,
    QSYNTH_ERROR_VOICE_UNAVAILABLE,
    QSYNTH_ERROR_UNSUPPORT,
} QSynthError;

// qsynth init/deinit
bool synth_init(Synthesizer** synth_ptr, double sample_rate, int channels);
void synth_cleanup(Synthesizer* synth);

// qsynth streaming
bool synth_start(Synthesizer* synth);
void synth_stop(Synthesizer* synth);

// qsynth basic sound interface
int synth_play_note(Synthesizer* synth, InstrumentType instrument, NoteCfg *cfg);

// qsynth global setting
int synth_set_master_volume(Synthesizer *synth, double volume);

// qsynth error handling
QSynthError synth_get_last_error();
const char* synth_get_error_string(QSynthError error);
