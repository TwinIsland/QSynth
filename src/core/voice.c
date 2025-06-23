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

    memset(voice->stream_buf, 0, sizeof(voice->stream_buf));

    for (int i = 0; i < MAX_TONE_LAYERS; ++i)
    {
        voice->phases[i] = 0.0;
    }
}

void voice_start(Voice *voice, double sample_rate)
{
    adsr_init(&voice->lenvelope, &voice->tone->envelope_opt, voice->duration_ms);
    biquad_init(&voice->lfilter, &voice->tone->filter_opt, sample_rate);
    voice->_sample_rate = sample_rate;
    
    stream_init(&voice->streamer, voice->stream_buf, VOICE_BUFFER_SIZE);

    adsr_note_on(&voice->lenvelope);
    biquad_reset(&voice->lfilter);

    voice->active = true;   // MUST set active in the end of function
}

double voice_step(Voice *voice, double delta_time)
{
    if (!voice->active)
        return 0.0;

    // prioritize envelope calculating can optimize processing speed
    double envelope = adsr_process(&voice->lenvelope, delta_time);
    if (envelope <= 0.0001)
    {
        if (!adsr_is_active(&voice->lenvelope))
        {
            voice->active = false;
        }
        return 0.0;
    }

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
    if (voice->lfilter.cfg.filter_type != FILTER_NONE)
    {
        sample_mixed = biquad_process(&voice->lfilter, sample_mixed);
    }

    // apply envelope
    sample_mixed *= envelope * voice->amplitude;

    // update voice status
    if (!adsr_is_active(&voice->lenvelope))
    {
        voice->active = false;
    }

    return sample_mixed;
}