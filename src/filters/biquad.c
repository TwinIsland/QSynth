#include "biquad.h"
#include <math.h>
#include <string.h>

#include "../utils/constant.h"

static void calculate_coefficients(BiquadFilter* filter, double sample_rate) {
    if (filter->cfg.filter_type == FILTER_NONE) return;
    
    double omega = 2.0 * M_PI * filter->cfg.cutoff / sample_rate;
    double sin_omega = sin(omega);
    double cos_omega = cos(omega);
    double alpha = sin_omega / (2.0 * filter->cfg.resonance);
    
    double a0_unnorm, a1, a2, b0, b1, b2;
    
    switch (filter->cfg.filter_type) {
        case FILTER_LOWPASS:
            b0 = (1.0 - cos_omega) / 2.0;
            b1 = 1.0 - cos_omega;
            b2 = (1.0 - cos_omega) / 2.0;
            a0_unnorm = 1.0 + alpha;
            a1 = -2.0 * cos_omega;
            a2 = 1.0 - alpha;
            break;
            
        case FILTER_HIGHPASS:
            b0 = (1.0 + cos_omega) / 2.0;
            b1 = -(1.0 + cos_omega);
            b2 = (1.0 + cos_omega) / 2.0;
            a0_unnorm = 1.0 + alpha;
            a1 = -2.0 * cos_omega;
            a2 = 1.0 - alpha;
            break;
            
        case FILTER_BANDPASS:
            b0 = alpha;
            b1 = 0.0;
            b2 = -alpha;
            a0_unnorm = 1.0 + alpha;
            a1 = -2.0 * cos_omega;
            a2 = 1.0 - alpha;
            break;
            
        case FILTER_NOTCH:
            b0 = 1.0;
            b1 = -2.0 * cos_omega;
            b2 = 1.0;
            a0_unnorm = 1.0 + alpha;
            a1 = -2.0 * cos_omega;
            a2 = 1.0 - alpha;
            break;
            
        default:
            return;
    }
    
    // Normalize coefficients
    filter->a0 = b0 / a0_unnorm;
    filter->a1 = b1 / a0_unnorm;
    filter->a2 = b2 / a0_unnorm;
    filter->b1 = a1 / a0_unnorm;
    filter->b2 = a2 / a0_unnorm;
}

void biquad_init(BiquadFilter* filter, const FilterCfg *cfg, double sample_rate) {
    memset(filter, 0, sizeof(BiquadFilter));
    filter->cfg = *cfg; 
    biquad_reset(filter);
    calculate_coefficients(filter, sample_rate);
}

void biquad_reset(BiquadFilter* filter) {
    filter->x1 = filter->x2 = 0.0;
    filter->y1 = filter->y2 = 0.0;
}

double biquad_process(BiquadFilter* filter, double input) {
    if (filter->cfg.filter_type == FILTER_NONE) return input;
    
    double output = filter->a0 * input + 
                   filter->a1 * filter->x1 + 
                   filter->a2 * filter->x2 -
                   filter->b1 * filter->y1 - 
                   filter->b2 * filter->y2;
    
    // Shift delay lines
    filter->x2 = filter->x1;
    filter->x1 = input;
    filter->y2 = filter->y1;
    filter->y1 = output;
    
    return output;
}

void biquad_set_cutoff(BiquadFilter* filter, double cutoff, double sample_rate) {
    if (filter->cfg.cutoff != cutoff) {
        filter->cfg.cutoff = cutoff;
        calculate_coefficients(filter, sample_rate);
    }
}

void biquad_set_resonance(BiquadFilter* filter, double resonance, double sample_rate) {
    if (filter->cfg.resonance != resonance) {
        filter->cfg.resonance = resonance;
        calculate_coefficients(filter, sample_rate);
    }
}

void biquad_set_type(BiquadFilter* filter, FilterType type, double sample_rate) {
    if (filter->cfg.resonance != type) {
        filter->cfg.resonance = type;
        calculate_coefficients(filter, sample_rate);
    }
}
