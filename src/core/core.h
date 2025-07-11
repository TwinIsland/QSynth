#pragma once

#include <stdbool.h>

#include "qsynth.h"
#include "stream.h"
#include "voice.h"

#include "../audio/miniaudio.h"
#include "../assets/pedal_core.h"
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
    ma_device device;

    // Voice management
    Voice voices[MAX_VOICE_ACTIVE];

    // Pedal management
    PedalChain *pedalchain;

    // Global settings
    double master_volume;

    // State
    int samples_played;
    bool voice_dp_generator_running;
    bool pedal_dp_generator_running;
    bool voice_mix_generator_running;
    uint64_t latency_ms;
    int voice_active;

    int16_t recent_samples[RECENT_SAMPLE_SIZE];
    uint32_t recent_samples_writeptr;

    // intermidiate streamers
    double voice_mix_buf[VOICE_MIX_BUFFER_SIZE];
    AudioStreamBuffer voice_mix_streamer;

    // static pre-computed data
    double delta_time;
};
