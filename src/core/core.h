#pragma once

#include <stdbool.h>

#include "audio_device.h"
#include "qsynth.h"

#include "voice.h"

#include "../filters/biquad.h"

#define BUF_AVAILABLE_IDX_MASK 0b1

struct Synthesizer {
    // Audio system
    short* audioBuffers[2];
    unsigned char buffer_state;  // [next_buffer_idx]
    AudioDevice device;
    
    // Voice management
    Voice voices[MAX_VOICE_ACTIVE];
    
    // Global settings
    double master_volume;
    
    // State
    int samples_played;
};

// internal functions
void _audio_callback(void *_, int16_t *output_buffer, int frame_count);
void _process_voice_buffer(Synthesizer* synth);
