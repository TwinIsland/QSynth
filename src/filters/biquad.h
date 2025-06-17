#pragma once

#include "qsynth.h"

typedef enum {
    FILTER_LOWPASS = 0,
    FILTER_HIGHPASS = 1,
    FILTER_BANDPASS = 2,
    FILTER_NOTCH = 3,
    FILTER_NONE = 4
} FilterType;


typedef struct {
    FilterType filter_type;
    double cutoff;
    double resonance;
} FilterCfg;

typedef struct
{
    // Coefficients
    double a0, a1, a2; // Feedforward
    double b1, b2;     // Feedback

    double x1, x2; // Input history
    double y1, y2; // Output history

    FilterCfg cfg;
} BiquadFilter;

// Filter functions
void biquad_init(BiquadFilter* filter, const FilterCfg *cfg, double sample_rate);
void biquad_reset(BiquadFilter *filter);
double biquad_process(BiquadFilter *filter, double input);
void biquad_set_cutoff(BiquadFilter *filter, double cutoff, double sample_rate);
void biquad_set_resonance(BiquadFilter *filter, double resonance, double sample_rate);
void biquad_set_type(BiquadFilter *filter, FilterType type, double sample_rate);
