#include "adsr.h"
#include <math.h>
#include <stdio.h>

void adsr_init(ADSREnvelope *env, const EnvelopeCfg *cfg, double duration)
{
    env->attack_time = duration * cfg->attack_ratio;

    env->decay_time = duration * cfg->decay_ratio;
    env->sustain_level = cfg->sustain_level;
    env->release_time = duration * cfg->release_ratio;

    env->sustain_time = duration - (env->attack_time + env->decay_time + env->release_time);

    adsr_reset(env);
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

void adsr_note_off(ADSREnvelope *env)
{
    env->note_released = 1;
    env->state = ADSR_RELEASE;
    env->phase = 0.0;
}

double adsr_process(ADSREnvelope *env, double delta_time)
{
    if (env->state == ADSR_IDLE)
    {
        return 0.0;
    }

    env->phase += delta_time;

    switch (env->state)
    {
    case ADSR_ATTACK:
        if (env->phase < env->attack_time)
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
        if (env->phase < env->decay_time)
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
        if (env->phase >= env->sustain_time)
        {
            env->state = ADSR_RELEASE;
            env->phase = 0.0;
        } 
        break;

    case ADSR_RELEASE:
        if (env->phase < env->release_time)
        {
            env->current_level = env->sustain_level *
                                 exp(-5.0 * env->phase / env->sustain_level);
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

    if (env->current_level < 0.001)
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
