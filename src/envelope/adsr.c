#include "adsr.h"
#include <math.h>
#include <stdio.h>

void adsr_init(ADSREnvelope *env, const EnvelopeCfg *cfg, int duration_ms)
{
    adsr_reset(env);

    env->attack_time = duration_ms * cfg->attack_ratio;
    env->decay_time = duration_ms * cfg->decay_ratio;
    env->release_time = duration_ms * cfg->release_ratio;
    env->sustain_time = duration_ms - (env->attack_time + env->decay_time + env->release_time);

    env->sustain_level = cfg->sustain_level;
}

void adsr_reset(ADSREnvelope *env)
{
    env->state = ADSR_IDLE;
    env->phase = 0.0;
    env->current_level = 0.0;
    env->note_released = 0;
}

void adsr_note_on(ADSREnvelope *env)
{
    env->state = ADSR_ATTACK;
    env->phase = 0.0;
    env->note_released = 0;
}

double adsr_process(ADSREnvelope *env, double delta_time)
{
    if (env->state == ADSR_IDLE)
    {
        return 0.0;
    }

    env->phase += delta_time * 1000.0; // Convert to milliseconds

    switch (env->state)
    {
    case ADSR_ATTACK:
        if (env->attack_time > 0 && env->phase < env->attack_time)
        {
            env->current_level = env->phase / env->attack_time;
        }
        else
        {
            env->current_level = 1.0;
            env->state = ADSR_DECAY;
            env->phase = 0.0;
        }
        break;

    case ADSR_DECAY:
        if (env->decay_time > 0 && env->phase < env->decay_time)
        {
            double decay_progress = env->phase / env->decay_time;
            env->current_level = 1.0 - decay_progress * (1.0 - env->sustain_level);
        }
        else
        {
            env->current_level = env->sustain_level;
            env->state = ADSR_SUSTAIN;
            env->phase = 0.0;
        }
        break;

    case ADSR_SUSTAIN:
        env->current_level = env->sustain_level;
        if (env->sustain_time > 0 && env->phase >= env->sustain_time)
        {
            env->state = ADSR_RELEASE;
            env->phase = 0.0;
        }
        else if (env->sustain_time <= 0)
        {
            env->state = ADSR_RELEASE;
            env->phase = 0.0;
        }
        break;

    case ADSR_RELEASE:
        if (env->release_time > 0 && env->phase < env->release_time)
        {
            double release_progress = env->phase / env->release_time;
            env->current_level = env->sustain_level * (1.0 - release_progress);
        }
        else
        {
            env->current_level = 0.0;
            env->state = ADSR_IDLE;
        }
        break;

    case ADSR_IDLE:
    default:
        env->current_level = 0.0;
        break;
    }

    // clamp to prevent negative values
    if (env->current_level < 0.001 && env->state == ADSR_RELEASE)     
    {         
        env->current_level = 0.0;         
        env->state = ADSR_IDLE;     
    }      


    return env->current_level;
}

int adsr_is_active(const ADSREnvelope *env)
{
    return env->state != ADSR_IDLE;
}
