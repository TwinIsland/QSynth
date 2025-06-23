#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "voice.h"

#include "../envelope/adsr.h"

void voice_init(Voice *voice)
{
    voice->active = false;
    voice->duration_ms = 0;
    voice->tone = NULL;
    voice->frequency = 0;
    voice->velocity = 0;
    voice->_sample_rate = 0;
    voice->amplitude = 0;
    voice->cur_duration = 0;
    voice->voice_is_end = false;

    memset(voice->stream_buf, 0, sizeof(voice->stream_buf));
    memset(voice->phases, 0, sizeof(voice->phases));
}

void voice_start(Voice *voice, double sample_rate)
{
    adsr_init(&voice->envelope, &voice->tone->envelope_opt);
    biquad_init(&voice->filter, &voice->tone->filter_opt, sample_rate);
    voice->_sample_rate = sample_rate;

    stream_init(&voice->streamer, voice->stream_buf, VOICE_BUFFER_SIZE);

    adsr_note_on(&voice->envelope);
    biquad_reset(&voice->filter);

    voice->active = true; // MUST set active in the end of function
}

void voice_end(Voice *voice)
{
    // printf("end voice\n");
    voice->voice_is_end = true;
    adsr_note_off(&voice->envelope);
}

double voice_step(Voice *voice, double delta_time)
{
    if (!voice->active)
        return 0.0;

    // if note controlled by duration, check if need to end voice
    voice->cur_duration += delta_time;
    if (voice->control_mode == NOTE_CONTROL_DURATION && !voice->voice_is_end && voice->cur_duration * 1000 >= voice->duration_ms)
        voice_end(voice);

    double envelope = adsr_process(&voice->envelope, delta_time);

    double base_phase_increment = phase_increment(voice->frequency, voice->_sample_rate);
    double sample_mixed = 0.0;

    for (int i = 0; i < MAX_TONE_LAYERS; i++)
    {
        // convert semitones to frequency ratio
        double detuned_phase_inc = base_phase_increment *
                                   pow(2.0, voice->tone->detune[i] / 12.0);

        double layer_sample = generate_waveform(
            voice->tone->layers[i].type,
            voice->phases[i] + voice->tone->phase_diff[i]);

        sample_mixed += layer_sample * voice->tone->mix_levels[i];

        voice->phases[i] += detuned_phase_inc;
        voice->phases[i] = wrap_phase(voice->phases[i]);
    }

    // apply filter
    if (voice->filter.cfg.filter_type != FILTER_NONE)
    {
        sample_mixed = biquad_process(&voice->filter, sample_mixed);
    }

    // apply envelope
    sample_mixed *= envelope * voice->amplitude;

    // update voice status
    if (!adsr_is_active(&voice->envelope))
    {
        voice->active = false;
    }

    return sample_mixed;
}