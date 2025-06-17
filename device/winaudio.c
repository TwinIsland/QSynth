#include "audio_device.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct WindowsAudioData {
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeaders[2];
    int16_t* audioBuffers[2];
    int current_buffer;
    HANDLE callback_event;
} WindowsAudioData;

// Global error state
static AudioError g_last_error = AUDIO_ERROR_NONE;

static int set_error(AudioError error) {
    g_last_error = error;
    return 0;
}

// Windows audio callback
static void CALLBACK windows_audio_callback(__attribute__ ((unused)) HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance,
                                           DWORD_PTR dwParam1, __attribute__ ((unused)) DWORD_PTR dwParam2) {
    if (uMsg == WOM_DONE) {
        AudioDevice* device = (AudioDevice*)dwInstance;
        if (!device || !device->is_playing) return;
        
        WindowsAudioData* win_data = (WindowsAudioData*)device->platform_data;
        WAVEHDR* pWaveHdr = (WAVEHDR*)dwParam1;
        
        // Call user callback to fill buffer
        if (device->config.callback) {
            device->config.callback(device->config.user_data, 
                                   (int16_t*)pWaveHdr->lpData, 
                                   device->config.buffer_size);
        }
        
        // Update statistics
        device->frames_processed += device->config.buffer_size;
        
        // Queue buffer back to Windows
        MMRESULT result = waveOutPrepareHeader(win_data->hWaveOut, pWaveHdr, sizeof(WAVEHDR));
        if (result == MMSYSERR_NOERROR) {
            waveOutWrite(win_data->hWaveOut, pWaveHdr, sizeof(WAVEHDR));
        } else {
            device->underrun_count++;
        }
    }
}

int audio_device_init(AudioDevice* device, const AudioConfig* config) {
    if (!device || !config) {
        return set_error(AUDIO_ERROR_INVALID_PARAMETER);
    }
    
    if (device->is_initialized) {
        return set_error(AUDIO_ERROR_ALREADY_INITIALIZED);
    }
    
    // Validate parameters
    if (config->sample_rate <= 0 || config->channels < 1 || config->channels > 8 ||
        config->buffer_size <= 0 || !config->callback) {
        return set_error(AUDIO_ERROR_INVALID_PARAMETER);
    }
    
    // Initialize device structure
    memset(device, 0, sizeof(AudioDevice));
    device->config = *config;
    
    // Allocate platform-specific data
    WindowsAudioData* win_data = (WindowsAudioData*)calloc(1, sizeof(WindowsAudioData));
    if (!win_data) {
        return set_error(AUDIO_ERROR_INIT_FAILED);
    }
    device->platform_data = win_data;
    
    // Allocate audio buffers
    size_t buffer_bytes = config->buffer_size * config->channels * sizeof(int16_t);
    for (int i = 0; i < 2; i++) {
        win_data->audioBuffers[i] = (int16_t*)calloc(1, buffer_bytes);
        if (!win_data->audioBuffers[i]) {
            audio_device_cleanup(device);
            return set_error(AUDIO_ERROR_INIT_FAILED);
        }
    }
    
    // Setup Windows audio format
    WAVEFORMATEX waveFormat = {0};
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = (WORD)config->channels;
    waveFormat.nSamplesPerSec = (DWORD)config->sample_rate;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;
    
    // Open Windows audio device
    MMRESULT result = waveOutOpen(&win_data->hWaveOut, WAVE_MAPPER, &waveFormat,
                                 (DWORD_PTR)windows_audio_callback, (DWORD_PTR)device,
                                 CALLBACK_FUNCTION);
    
    if (result != MMSYSERR_NOERROR) {
        audio_device_cleanup(device);
        
        switch (result) {
            case MMSYSERR_BADDEVICEID:
                return set_error(AUDIO_ERROR_DEVICE_NOT_FOUND);
            case WAVERR_BADFORMAT:
                return set_error(AUDIO_ERROR_FORMAT_NOT_SUPPORTED);
            default:
                return set_error(AUDIO_ERROR_INIT_FAILED);
        }
    }
    
    // Initialize wave headers
    for (int i = 0; i < 2; i++) {
        memset(&win_data->waveHeaders[i], 0, sizeof(WAVEHDR));
        win_data->waveHeaders[i].lpData = (LPSTR)win_data->audioBuffers[i];
        win_data->waveHeaders[i].dwBufferLength = (DWORD)buffer_bytes;
        win_data->waveHeaders[i].dwFlags = 0;
        win_data->waveHeaders[i].dwLoops = 0;
    }
    
    device->is_initialized = 1;
    g_last_error = AUDIO_ERROR_NONE;
    
    printf("Windows audio device initialized: %.1fHz, %d channels, %d frames\n",
           config->sample_rate, config->channels, config->buffer_size);
    
    return 1;
}

