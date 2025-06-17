#include "instruments_core.h"
#include <stdio.h>


static const InstrumentSignature instrument_signatures[INST_COUNT] = {
    // ===== LEAD INSTRUMENTS =====
    [INST_LEAD_SQUARE] = {
        .tone = {
            .layers = {
                // Main square wave layer
                {
                    .base_wave = WAVE_SQUARE,
                },
                // Sub-octave layer for fatness
                {
                    .base_wave = WAVE_SQUARE,
                },
                // Slight detune layer for width
                {
                    .base_wave = WAVE_SQUARE,
                },
                // High harmonic layer for brightness
                {
                    .base_wave = WAVE_TRIANGLE, // Softer wave for harmonics
                },
            },
            .detune = {0.0, -12.0, +0.05, +12.0},
            .mix_levels = {1.0, 0.4, 0.3, 0.2},
            .phase_diff = {0, 0, 0, 0},
            .filter_opt = {
                .cutoff = 1000, 
                .filter_type = FILTER_HIGHPASS, 
                .resonance = 0.77
            },
            .envelope_opt = {
                .attack_ratio = 0.01, 
                .decay_ratio = 0.29, 
                .release_ratio = 0.7, 
                .sustain_level = 0.5
            },
        },
        .name = "Lead Square",
        .category = "Lead",
        .description = "Rich square wave lead with sub-bass, detuning, and harmonic layers"
    },
};

const InstrumentSignature *instrument_get_signature(InstrumentType type)
{
    if (type < 0 || type >= INST_COUNT)
        return NULL;
    
    return &instrument_signatures[type];
}