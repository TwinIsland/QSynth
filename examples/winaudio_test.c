// ===============================================
// windaudio_test.c - Simple 5 Second Sine Wave Test
// ===============================================
#include "audio_device.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Simple sine wave data
typedef struct
{
    double phase;       // Current phase of sine wave
    double frequency;   // Frequency in Hz (440Hz = A4)
    double amplitude;   // Volume (0.0 to 1.0)
    double sample_rate; // Sample rate (44100Hz)
} SineWaveData;

// Audio callback - generates sine wave
void sine_wave_callback(void *user_data, int16_t *output_buffer, int frame_count)
{
    SineWaveData *sine_data = (SineWaveData *)user_data;

    for (int i = 0; i < frame_count; i++)
    {
        // Generate sine wave sample
        double sine_value = sin(sine_data->phase) * sine_data->amplitude;
        int16_t sample = (int16_t)(sine_value * 32767);

        // Output to both stereo channels
        output_buffer[i * 2] = sample;     // Left channel
        output_buffer[i * 2 + 1] = sample; // Right channel

        // Update phase for next sample
        sine_data->phase += 2.0 * M_PI * sine_data->frequency / sine_data->sample_rate;
        if (sine_data->phase >= 2.0 * M_PI)
        {
            sine_data->phase -= 2.0 * M_PI;
        }
    }
}

int main(void)
{
    printf("Playing 440Hz sine wave for 5 seconds...\n");

    // Setup sine wave generator
    SineWaveData sine_data = {
        .phase = 0.0,
        .frequency = 880.0, // A4 note
        .amplitude = 0.5,   // 50% volume
        .sample_rate = 44100.0};

    // Setup audio configuration
    AudioConfig config = {
        .sample_rate = 44100.0,
        .channels = 2, // Stereo
        .buffer_size = 4410,
        .callback = sine_wave_callback,
        .user_data = &sine_data};

    // Initialize audio device
    AudioDevice device = {0};
    if (!audio_device_init(&device, &config))
    {
        printf("Failed to initialize audio device: %s\n",
               audio_device_get_error_string(audio_device_get_last_error()));
        return 1;
    }

    // Start audio playback
    if (!audio_device_start(&device))
    {
        printf("Failed to start audio: %s\n",
               audio_device_get_error_string(audio_device_get_last_error()));
        audio_device_cleanup(&device);
        return 1;
    }

    printf("Playing... ");
    fflush(stdout);

    // Play for 5 seconds
    Sleep(5000);

    printf("Done!\n");

    // Stop and cleanup
    audio_device_stop(&device);
    audio_device_cleanup(&device);

    return 0;
}