int audio_device_start(AudioDevice* device) {
    if (!device || !device->is_initialized) {
        return set_error(AUDIO_ERROR_NOT_INITIALIZED);
    }
    
    if (device->is_playing) {
        return 1; // Already playing
    }
    
    WindowsAudioData* win_data = (WindowsAudioData*)device->platform_data;
    
    // Fill initial buffers with silence and queue them
    size_t buffer_bytes = device->config.buffer_size * device->config.channels * sizeof(int16_t);
    
    for (int i = 0; i < 2; i++) {
        memset(win_data->audioBuffers[i], 0, buffer_bytes);
        
        MMRESULT result = waveOutPrepareHeader(win_data->hWaveOut, 
                                              &win_data->waveHeaders[i], 
                                              sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR) {
            return set_error(AUDIO_ERROR_PLAYBACK_FAILED);
        }
        
        result = waveOutWrite(win_data->hWaveOut, &win_data->waveHeaders[i], sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR) {
            return set_error(AUDIO_ERROR_PLAYBACK_FAILED);
        }
    }
    
    device->is_playing = 1;
    device->frames_processed = 0;
    device->underrun_count = 0;
    
    printf("Windows audio playback started\n");
    return 1;
}

void audio_device_stop(AudioDevice* device) {
    if (!device || !device->is_playing) return;
    
    WindowsAudioData* win_data = (WindowsAudioData*)device->platform_data;
    
    device->is_playing = 0;
    
    // Reset audio device
    waveOutReset(win_data->hWaveOut);
    
    // Unprepare headers
    for (int i = 0; i < 2; i++) {
        waveOutUnprepareHeader(win_data->hWaveOut, &win_data->waveHeaders[i], sizeof(WAVEHDR));
    }
    
    printf("Windows audio playback stopped\n");
}

void audio_device_cleanup(AudioDevice* device) {
    if (!device) return;
    
    audio_device_stop(device);
    
    WindowsAudioData* win_data = (WindowsAudioData*)device->platform_data;
    if (win_data) {
        // Close audio device
        if (win_data->hWaveOut) {
            waveOutClose(win_data->hWaveOut);
        }
        
        // Free audio buffers
        for (int i = 0; i < 2; i++) {
            if (win_data->audioBuffers[i]) {
                free(win_data->audioBuffers[i]);
            }
        }
        
        free(win_data);
        device->platform_data = NULL;
    }
    
    device->is_initialized = 0;
    printf("Windows audio device cleaned up\n");
}

// === UTILITY FUNCTIONS ===

int audio_device_is_playing(const AudioDevice* device) {
    return device ? device->is_playing : 0;
}

double audio_device_get_sample_rate(const AudioDevice* device) {
    return device ? device->config.sample_rate : 0.0;
}

int audio_device_get_buffer_size(const AudioDevice* device) {
    return device ? device->config.buffer_size : 0;
}

double audio_device_get_latency_ms(const AudioDevice* device) {
    if (!device) return 0.0;
    
    // Estimate latency: 2 buffers * buffer_size / sample_rate * 1000
    return (2.0 * device->config.buffer_size / device->config.sample_rate) * 1000.0;
}

const char* audio_device_get_platform_name(void) {
    return "Windows WaveOut";
}

// === ERROR HANDLING ===

AudioError audio_device_get_last_error(void) {
    return g_last_error;
}

const char* audio_device_get_error_string(AudioError error) {
    switch (error) {
        case AUDIO_ERROR_NONE:
            return "No error";
        case AUDIO_ERROR_INIT_FAILED:
            return "Failed to initialize audio device";
        case AUDIO_ERROR_DEVICE_NOT_FOUND:
            return "Audio device not found";
        case AUDIO_ERROR_FORMAT_NOT_SUPPORTED:
            return "Audio format not supported";
        case AUDIO_ERROR_ALREADY_INITIALIZED:
            return "Audio device already initialized";
        case AUDIO_ERROR_NOT_INITIALIZED:
            return "Audio device not initialized";
        case AUDIO_ERROR_PLAYBACK_FAILED:
            return "Failed to start audio playback";
        case AUDIO_ERROR_INVALID_PARAMETER:
            return "Invalid parameter";
        default:
            return "Unknown error";
    }
}


