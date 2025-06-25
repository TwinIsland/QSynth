#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define PEDAL_MAX_PARAMS 8

typedef struct {
    double sample_rate;
    
    // Effect parameters
    double gain;           // Input gain (1.0-20.0)
    double drive;          // Distortion amount (0.0-1.0)
    double tone;           // Tone control (0.0-1.0)
    double output_level;   // Output level (0.0-2.0)
    double asymmetry;      // Asymmetric clipping (0.0-1.0)
    
    // Simple tone filter state
    double low_pass_state;
    double high_pass_state;
    
    bool initialized;
} distortion_instance_t;

// Soft clipping function - creates warm overdrive
static double soft_clip(double input, double drive) {
    double driven = input * (1.0 + drive * 4.0);  // Amplify based on drive
    
    // Soft clipping using hyperbolic tangent
    return tanh(driven) * 0.7;  // Scale down to prevent excessive volume
}

// Asymmetric clipping - adds even harmonics
static double asymmetric_clip(double input, double asymmetry) {
    if (input >= 0.0) {
        return input;  // Positive half unchanged
    } else {
        // Compress negative half based on asymmetry
        return input * (1.0 - asymmetry * 0.5);
    }
}

// Simple tone control (combined high and low pass)
static double apply_tone(distortion_instance_t *dist, double input) {
    // Low pass filter coefficient (tone = 0 = dark, tone = 1 = bright)
    double lp_cutoff = 0.1 + dist->tone * 0.4;  // 0.1 to 0.5
    dist->low_pass_state += lp_cutoff * (input - dist->low_pass_state);
    
    // High pass filter to remove DC offset
    double hp_cutoff = 0.02;
    dist->high_pass_state += hp_cutoff * (input - dist->high_pass_state);
    double hp_output = input - dist->high_pass_state;
    
    // Blend between filtered and original based on tone setting
    return dist->low_pass_state * (1.0 - dist->tone * 0.3) + hp_output * (dist->tone * 0.3);
}

// Create distortion instance
bool distortion_create(void **instance_ptr, double sample_rate) {
    if (!instance_ptr || sample_rate <= 0) return false;
    
    distortion_instance_t *dist = malloc(sizeof(distortion_instance_t));
    if (!dist) return false;
    
    memset(dist, 0, sizeof(distortion_instance_t));
    dist->sample_rate = sample_rate;
    
    // Set default parameters
    dist->gain = 3.0;           // 3x input gain
    dist->drive = 0.6;          // 60% drive
    dist->tone = 0.7;           // Bright tone
    dist->output_level = 0.8;   // Slightly reduced output
    dist->asymmetry = 0.3;      // Some asymmetric clipping
    
    dist->low_pass_state = 0.0;
    dist->high_pass_state = 0.0;
    dist->initialized = true;
    
    *instance_ptr = dist;
    return true;
}

// Destroy distortion instance
void distortion_destroy(void *instance) {
    if (!instance) return;
    free(instance);
}

// Process single sample through distortion
double distortion_process(void *instance, double sample) {
    if (!instance) return sample;
    
    distortion_instance_t *dist = (distortion_instance_t*)instance;
    if (!dist->initialized) return sample;
    
    // Apply input gain
    double gained_sample = sample * dist->gain;
    
    // Apply asymmetric clipping first
    double asymmetric_sample = asymmetric_clip(gained_sample, dist->asymmetry);
    
    // Apply soft clipping distortion
    double distorted_sample = soft_clip(asymmetric_sample, dist->drive);
    
    // Apply tone control
    double toned_sample = apply_tone(dist, distorted_sample);
    
    // Apply output level
    double output = toned_sample * dist->output_level;
    
    // Final hard limiting to prevent digital clipping
    if (output > 1.0) output = 1.0;
    if (output < -1.0) output = -1.0;
    
    return output;
}

// Set distortion parameters
void distortion_set_params(void *instance, double params[PEDAL_MAX_PARAMS]) {
    if (!instance || !params) return;
    
    distortion_instance_t *dist = (distortion_instance_t*)instance;
    if (!dist->initialized) return;
    
    // Update parameters with bounds checking
    dist->gain = fmax(1.0, fmin(20.0, params[0]));           // 1x to 20x gain
    dist->drive = fmax(0.0, fmin(1.0, params[1]));           // 0-100% drive
    dist->tone = fmax(0.0, fmin(1.0, params[2]));            // 0-100% tone
    dist->output_level = fmax(0.0, fmin(2.0, params[3]));    // 0-200% output
    dist->asymmetry = fmax(0.0, fmin(1.0, params[4]));       // 0-100% asymmetry
}