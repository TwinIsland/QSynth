#pragma once

#include <stdbool.h>

#include "audio_device.h"
#include "qsynth.h"

#include "voice.h"

#include "../filters/biquad.h"

#define BUF_AVAILABLE_IDX_MASK 0b1

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

struct Synthesizer
{
    // Audio system
    short *audioBuffers[2];
    unsigned char buffer_state; // [next_buffer_idx]
    AudioDevice device;

    // Voice management
    Voice voices[MAX_VOICE_ACTIVE];

    // Global settings
    double master_volume;

    // State
    int samples_played;
    bool voice_dp_generator_running;

    // static pre-computed data
    double delta_time;
};

void _process_voice_buffer(Synthesizer *synth);