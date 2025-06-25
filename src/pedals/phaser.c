#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "phaser.h"

#define NUM_STAGES 4  // Number of allpass filter stages

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Allpass filter for phase shifting
typedef struct {
    double delay;
    double feedback;
    double state;
} allpass_stage_t;

typedef struct {
    double sample_rate;
    
    // Allpass filter stages
    allpass_stage_t stages[NUM_STAGES];
    
    // LFO for sweeping
    double lfo_phase;
    
    // Effect parameters  
    double rate;           // LFO rate (0.1-10 Hz)
    double depth;          // Sweep depth (0.0-1.0)
    double feedback;       // Overall feedback (0.0-0.9)
    double wet_dry_mix;    // Wet/dry mix (0.0-1.0)
    double center_freq;    // Center frequency (100-2000 Hz)
    
    bool initialized;
} phaser_instance_t;

// Process sample through allpass filter
static double process_allpass(allpass_stage_t *stage, double input) {
    double output = -input + stage->state;
    stage->state = input + stage->feedback * output;
    return output;
}

// Calculate allpass filter coefficients based on frequency
static double freq_to_allpass_coeff(double frequency, double sample_rate) {
    double omega = 2.0 * M_PI * frequency / sample_rate;
    return (1.0 - tan(omega / 2.0)) / (1.0 + tan(omega / 2.0));
}

// Create phaser instance
bool phaser_create(void **instance_ptr, double sample_rate) {
    if (!instance_ptr || sample_rate <= 0) return false;
    
    phaser_instance_t *phaser = malloc(sizeof(phaser_instance_t));
    if (!phaser) return false;
    
    memset(phaser, 0, sizeof(phaser_instance_t));
    phaser->sample_rate = sample_rate;
    
    // Initialize allpass stages
    for (int i = 0; i < NUM_STAGES; i++) {
        phaser->stages[i].state = 0.0;
        phaser->stages[i].feedback = 0.7;  // Default feedback
    }
    
    // Set default parameters
    phaser->rate = 0.5;           // 0.5 Hz sweep rate
    phaser->depth = 0.8;          // 80% depth
    phaser->feedback = 0.6;       // 60% feedback
    phaser->wet_dry_mix = 0.5;    // 50% wet
    phaser->center_freq = 800.0;  // 800 Hz center frequency
    
    phaser->lfo_phase = 0.0;
    phaser->initialized = true;
    
    *instance_ptr = phaser;
    return true;
}

// Destroy phaser instance
void phaser_destroy(void *instance) {
    if (!instance) return;
    free(instance);
}

// Process single sample through phaser
double phaser_process(void *instance, double sample) {
    if (!instance) return sample;
    
    phaser_instance_t *phaser = (phaser_instance_t*)instance;
    if (!phaser->initialized) return sample;
    
    // Generate LFO (sine wave for smooth sweeping)
    double lfo_value = sin(phaser->lfo_phase);
    
    // Calculate sweep frequency based on LFO and depth
    double freq_variation = phaser->depth * phaser->center_freq * 0.8;  // 80% of center freq max variation
    double sweep_freq = phaser->center_freq + lfo_value * freq_variation;
    
    // Ensure frequency stays in reasonable range
    if (sweep_freq < 50.0) sweep_freq = 50.0;
    if (sweep_freq > 4000.0) sweep_freq = 4000.0;
    
    // Update allpass filter coefficients for each stage
    // Use different frequencies for each stage to create richer effect
    for (int i = 0; i < NUM_STAGES; i++) {
        double stage_freq = sweep_freq * (1.0 + i * 0.3);  // Spread out frequencies
        phaser->stages[i].feedback = freq_to_allpass_coeff(stage_freq, phaser->sample_rate);
    }
    
    // Process through allpass filter chain
    double processed = sample;
    for (int i = 0; i < NUM_STAGES; i++) {
        processed = process_allpass(&phaser->stages[i], processed);
    }
    
    // Apply overall feedback (creates resonance peaks)
    static double feedback_state = 0.0;
    processed += feedback_state * phaser->feedback;
    feedback_state = processed * 0.5;  // Store for next sample
    
    // Mix wet and dry signals
    double wet_signal = processed * phaser->wet_dry_mix;
    double dry_signal = sample * (1.0 - phaser->wet_dry_mix);
    
    // Update LFO phase
    double lfo_increment = 2.0 * M_PI * phaser->rate / phaser->sample_rate;
    phaser->lfo_phase += lfo_increment;
    if (phaser->lfo_phase >= 2.0 * M_PI) {
        phaser->lfo_phase -= 2.0 * M_PI;
    }
    
    return wet_signal + dry_signal;
}

// Set phaser parameters
void phaser_set_params(void *instance, double params[PEDAL_MAX_PARAMS]) {
    if (!instance || !params) return;
    
    phaser_instance_t *phaser = (phaser_instance_t*)instance;
    if (!phaser->initialized) return;
    
    // Update parameters with bounds checking
    phaser->rate = fmax(0.1, fmin(10.0, params[0]));           // 0.1-10 Hz rate
    phaser->depth = fmax(0.0, fmin(1.0, params[1]));           // 0-100% depth
    phaser->feedback = fmax(0.0, fmin(0.9, params[2]));        // 0-90% feedback
    phaser->wet_dry_mix = fmax(0.0, fmin(1.0, params[3]));     // 0-100% wet
    phaser->center_freq = fmax(100.0, fmin(2000.0, params[4])); // 100-2000 Hz center
}
