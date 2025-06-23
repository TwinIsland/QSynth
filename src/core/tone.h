#pragma once

#include "wave.h"
#include "qsynth.h"
#include "../envelope/adsr.h"
#include "../filters/biquad.h"

#define TONE_MIX_LEVEL_DEFAULT (1 / MAX_TONE_LAYERS)

typedef struct
{
    Wave layers[MAX_TONE_LAYERS];
    double detune[MAX_TONE_LAYERS];     // frequency offsets for layers
    double mix_levels[MAX_TONE_LAYERS];
    double phase_diff[MAX_TONE_LAYERS]; // phase difference for each layers

    FilterCfg filter_opt;           // filter options
    EnvelopeCfg envelope_opt;       // envelop duration control options
} Tone;
