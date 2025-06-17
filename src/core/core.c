#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qsynth.h"
#include "core.h"
#include "instruments.h"

#include "../assets/instruments_core.h"
#include "../utils/note_table.h"

static QSynthError g_last_error = QSYNTH_ERROR_NONE;

static int set_error(AudioError error)
{
    g_last_error = error;
    return 0;
}

void _audio_callback(void *user_data, int16_t *output_buffer, int frame_count)
{
    // printf("called\n");
    Synthesizer *synth = (Synthesizer *)user_data;

    if (!synth) {
        return;
    }

    int current_buffer_idx = synth->buffer_state & BUF_AVAILABLE_IDX_MASK;

    memcpy(output_buffer, synth->audioBuffers[current_buffer_idx],
           frame_count * 2 * sizeof(int16_t));

    synth->buffer_state ^= BUF_AVAILABLE_IDX_MASK;

    _process_voice_buffer(synth);

    synth->samples_played += frame_count;
}

void _process_voice_buffer(Synthesizer *synth)
{
    if (!synth)
        return;

    int buffer_to_fill = (synth->buffer_state & BUF_AVAILABLE_IDX_MASK) ^ 1;

    double delta_time = 1.0 / synth->device.config.sample_rate;
    int frame_count = synth->device.config.buffer_size;

    // int active_voices = 0;

    for (int i = 0; i < frame_count; i++)
    {
        double left_mix = 0.0, right_mix = 0.0;

        // process all voices for this sample
        for (int v = 0; v < MAX_VOICE_ACTIVE; v++)
        {
            Voice *voice = &synth->voices[v];
            if (!voice->active)
                continue;

            // if (i == 0)
            //     active_voices++; 

            double sample = voice_step(voice, delta_time);
            // apply panning
            double left_gain = 1.0 - voice->pan;
            double right_gain = voice->pan;

            left_mix += sample * left_gain;
            right_mix += sample * right_gain;
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

        synth->audioBuffers[buffer_to_fill][i * 2] = (int16_t)(left_mix * 32767);
        synth->audioBuffers[buffer_to_fill][i * 2 + 1] = (int16_t)(right_mix * 32767);
    }

    // if (active_voices > 0)
    // {
    //     printf("Processed buffer %d with %d active voices\n", buffer_to_fill, active_voices);
    // }
}

bool synth_init(Synthesizer **synth_ptr, double sample_rate, int channels)
{
    if (channels != 2)
    {
        printf("QSynth only support 2 channel audio currently\n");
        set_error(QSYNTH_ERROR_UNSUPPORT);
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
    AudioConfig config = {
        .sample_rate = sample_rate,
        .channels = channels,
        .buffer_size = AUDIO_BUFFER_SIZE,
        .user_data = synth,
        .callback = _audio_callback,
    };

    // init audio device
    synth->device = (AudioDevice){0};
    
    if (!audio_device_init(&synth->device, &config))
    {
        printf("Failed to init device: %s\n",
               audio_device_get_error_string(audio_device_get_last_error()));
        set_error(QSYNTH_ERROR_DEVICE);
        synth_cleanup(synth);
        return false;
    }

    synth->buffer_state = 0;

    // allocate audio buffers
    for (int i = 0; i < 2; i++)
    {
        synth->audioBuffers[i] = (short *)calloc(AUDIO_BUFFER_SIZE * channels, sizeof(short));
        if (!synth->audioBuffers[i])
        {
            set_error(QSYNTH_ERROR_MEMALLOC);
            synth_cleanup(synth);
            return false;
        }
    }

    // init voice
    for (int i = 0; i < MAX_VOICE_ACTIVE; i++)
    {
        voice_reset(&synth->voices[i]);
    }

    // init global settings
    synth->master_volume = 0.5;

    // init state
    synth->device.is_playing = false;
    synth->samples_played = 0;

    printf("QSynth initialized: %.1fHz, %d channels\n", sample_rate, channels);
    return true;
}

void synth_cleanup(Synthesizer *synth)
{
    if (!synth)
        return;

    audio_device_cleanup(&synth->device);

    // free audio buffers
    for (int i = 0; i < 2; i++)
    {
        if (synth->audioBuffers[i])
        {
            free(synth->audioBuffers[i]);
        }
    }

    free(synth);
    printf("QSynth cleaned up\n");
}

bool synth_start(Synthesizer *synth)
{
    if (!audio_device_start(&synth->device))
    {
        printf("Failed to start audio: %s\n",
               audio_device_get_error_string(audio_device_get_last_error()));
        audio_device_cleanup(&synth->device);
        return false;
    }

    printf("Audio playback started\n");
    return true;
}

void synth_stop(Synthesizer *synth)
{
    audio_device_stop(&synth->device);
    synth->device.is_playing = false;

    printf("Audio playback ended\n");
}

int synth_play_note(Synthesizer *synth, InstrumentType instrument, NoteCfg *cfg)
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
            voice_reset(voice);

            voice->duration_ms = cfg->duration_ms;
            voice->tone = &sig->tone;
            voice->velocity = cfg->velocity;
            voice->frequency = frequency;
            voice->amplitude = cfg->amplitude;
            voice->pan = cfg->pan;

            voice_start(voice, synth->device.config.sample_rate);

            printf("Started voice %d: freq=%.2f, amp=%.2f\n", i, frequency, cfg->amplitude);
            return i;
        }
    }

    printf("voice unavailable, used up %d voices\n", MAX_VOICE_ACTIVE);
    set_error(QSYNTH_ERROR_VOICE_UNAVAILABLE);
    return -1;
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
        printf("volume can only be set in range 0-1");
        set_error(QSYNTH_ERROR_NOTECFG);
        return synth->master_volume;
    }

    printf("set master volume to be: %d", volume);
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
