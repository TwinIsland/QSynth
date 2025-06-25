#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "reverb.h"

#define MAX_COMB_FILTERS 4
#define MAX_ALLPASS_FILTERS 2
#define MAX_DELAY_SAMPLES 48000 // 1 second at 48kHz max

// Comb filter structure
typedef struct
{
    double *buffer;
    int buffer_size;
    int write_index;
    double feedback;
    double damping;
    double filter_state;
} comb_filter_t;

// Allpass filter structure
typedef struct
{
    double *buffer;
    int buffer_size;
    int write_index;
    double feedback;
} allpass_filter_t;

// Pre-delay line structure
typedef struct
{
    double *buffer;
    int buffer_size;
    int write_index;
} delay_line_t;

// Main reverb instance structure
typedef struct
{
    double sample_rate;

    // Filter arrays
    comb_filter_t comb_filters[MAX_COMB_FILTERS];
    allpass_filter_t allpass_filters[MAX_ALLPASS_FILTERS];
    delay_line_t predelay;

    // Parameters
    double room_size;    // 0.0 - 1.0
    double decay_time;   // 0.1 - 10.0 seconds
    double damping;      // 0.0 - 1.0
    double wet_dry_mix;  // 0.0 - 1.0
    double predelay_ms;  // 0 - 100ms
    double output_level; // 0-1

    bool initialized;
} reverb_instance_t;

// Comb filter delay times (in samples at 44.1kHz, scaled for actual sample rate)
static const int comb_delays[] = {1116, 1188, 1277, 1356};

// Allpass filter delay times (in samples at 44.1kHz, scaled for actual sample rate)
static const int allpass_delays[] = {556, 441};

// Initialize a comb filter
static bool init_comb_filter(comb_filter_t *filter, int delay_samples)
{
    filter->buffer = (double *)calloc(delay_samples, sizeof(double));
    if (!filter->buffer)
        return false;

    filter->buffer_size = delay_samples;
    filter->write_index = 0;
    filter->feedback = 0.5;
    filter->damping = 0.5;
    filter->filter_state = 0.0;

    return true;
}

// Initialize an allpass filter
static bool init_allpass_filter(allpass_filter_t *filter, int delay_samples)
{
    filter->buffer = (double *)calloc(delay_samples, sizeof(double));
    if (!filter->buffer)
        return false;

    filter->buffer_size = delay_samples;
    filter->write_index = 0;
    filter->feedback = 0.7;

    return true;
}

// Initialize delay line
static bool init_delay_line(delay_line_t *delay, int max_delay_samples)
{
    delay->buffer = (double *)calloc(max_delay_samples, sizeof(double));
    if (!delay->buffer)
        return false;

    delay->buffer_size = max_delay_samples;
    delay->write_index = 0;

    return true;
}

// Process comb filter
static double process_comb_filter(comb_filter_t *filter, double input)
{
    int read_index = filter->write_index;
    double delayed = filter->buffer[read_index];

    // Apply damping (low-pass filtering)
    filter->filter_state = delayed * (1.0 - filter->damping) +
                           filter->filter_state * filter->damping;

    // Store input + feedback
    filter->buffer[filter->write_index] = input + filter->filter_state * filter->feedback;

    // Update write index
    filter->write_index = (filter->write_index + 1) % filter->buffer_size;

    return delayed;
}

// Process allpass filter
static double process_allpass_filter(allpass_filter_t *filter, double input)
{
    int read_index = filter->write_index;
    double delayed = filter->buffer[read_index];
    double output = -input + delayed;

    filter->buffer[filter->write_index] = input + delayed * filter->feedback;
    filter->write_index = (filter->write_index + 1) % filter->buffer_size;

    return output;
}

// Process delay line
static double process_delay_line(delay_line_t *delay, double input, int delay_samples)
{
    if (delay_samples <= 0)
        return input;
    if (delay_samples >= delay->buffer_size)
        delay_samples = delay->buffer_size - 1;

    int read_index = (delay->write_index - delay_samples + delay->buffer_size) % delay->buffer_size;
    double delayed = delay->buffer[read_index];

    delay->buffer[delay->write_index] = input;
    delay->write_index = (delay->write_index + 1) % delay->buffer_size;

    return delayed;
}

