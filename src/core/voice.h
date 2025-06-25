#pragma once

#include <stdbool.h>

#include "pthread.h"

#include "tone.h"
#include "stream.h"
#include "../envelope/adsr.h"

typedef struct
{
    const Tone *tone; // What to play
    double velocity;  // Playing strength (0.0-1.0)
    double frequency;
    double amplitude;
    double pan;
    int duration_ms;
    NoteControlMode control_mode;

    // state
    bool active;
    double phases[MAX_TONE_LAYERS]; // phase for each toneblock
    double cur_duration;
    bool voice_is_end;
    double _sample_rate;

    // lefted filter/envelope
    BiquadFilter filter;
    ADSREnvelope envelope;

    // streaming state
    double stream_buf[VOICE_BUFFER_SIZE];
    AudioStreamBuffer streamer; // buffer for streaming audio
} Voice;

// void voice_init(Voice *voice, double sample_rate);
void voice_init(Voice *voice);
void voice_start(Voice *voice, double sample_rate);
void voice_end(Voice *voice);
double voice_step(Voice *voice, double delta_time);