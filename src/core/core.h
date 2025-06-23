#pragma once

#include <stdbool.h>

#include "audio_device.h"
#include "qsynth.h"

#include "stream.h"

#include "voice.h"

#include "../filters/biquad.h"

#define RECENT_SAMPLE_SIZE 1024 // have to be power of 2
#define RECENT_SAMPLE_MASK 1023 // always equal to RECENT_SAMPLE_SIZE-1

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
    // short *audioBuffers[2];
    // unsigned char buffer_state; // [next_buffer_idx]
    AudioDevice device;

    // Voice management
    Voice voices[MAX_VOICE_ACTIVE];

    // Global settings
    double master_volume;

    // State
    int samples_played;
    bool voice_dp_generator_running;
    uint64_t latency_ms; 
    int voice_active;

    int16_t recent_samples[RECENT_SAMPLE_SIZE];
    uint32_t recent_samples_writeptr;

    // static pre-computed data
    double delta_time;
};
