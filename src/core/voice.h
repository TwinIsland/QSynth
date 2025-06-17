#pragma once

#include <stdbool.h>

#include "tone.h"
#include "../envelope/adsr.h"

typedef struct
{
    const Tone *tone; // What to play
    int duration_ms;  // Note length
    double velocity;  // Playing strength (0.0-1.0)
    double frequency;
    double amplitude;
    double pan;

    // state
    bool active;
    double phases[MAX_TONE_LAYERS]; // phase for each toneblock
    double _sample_rate;

    // lefted filter/envelope
    BiquadFilter lfilter;
    ADSREnvelope lenvelope;
} Voice;

// void voice_init(Voice *voice, double sample_rate);
void voice_reset(Voice *voice);

void voice_start(Voice *voice, double sample_rate);
double voice_step(Voice *voice, double delta_time);
