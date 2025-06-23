#include "adsr.h"
#include <stdio.h>

void adsr_init(ADSREnvelope *env, const EnvelopeCfg *cfg)
{
    if (!env || !cfg)
        return;

    env->state = ADSR_IDLE;
    env->current_level = 0.0;
    env->target_level = 0.0;
    env->rate = 0.0;
    env->sustain_level = cfg->sustain_level;
    env->attack_time = cfg->attack_time;
    env->decay_time = cfg->decay_time;
    env->release_time = cfg->release_time;
    env->time_in_state = 0.0;
    env->note_is_on = 0;
}

void adsr_reset(ADSREnvelope *env)
{
    if (!env)
        return;

    env->state = ADSR_IDLE;
    env->current_level = 0.0;
    env->target_level = 0.0;
    env->rate = 0.0;
    env->time_in_state = 0.0;
    env->note_is_on = 0;
}

void adsr_note_on(ADSREnvelope *env)
{
    if (!env)
        return;

    env->note_is_on = 1;
    env->state = ADSR_ATTACK;
    env->target_level = 1.0;
    env->time_in_state = 0.0;

    // Calculate attack rate
    if (env->attack_time > 0.0)
    {
        env->rate = (env->target_level - env->current_level) / env->attack_time;
    }
    else
    {
        env->current_level = env->target_level;
        env->rate = 0.0;
    }
}

void adsr_note_off(ADSREnvelope *env)
{
    if (!env)
        return;

    env->note_is_on = 0;
    env->state = ADSR_RELEASE;
    env->target_level = 0.0;
    env->time_in_state = 0.0;

    // Calculate release rate
    if (env->release_time > 0.0)
    {
        env->rate = (env->target_level - env->current_level) / env->release_time;
    }
    else
    {
        env->current_level = env->target_level;
        env->rate = 0.0;
    }
}

double adsr_process(ADSREnvelope *env, double delta_time)
{
    if (!env)
        return 0.0;

    env->time_in_state += delta_time;

    switch (env->state)
    {
    case ADSR_IDLE:
        env->current_level = 0.0;
        break;

    case ADSR_ATTACK:
        if (env->attack_time <= 0.0)
        {
            // Instant attack
            env->current_level = 1.0;
            env->state = ADSR_DECAY;
            env->target_level = env->sustain_level;
            env->time_in_state = 0.0;

            if (env->decay_time > 0.0)
            {
                env->rate = (env->target_level - env->current_level) / env->decay_time;
            }
            else
            {
                env->current_level = env->target_level;
                env->rate = 0.0;
            }
        }
        else
        {
            // Linear attack
            env->current_level += env->rate * delta_time;

            if (env->current_level >= 1.0)
            {
                env->current_level = 1.0;
                env->state = ADSR_DECAY;
                env->target_level = env->sustain_level;
                env->time_in_state = 0.0;

                if (env->decay_time > 0.0)
                {
                    env->rate = (env->target_level - env->current_level) / env->decay_time;
                }
                else
                {
                    env->current_level = env->target_level;
                    env->rate = 0.0;
                }
            }
        }
        break;

    case ADSR_DECAY:
        if (env->decay_time <= 0.0)
        {
            // Instant decay
            env->current_level = env->sustain_level;
            env->state = ADSR_SUSTAIN;
            env->rate = 0.0;
        }
        else
        {
            // Linear decay
            env->current_level += env->rate * delta_time;

            if (env->current_level <= env->sustain_level)
            {
                env->current_level = env->sustain_level;
                env->state = ADSR_SUSTAIN;
                env->rate = 0.0;
            }
        }
        break;

    case ADSR_SUSTAIN:
        env->current_level = env->sustain_level;
        break;

    case ADSR_RELEASE:
        if (env->release_time <= 0.0)
        {
            // Instant release
            env->current_level = 0.0;
            env->state = ADSR_IDLE;
            env->rate = 0.0;
        }
        else
        {
            // Linear release
            env->current_level += env->rate * delta_time;

            if (env->current_level <= 0.0)
            {
                env->current_level = 0.0;
                env->state = ADSR_IDLE;
                env->rate = 0.0;
            }
        }
        break;
    }

    // Clamp output to valid range
    if (env->current_level < 0.0)
        env->current_level = 0.0;
    if (env->current_level > 1.0)
        env->current_level = 1.0;

    return env->current_level;
}

int adsr_is_active(const ADSREnvelope *env)
{
    if (!env) return 0;
    return (env->state != ADSR_IDLE);
}
