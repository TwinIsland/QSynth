#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "qsynth.h"
#include "core.h"
#include "instruments.h"

#include "../assets/instruments_core.h"
#include "../utils/note_table.h"

#ifdef _WIN32
#define _TIMESPEC_DEFINED
#endif
#include "pthread.h"

#ifdef _WIN32
#include <windows.h>
#define GET_TIME_MS() GetTickCount()
#else
#include <time.h>
static inline uint64_t GET_TIME_MS()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
#endif

static QSynthError g_last_error = QSYNTH_ERROR_NONE;
pthread_t voice_dp_generator_workers[MAX_VOICE_ACTIVE];

static int set_error(QSynthError error)
{
    g_last_error = error;
    return 0;
}

static void audio_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount)
{
    (void *)pInput;

    // in our case, frame_count*channel is always AUDIO_BUFFER_SIZE
    Synthesizer *synth = (Synthesizer *)(pDevice->pUserData);
    int16_t *output_buffer = (int16_t *)pOutput;

    if (!synth)
        return;

    for (ma_uint32 i = 0; i < frameCount; i++)
    {
        double left_mix = 0.0, right_mix = 0.0;
        synth->voice_active = 0;

        // process all voices for this sample
        for (int v = 0; v < MAX_VOICE_ACTIVE; v++)
        {
            Voice *voice = &synth->voices[v];
            if (!voice->active)
                continue;

            if (stream_available(&voice->streamer) == 0)
            {
                uint64_t start_time = GET_TIME_MS();
                while (stream_available(&voice->streamer) == 0)
                    ;
                synth->latency_ms += GET_TIME_MS() - start_time;
            }

            double sample = stream_readDouble(&voice->streamer);

            // apply panning
            double left_gain = 1.0 - voice->pan;
            double right_gain = voice->pan;

            left_mix += sample * left_gain;
            right_mix += sample * right_gain;
            synth->voice_active++;
            synth->samples_played++;
        }

        // apply master volume and clamp
        left_mix *= synth->master_volume;
        right_mix *= synth->master_volume;

        if (left_mix > 1.0)
            left_mix = 1.0;
        if (left_mix < -1.0)
            left_mix = -1.0;
        if (right_mix > 1.0)
            right_mix = 1.0;
        if (right_mix < -1.0)
            right_mix = -1.0;

        // to 16bit
        output_buffer[i * 2] = (int16_t)(left_mix * 32767);
        output_buffer[i * 2 + 1] = (int16_t)(right_mix * 32767);

        // update recent sample
        synth->recent_samples[synth->recent_samples_writeptr] = output_buffer[i * 2];
        synth->recent_samples_writeptr = (synth->recent_samples_writeptr + 1) & RECENT_SAMPLE_MASK;

        synth->recent_samples[synth->recent_samples_writeptr] = output_buffer[i * 2 + 1];
        synth->recent_samples_writeptr = (synth->recent_samples_writeptr + 1) & RECENT_SAMPLE_MASK;
    }
}

struct voice_dp_generator_args
{
    Synthesizer *synth;
    int voice_index;
};

static void *voice_dp_generator(void *arg)
{
    struct voice_dp_generator_args *args = (struct voice_dp_generator_args *)arg;
    Synthesizer *synth = args->synth;
    int voice_index = args->voice_index;
    free(args);

    if (!synth)
        return NULL;

    Voice *voice = &synth->voices[voice_index];

    while (synth->voice_dp_generator_running)
    {
        if (voice->active && stream_fillRatio(&voice->streamer) <= VOICE_BUFFER_REFILL_THRESHOLD)
        {
            int refill_count = 0;
            while (stream_space(&voice->streamer) > 0 && refill_count++ < REFILL_CHUNK_SIZE)
            {
                stream_writeDouble(&voice->streamer, voice_step(voice, synth->delta_time));
            }
        }
        SLEEP_MS(1);
    }

    return NULL;
}

