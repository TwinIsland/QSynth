#pragma once

typedef enum
{
    ADSR_IDLE,
    ADSR_ATTACK,
    ADSR_DECAY,
    ADSR_SUSTAIN,
    ADSR_RELEASE
} ADSRState;

// Volume
//   1.0 |
//       |    ∩               ← ATTACK: Rise to peak
//       |   /│
//       |  / │
//  0.5  |./  └───────────    ← SUSTAIN: Hold at sustain level
//       |                │
//       |                │   ← DECAY: Drop to sustain
//       |                │
//       |                └─── ← RELEASE: Fade to silence
//   0.0 |______________________
//       0
typedef struct
{
    double attack_ratio;
    double decay_ratio;
    double sustain_level;
    double release_ratio;
} EnvelopeCfg;

typedef struct
{
    EnvelopeCfg cfg;

    // parameters
    double attack_time;
    double decay_time;
    double sustain_level;
    double sustain_time;
    double release_time;

    // State
    ADSRState state;
    double phase;
    double current_level;
    int note_released;
} ADSREnvelope;

// Envelope functions
void adsr_init(ADSREnvelope *env, const EnvelopeCfg *cfg, int duration_ms);
void adsr_reset(ADSREnvelope *env);
void adsr_note_on(ADSREnvelope *env);
double adsr_process(ADSREnvelope *env, double delta_time);
int adsr_is_active(const ADSREnvelope *env);
