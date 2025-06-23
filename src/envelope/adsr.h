#pragma once

typedef enum
{
    ADSR_IDLE,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE
} ADSRState;

typedef struct
{
    double attack_time;   // Attack time in seconds
    double decay_time;    // Decay time in seconds
    double sustain_level; // Sustain level (0.0 to 1.0)
    double release_time;  // Release time in seconds
} EnvelopeCfg;

typedef struct
{
    ADSRState state;
    double current_level;
    double target_level;
    double rate;
    double sustain_level;
    double attack_time;
    double decay_time;
    double release_time;
    double time_in_state;
    int note_is_on;
} ADSREnvelope;

void adsr_init(ADSREnvelope *env, const EnvelopeCfg *cfg);
void adsr_reset(ADSREnvelope *env);
void adsr_note_on(ADSREnvelope *env);
void adsr_note_off(ADSREnvelope *env);
double adsr_process(ADSREnvelope *env, double delta_time);
int adsr_is_active(const ADSREnvelope *env);