static bool synth_precheck(double sample_rate, int channels)
{
    if (!(VOICE_BUFFER_SIZE > 0 && (VOICE_BUFFER_SIZE & (VOICE_BUFFER_SIZE - 1)) == 0))
    {
        printf("VOICE_BUFFER_SIZE must be a power of 2\n");
        set_error(QSYNTH_ERROR_CONFIG);
        return false;
    }
    if (channels != 2)
    {
        printf("QSynth only support 2 channel audio currently\n");
        set_error(QSYNTH_ERROR_UNSUPPORT);
        return false;
    }

    if (sample_rate < 8000.0 || sample_rate > 192000.0)
    {
        printf("Sample rate must be between 8000Hz and 192000Hz, your set to %f\n", sample_rate);
        set_error(QSYNTH_ERROR_CONFIG);
        return false;
    }

    printf("QSynth pre-check passed\n");
    return true;
}

bool synth_init(Synthesizer **synth_ptr, double sample_rate, int channels)
{
    if (!synth_precheck(sample_rate, channels))
    {
        printf("QSynth pre-check failed\n");
        return false;
    }

    // init note table
    init_note_table();

    // init synthesizer
    Synthesizer *synth = (Synthesizer *)calloc(1, sizeof(Synthesizer));
    if (!synth)
    {
        set_error(QSYNTH_ERROR_MEMALLOC);
        return false;
    };

    *synth_ptr = synth;

    // init audio device
    ma_device_config deviceConfig;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_s16;
    deviceConfig.playback.channels = channels;
    deviceConfig.sampleRate = sample_rate;
    deviceConfig.dataCallback = audio_callback;
    deviceConfig.pUserData = synth;

    ma_result ret = ma_device_init(NULL, &deviceConfig, &synth->device);

    if (ret != MA_SUCCESS)
    {
        set_error(QSYNTH_ERROR_DEVICE);
        printf("audio device init failed: %s\n", ma_result_description(ret));
        return false;
    }

    // init voice
    for (int i = 0; i < MAX_VOICE_ACTIVE; i++)
    {
        voice_init(&synth->voices[i]);
    }

    // init global settings
    synth->master_volume = 0.5;

    // pre-compute attributes
    synth->delta_time = 1.0 / sample_rate;

    // init state
    synth->samples_played = 0;
    synth->voice_dp_generator_running = false;
    synth->voice_active = 0;

    memset(synth->recent_samples, 0, sizeof(synth->recent_samples));
    synth->recent_samples_writeptr = 0;

    printf("QSynth initialized: %.1fHz, %d channels\n", sample_rate, channels);
    return true;
}

void synth_cleanup(Synthesizer *synth)
{
    if (!synth)
        return;

    synth_stop(synth);
    ma_device_uninit(&synth->device);

    free(synth);
    printf("QSynth cleaned up\n");
}

bool synth_start(Synthesizer *synth)
{
    ma_result ret = ma_device_start(&synth->device);

    if (ret != MA_SUCCESS)
    {
        printf("Failed to start audio: %s\n", ma_result_description(ret));
        return false;
    }

    synth->voice_dp_generator_running = true;
    // start voice DP generator thread
    for (int i = 0; i < MAX_VOICE_ACTIVE; i++)
    {
        struct voice_dp_generator_args *args = malloc(sizeof(struct voice_dp_generator_args));
        args->synth = synth;
        args->voice_index = i;

        if (pthread_create(&voice_dp_generator_workers[i], NULL, voice_dp_generator, args) != 0)
        {
            free(args);
            printf("Failed to create voice DP generator thread %d\n", i);

            synth_cleanup(synth);
            set_error(QSYNTH_ERROR_WORKER);
            return false;
        }
    }
    printf("Voice DP generator threads(%zu) created\n", sizeof(voice_dp_generator_workers) / sizeof(pthread_t));

    printf("Audio playback started\n");
    return true;
}

void synth_stop(Synthesizer *synth)
{
    if (!synth)
        return;

    ma_device_stop(&synth->device);

    printf("waiting for voice DP generator thread to finish...\n");
    synth->voice_dp_generator_running = false;

    for (int i = 0; i < MAX_VOICE_ACTIVE; i++)
    {
        pthread_join(voice_dp_generator_workers[i], NULL);
    }

    printf("Voice DP generator thread exiting\n");

    printf("Audio playback ended\n");
}

