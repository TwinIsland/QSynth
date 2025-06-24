// ===============================================
// miniaudio_test.c - Simple 5 Second Sine Wave Test
// ===============================================
#include "../src/audio/miniaudio.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#define SLEEP(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP(ms) usleep((ms) * 1000)
#endif

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
void sine_wave_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    printf("called\n");
    SineWaveData* sine_data = (SineWaveData*)pDevice->pUserData;
    int16_t* output_buffer = (int16_t*)pOutput;

    // Suppress unused parameter warning
    (void)pInput;

    for (ma_uint32 i = 0; i < frameCount; i++)
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
    printf("Playing 880Hz sine wave for 5 seconds...\n");

    // Setup sine wave generator
    SineWaveData sine_data = {
        .phase = 0.0,
        .frequency = 880.0, // A5 note (higher than A4)
        .amplitude = 0.5,   // 50% volume
        .sample_rate = 44100.0
    };

    // Setup audio device configuration
    ma_device_config deviceConfig;
    ma_device device;
    ma_result result;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_s16;    // 16-bit signed integer
    deviceConfig.playback.channels = 2;                // Stereo
    deviceConfig.sampleRate        = 44100;            // Sample rate
    deviceConfig.dataCallback      = sine_wave_callback;
    deviceConfig.pUserData         = &sine_data;       // User data

    // Initialize audio device
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio device: %s\n", ma_result_description(result));
        return 1;
    }

    // Print device information
    printf("Device Info:\n");
    printf("  Sample Rate: %u Hz\n", device.sampleRate);
    printf("  Channels: %u\n", device.playback.channels);
    printf("  Format: %s\n", ma_get_format_name(device.playback.format));

    // Start audio playback
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        printf("Failed to start audio: %s\n", ma_result_description(result));
        ma_device_uninit(&device);
        return 1;
    }

    printf("Playing... ");
    fflush(stdout);

    // Check if device is actually started
    if (ma_device_is_started(&device)) {
        printf("(device confirmed started)\n");
    } else {
        printf("(warning: device not started)\n");
    }

    // Play for 5 seconds
    SLEEP(5000);

    printf("Done!\n");

    // Stop and cleanup
    ma_device_stop(&device);
    ma_device_uninit(&device);

    printf("Audio device stopped and cleaned up.\n");

    return 0;
}