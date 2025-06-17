#include <stdio.h>

#include "voice.h"

#include "../envelope/adsr.h"

void voice_reset(Voice *voice)
{
    voice->active = false;
    voice->duration = 0;
    voice->tone = NULL;
    voice->frequency = 0;
    voice->velocity = 0;
    voice->_sample_rate = 0;
    voice->amplitude = 0;

    for (int i = 0; i < MAX_TONE_LAYERS; ++i) {
        voice->phases[i] = 0;
    }
}

void voice_start(Voice *voice, double sample_rate)
{
    voice_reset(voice);
    adsr_init(&voice->lenvelope, &voice->tone->envelope_opt, voice->duration);
    biquad_init(&voice->lfilter, &voice->tone->filter_opt, sample_rate);
    
    voice->_sample_rate = sample_rate;
    voice->active = true;

    adsr_note_on(&voice->lenvelope);
    biquad_reset(&voice->lfilter);
}

double voice_step(Voice *voice, double delta_time)
{
    if (!voice->active)
        return 0.0;

    double sample_mixed = 0.0;
    for (int i = 0; i < MAX_TONE_LAYERS; i++)
    {
        if (voice->tone->layers[i].base_wave.type == WAVE_NONE)
        {
            break;
        }

        // mix layers
        sample_mixed += generate_waveform(voice->tone->layers[i].base_wave.type, voice->phases[i] + voice->tone->phase_diff[i]) * voice->tone->mix_levels[i];

        // detune
        voice->phases[i] += phase_increment(voice->frequency + voice->tone->detune[i], voice->_sample_rate);
        voice->phases[i] = wrap_phase(voice->phases[i]);
    }


    // apply filter
    if (voice->lfilter.cfg.filter_type != FILTER_NONE)
    {
        sample_mixed = biquad_process(&voice->lfilter, sample_mixed);
    }

    // apply envelope
    double envelope = adsr_process(&voice->lenvelope, delta_time);
    sample_mixed *= envelope * voice->amplitude;


    // check if voice should be deactivated
    if (!adsr_is_active(&voice->lenvelope))
    {
        voice->active = false;
    }

    return sample_mixed;
}