QSynthStat synth_get_stat(Synthesizer *synth)
{

    ma_device_state device_state = ma_device_get_state(&synth->device);

    return (QSynthStat){
        .frame_per_read = AUDIO_FRAME_PER_READ,
        .latency_ms = (int)synth->latency_ms,
        .max_voice = MAX_VOICE_ACTIVE,
        .recent_sample_size = RECENT_SAMPLE_SIZE,
        .recent_samples = synth->recent_samples,
        .sample_processed = synth->samples_played,
        .voice_active = synth->voice_active,
        .voice_buffer = VOICE_BUFFER_SIZE,
        .device_state = (QSynthDeviceState)device_state,
    };
}

void synth_print_stat(Synthesizer *synth)
{
    if (!synth)
    {
        printf("Synthesizer not initialized\n");
        return;
    }

    printf("=== QSynth Statistics ===\n");
    printf("Voices Active: %d\n", MAX_VOICE_ACTIVE);
    printf("Master Volume: %.2f\n", synth->master_volume);
    printf("Samples Played: %d\n", synth->samples_played);
    printf("Latency: %dms\n", (int)synth->latency_ms);
    printf("=========================\n");
}

int synth_play_note(Synthesizer *synth, InstrumentType instrument, NoteControlMode control_mode, NoteCfg *cfg)
{
    if (!synth)
    {
        set_error(QSYNTH_ERROR_UNINIT);
        return -1;
    }

    if (cfg->midi_note < 0 || cfg->midi_note > 127)
    {
        set_error(QSYNTH_ERROR_NOTECFG);
        return -1;
    }
    // find free voice
    for (int i = 0; i < MAX_VOICE_ACTIVE; i++)
    {
        if (!synth->voices[i].active)
        {
            double frequency = midi_to_frequency(cfg->midi_note);
            const InstrumentSignature *sig = instrument_get_signature(instrument);

            if (!sig)
            {
                printf("cannot find instrument (%d)\n", instrument);
                set_error(QSYNTH_ERROR_NOTECFG);
                return -1;
            }

            Voice *voice = &synth->voices[i];
            voice_init(voice);

            voice->duration_ms = cfg->duration_ms;
            voice->tone = &sig->tone;
            voice->frequency = frequency;
            voice->amplitude = cfg->amplitude;
            voice->pan = cfg->pan;
            voice->control_mode = control_mode;

            voice_start(voice, synth->device.sampleRate);

            printf("Started voice %d: note=%d, freq=%.2f, amp=%.2f\n", i, cfg->midi_note, frequency, cfg->amplitude);
            return i;
        }
    }

    printf("voice unavailable, used up %d voices\n", MAX_VOICE_ACTIVE);
    set_error(QSYNTH_ERROR_VOICE_UNAVAILABLE);
    return -1;
}

void synth_end_note(Synthesizer *synth, int voice_id)
{
    voice_end(&synth->voices[voice_id]);
}

int synth_set_master_volume(Synthesizer *synth, double volume)
{
    if (!synth)
    {
        set_error(QSYNTH_ERROR_UNINIT);
        return synth->master_volume;
    }

    if (volume < 0 || volume > 1)
    {
        printf("volume can only be set in range 0-1\n");
        set_error(QSYNTH_ERROR_NOTECFG);
        return synth->master_volume;
    }

    printf("set master volume to be: %f\n", volume);
    synth->master_volume = volume;
    return synth->master_volume;
}

// ERROR HANDLING FUNCTIONS
QSynthError synth_get_last_error()
{
    return g_last_error;
}

const char *synth_get_error_string(QSynthError error)
{
    switch (error)
    {
    case QSYNTH_ERROR_NONE:
        return "No error";
    case QSYNTH_ERROR_MEMALLOC:
        return "Failed to allocate memory";
    case QSYNTH_ERROR_NOTECFG:
        return "Wrong note configuration";
    case QSYNTH_ERROR_UNINIT:
        return "Synthesizer not initialized";
    case QSYNTH_ERROR_UNSUPPORT:
        return "Supported function";
    default:
        return "Unknown error";
    }
}
