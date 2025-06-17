#pragma once

#include <stdint.h>

typedef struct AudioDevice AudioDevice;
typedef struct AudioConfig AudioConfig;

// Audio callback function type
// Called when the audio system needs more data
// Parameters: user_data, output_buffer, frame_count
// frame_count = number of sample frames (not individual samples)
// For stereo: frame_count * 2 = total samples in buffer
typedef void (*AudioCallback)(void* user_data, int16_t* output_buffer, int frame_count);

// Audio configuration
typedef struct AudioConfig {
    double sample_rate;      // 44100.0, 48000.0, etc.
    int channels;            // 1 = mono, 2 = stereo
    int buffer_size;         // Frames per buffer (e.g., 512, 1024)
    AudioCallback callback;  // Function to call for audio data
    void* user_data;         // Passed to callback function
} AudioConfig;

// Platform-independent audio device
typedef struct AudioDevice {
    // Configuration
    AudioConfig config;
    
    // State
    int is_initialized;
    int is_playing;
    
    // Platform-specific data (opaque pointer)
    void* platform_data;
    
    // Statistics
    uint64_t frames_processed;
    uint32_t underrun_count;
} AudioDevice;

// === CORE API ===

// Initialize audio device with configuration
// Returns: 1 on success, 0 on failure
int audio_device_init(AudioDevice* device, const AudioConfig* config);

// Start audio playback
// Returns: 1 on success, 0 on failure
int audio_device_start(AudioDevice* device);

// Stop audio playback
void audio_device_stop(AudioDevice* device);

// Cleanup and release resources
void audio_device_cleanup(AudioDevice* device);

// === UTILITY FUNCTIONS ===

// Get current playback state
int audio_device_is_playing(const AudioDevice* device);

// Get actual sample rate (may differ from requested)
double audio_device_get_sample_rate(const AudioDevice* device);

// Get actual buffer size (may differ from requested)
int audio_device_get_buffer_size(const AudioDevice* device);

// Get audio latency in milliseconds
double audio_device_get_latency_ms(const AudioDevice* device);

// Get platform name
const char* audio_device_get_platform_name(void);

// === ERROR HANDLING ===

typedef enum {
    AUDIO_ERROR_NONE = 0,
    AUDIO_ERROR_INIT_FAILED,
    AUDIO_ERROR_DEVICE_NOT_FOUND,
    AUDIO_ERROR_FORMAT_NOT_SUPPORTED,
    AUDIO_ERROR_ALREADY_INITIALIZED,
    AUDIO_ERROR_NOT_INITIALIZED,
    AUDIO_ERROR_PLAYBACK_FAILED,
    AUDIO_ERROR_INVALID_PARAMETER
} AudioError;

// Get last error
AudioError audio_device_get_last_error(void);

// Get error description
const char* audio_device_get_error_string(AudioError error);

// === DEVICE ENUMERATION ===

typedef struct AudioDeviceInfo {
    int device_id;
    char name[256];
    int max_channels;
    double default_sample_rate;
    int is_default;
} AudioDeviceInfo;