// Create reverb pedal instance
bool reverb_create(void **instance_ptr, double sample_rate)
{
    if (!instance_ptr || sample_rate <= 0)
        return false;

    reverb_instance_t *reverb = (reverb_instance_t *)malloc(sizeof(reverb_instance_t));
    if (!reverb)
        return false;

    memset(reverb, 0, sizeof(reverb_instance_t));
    reverb->sample_rate = sample_rate;

    // Scale delay times based on sample rate (reference is 44.1kHz)
    double scale_factor = sample_rate / 44100.0;

    // Initialize comb filters
    for (int i = 0; i < MAX_COMB_FILTERS; i++)
    {
        int delay_samples = (int)(comb_delays[i] * scale_factor);
        if (!init_comb_filter(&reverb->comb_filters[i], delay_samples))
        {
            reverb_destroy(reverb);
            return false;
        }
    }

    // Initialize allpass filters
    for (int i = 0; i < MAX_ALLPASS_FILTERS; i++)
    {
        int delay_samples = (int)(allpass_delays[i] * scale_factor);
        if (!init_allpass_filter(&reverb->allpass_filters[i], delay_samples))
        {
            reverb_destroy(reverb);
            return false;
        }
    }

    // Initialize pre-delay
    if (!init_delay_line(&reverb->predelay, MAX_DELAY_SAMPLES))
    {
        reverb_destroy(reverb);
        return false;
    }

    // Set default parameters
    reverb->room_size = 0.5;
    reverb->decay_time = 2.0;
    reverb->damping = 0.5;
    reverb->wet_dry_mix = 0.3;
    reverb->predelay_ms = 20.0;
    reverb->initialized = true;

    *instance_ptr = reverb;
    return true;
}

// Destroy reverb pedal instance
void reverb_destroy(void *instance)
{
    if (!instance)
        return;

    reverb_instance_t *reverb = (reverb_instance_t *)instance;

    // Free comb filter buffers
    for (int i = 0; i < MAX_COMB_FILTERS; i++)
    {
        if (reverb->comb_filters[i].buffer)
        {
            free(reverb->comb_filters[i].buffer);
        }
    }

    // Free allpass filter buffers
    for (int i = 0; i < MAX_ALLPASS_FILTERS; i++)
    {
        if (reverb->allpass_filters[i].buffer)
        {
            free(reverb->allpass_filters[i].buffer);
        }
    }

    // Free pre-delay buffer
    if (reverb->predelay.buffer)
    {
        free(reverb->predelay.buffer);
    }

    free(reverb);
}

// Process a single sample through the reverb
double reverb_process(void *instance, double sample)
{
    // if (sample == 0.0) return 0.0;
    if (!instance)
        return sample;

    reverb_instance_t *reverb = (reverb_instance_t *)instance;
    if (!reverb->initialized)
        return sample;

    // Apply pre-delay
    int predelay_samples = (int)(reverb->predelay_ms * reverb->sample_rate / 1000.0);
    double delayed_input = process_delay_line(&reverb->predelay, sample, predelay_samples);

    // Process through comb filters (parallel)
    double comb_output = 0.0;
    for (int i = 0; i < MAX_COMB_FILTERS; i++)
    {
        comb_output += process_comb_filter(&reverb->comb_filters[i], delayed_input);
    }

    // Process through allpass filters (series)
    double output = comb_output;
    for (int i = 0; i < MAX_ALLPASS_FILTERS; i++)
    {
        output = process_allpass_filter(&reverb->allpass_filters[i], output);
    }

    // Apply wet/dry mix
    double wet_signal = output * reverb->wet_dry_mix;
    double dry_signal = sample * (1.0 - reverb->wet_dry_mix);

    return reverb->output_level * (wet_signal + dry_signal);
}

// Set reverb parameters
void reverb_set_params(void *instance, double params[PEDAL_MAX_PARAMS])
{
    if (!instance || !params)
        return;

    reverb_instance_t *reverb = (reverb_instance_t *)instance;
    if (!reverb->initialized)
        return;

    // Update parameters with bounds checking
    reverb->room_size = fmax(0.0, fmin(1.0, params[0]));
    reverb->decay_time = fmax(0.1, fmin(10.0, params[1]));
    reverb->damping = fmax(0.0, fmin(1.0, params[2]));
    reverb->wet_dry_mix = fmax(0.0, fmin(1.0, params[3]));
    reverb->predelay_ms = fmax(0.0, fmin(100.0, params[4]));
    reverb->output_level = fmax(0.0, fmin(2.0, params[5]));

    // Update comb filter parameters based on room size and decay time
    for (int i = 0; i < MAX_COMB_FILTERS; i++)
    {
        // Calculate feedback based on decay time and delay length
        double delay_time = (double)reverb->comb_filters[i].buffer_size / reverb->sample_rate;
        reverb->comb_filters[i].feedback = pow(0.001, delay_time / reverb->decay_time) * reverb->room_size;
        reverb->comb_filters[i].damping = reverb->damping;
    }

    // Update allpass filter feedback based on room size
    for (int i = 0; i < MAX_ALLPASS_FILTERS; i++)
    {
        reverb->allpass_filters[i].feedback = 0.7 * reverb->room_size;
    }
}