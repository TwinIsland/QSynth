#include <stdio.h>
#include <string.h>

#include "envelop_setting.h"
#include "instruments_core.h"

static const InstrumentSignature instrument_signatures[INST_COUNT] = {
    // 0. Lead Square
    [INST_LEAD_SQUARE] = {
        .tone = {
            .layers = {
                {.base_wave = WAVE_SQUARE},
                {.base_wave = WAVE_SAWTOOTH},
                {.base_wave = WAVE_SQUARE},
                {.base_wave = WAVE_TRIANGLE},
            },
            .detune = {0.0, 0.03, -12.0, 12.0},
            .mix_levels = {0.5, 0.3, 0.15, 0.05},
            .phase_diff = {0, 180, 0, 90},
            .filter_opt = {.cutoff = 2500, .filter_type = FILTER_BANDPASS, .resonance = 0.6},
            .envelope_opt = ENVELOPE_OPT_LEAD,
        },
        .name = "Lead Square",
        .category = "Lead",
        .description = "Rich square wave lead with sub-bass, detuning, and harmonic layers"},

    // 1. WARM ANALOG BASS
    [INST_WARM_BASS] = {.tone = {
                            .layers = {
                                {.base_wave = WAVE_SAWTOOTH},
                                {.base_wave = WAVE_SQUARE},
                                {.base_wave = WAVE_SINE},
                            },
                            .detune = {0.0, -12.0, -24.0},
                            .mix_levels = {0.6, 0.25, 0.15},
                            .phase_diff = {0, 0, 0},
                            .filter_opt = {.cutoff = 800, .filter_type = FILTER_LOWPASS, .resonance = 0.7},
                            .envelope_opt = ENVELOPE_OPT_BASS,
                        },
                        .name = "Warm Bass",
                        .category = "Bass",
                        .description = "Rich analog-style bass with sub harmonics"},

    // 2. ETHEREAL PAD
    [INST_ETHEREAL_PAD] = {.tone = {
                               .layers = {
                                   {.base_wave = WAVE_SINE},
                                   {.base_wave = WAVE_TRIANGLE},
                                   {.base_wave = WAVE_SINE},
                                   {.base_wave = WAVE_TRIANGLE},
                               },
                               .detune = {0.0, 7.0, 12.0, 19.0},
                               .mix_levels = {0.4, 0.3, 0.2, 0.1},
                               .phase_diff = {0, 90, 180, 270},
                               .filter_opt = {.cutoff = 2000, .filter_type = FILTER_LOWPASS, .resonance = 0.1},
                               .envelope_opt = ENVELOPE_OPT_PAD,
                           },
                           .name = "Ethereal Pad",
                           .category = "Pad",
                           .description = "Dreamy harmonic pad with gentle attack"},

    // 3. METALLIC PLUCK
    [INST_METALLIC_PLUCK] = {.tone = {
                                 .layers = {
                                     {.base_wave = WAVE_SAWTOOTH},
                                     {.base_wave = WAVE_SQUARE},
                                     {.base_wave = WAVE_TRIANGLE},
                                 },
                                 .detune = {0.0, 0.03, -0.03},
                                 .mix_levels = {0.5, 0.3, 0.2},
                                 .phase_diff = {0, 0, 0},
                                 .filter_opt = {.cutoff = 3000, .filter_type = FILTER_BANDPASS, .resonance = 0.8},
                                 .envelope_opt = ENVELOPE_OPT_PLUCK,
                             },
                             .name = "Metallic Pluck",
                             .category = "Pluck",
                             .description = "Sharp attack pluck with metallic resonance"},

    // 4. WOBBLE BASS
    [INST_WOBBLE_BASS] = {.tone = {
                              .layers = {
                                  {.base_wave = WAVE_SAWTOOTH},
                                  {.base_wave = WAVE_SQUARE},
                              },
                              .detune = {0.0, 0.07},
                              .mix_levels = {0.7, 0.3},
                              .phase_diff = {0, 180},
                              .filter_opt = {.cutoff = 400, .filter_type = FILTER_LOWPASS, .resonance = 0.9},
                              .envelope_opt = ENVELOPE_OPT_BASS,
                          },
                          .name = "Wobble Bass",
                          .category = "Bass",
                          .description = "High resonance bass perfect for filter modulation"},

    // 5. BELL LEAD
    [INST_BELL_LEAD] = {.tone = {.layers = {
                                     {.base_wave = WAVE_SINE},
                                     {.base_wave = WAVE_SINE},
                                     {.base_wave = WAVE_SINE},
                                     {.base_wave = WAVE_TRIANGLE},
                                 },
                                 .detune = {0.0, 12.0, 19.0, 24.0},
                                 .mix_levels = {0.6, 0.3, 0.2, 0.1},
                                 .phase_diff = {0, 0, 0, 0},
                                 .filter_opt = {.cutoff = 1500, .filter_type = FILTER_BANDPASS, .resonance = 0.3},
                                 .envelope_opt = ENVELOPE_OPT_LEAD},
                        .name = "Bell Lead",
                        .category = "Lead",
                        .description = "Bright bell-like lead with harmonic series"},

    // 6. DEEP DRONE
    [INST_DEEP_DRONE] = {.tone = {
                             .layers = {
                                 {.base_wave = WAVE_SINE},
                                 {.base_wave = WAVE_TRIANGLE},
                                 {.base_wave = WAVE_SAWTOOTH},
                             },
                             .detune = {-12.0, -24.0, -12.02},
                             .mix_levels = {0.4, 0.4, 0.2},
                             .phase_diff = {0, 120, 240},
                             .filter_opt = {.cutoff = 200, .filter_type = FILTER_LOWPASS, .resonance = 0.5},
                             .envelope_opt = ENVELOPE_OPT_BASS,
                         },
                         .name = "Deep Drone",
                         .category = "Drone",
                         .description = "Ultra-low frequency drone with subtle beating"},
};

const InstrumentSignature *instrument_get_signature(InstrumentType type)
{
    if (type < 0 || type >= INST_COUNT)
        return NULL;

    return &instrument_signatures[type];
}

InstrumentInfo instrument_get_info(InstrumentType type)
{
    const InstrumentSignature *sig = instrument_get_signature(type);

    return (InstrumentInfo){
        .name = sig->name,
        .category = sig->category,
        .description = sig->description,
    };
}

// print all available instruments with details
void instrument_print_all(void)
{
    printf("=== QSYNTH INSTRUMENT LIBRARY ===\n");
    printf("Total Instruments: %d\n\n", INST_COUNT);

    for (int i = 0; i < INST_COUNT; i++)
    {
        const InstrumentSignature *inst = instrument_get_signature((InstrumentType)i);
        if (inst != NULL)
        {
            printf("[%02d] %s\n", i, inst->name);
            printf("     Category: %s\n", inst->category);
            printf("     Description: %s\n", inst->description);

            // Print technical details
            printf("     Layers: ");
            int layer_count = 0;
            for (int j = 0; j < MAX_TONE_LAYERS; j++)
            {
                if (inst->tone.layers[j].base_wave.type != WAVE_NONE)
                {
                    layer_count++;
                    const char *wave_name = "Unknown";
                    switch (inst->tone.layers[j].base_wave.type)
                    {
                    case WAVE_SINE:
                        wave_name = "Sine";
                        break;
                    case WAVE_SQUARE:
                        wave_name = "Square";
                        break;
                    case WAVE_SAWTOOTH:
                        wave_name = "Saw";
                        break;
                    case WAVE_TRIANGLE:
                        wave_name = "Triangle";
                        break;
                    default:
                        wave_name = "Unknown";
                        break;
                    }
                    printf("%s", wave_name);
                    if (j < MAX_TONE_LAYERS - 1 &&
                        inst->tone.layers[j + 1].base_wave.type != WAVE_NONE)
                        printf(", ");
                }
            }
            printf(" (%d layers)\n", layer_count);

            // Print envelope info
            printf("     Envelope: A:%.1f%% D:%.1f%% R:%.1f%% S:%.1f\n",
                   inst->tone.envelope_opt.attack_ratio * 100,
                   inst->tone.envelope_opt.decay_ratio * 100,
                   inst->tone.envelope_opt.release_ratio * 100,
                   inst->tone.envelope_opt.sustain_level * 100);

            printf("\n");
        }
    }
}

// print instruments by category
void instrument_print_by_category(const char *category)
{
    printf("=== %s INSTRUMENTS ===\n", category);

    int found = 0;
    for (int i = 0; i < INST_COUNT; i++)
    {
        const InstrumentSignature *inst = instrument_get_signature((InstrumentType)i);
        if (inst != NULL && strcmp(inst->category, category) == 0)
        {
            printf("[%02d] %s - %s\n", i, inst->name, inst->description);
            found++;
        }
    }

    if (found == 0)
    {
        printf("No instruments found in category '%s'\n", category);
    }
    else
    {
        printf("Found %d instruments in category '%s'\n", found, category);
    }
    printf("\n");
}

// get all unique categories
void instrument_print_categories()
{
    printf("=== INSTRUMENT CATEGORIES ===\n");

    const char *categories[INST_COUNT];
    int category_count = 0;

    for (int i = 0; i < INST_COUNT; i++)
    {
        const InstrumentSignature *inst = instrument_get_signature((InstrumentType)i);
        if (inst != NULL)
        {
            int exists = 0;
            for (int j = 0; j < category_count; j++)
            {
                if (strcmp(categories[j], inst->category) == 0)
                {
                    exists = 1;
                    break;
                }
            }

            if (!exists)
            {
                categories[category_count++] = inst->category;
            }
        }
    }

    for (int i = 0; i < category_count; i++)
    {
        int count = 0;
        for (int j = 0; j < INST_COUNT; j++)
        {
            const InstrumentSignature *inst = instrument_get_signature((InstrumentType)j);
            if (inst != NULL && strcmp(inst->category, categories[i]) == 0)
            {
                count++;
            }
        }
        printf("- %s (%d instruments)\n", categories[i], count);
    }
    printf("\n");
}

// find instrument by name (case-insensitive partial match)
InstrumentType instrument_find_by_name(const char *name)
{
    for (int i = 0; i < INST_COUNT; i++)
    {
        const InstrumentSignature *inst = instrument_get_signature((InstrumentType)i);
        if (inst != NULL)
        {
            // simple case-insensitive substring search
            if (strstr(inst->name, name) != NULL ||
                strstr(inst->description, name) != NULL)
            {
                return (InstrumentType)i;
            }
        }
    }
    return -1; // Not found
}

// get random instrument from a category
InstrumentType instrument_get_random_from_category(const char *category)
{
    InstrumentType candidates[INST_COUNT];
    int candidate_count = 0;

    for (int i = 0; i < INST_COUNT; i++)
    {
        const InstrumentSignature *inst = instrument_get_signature((InstrumentType)i);
        if (inst != NULL && strcmp(inst->category, category) == 0)
        {
            candidates[candidate_count++] = (InstrumentType)i;
        }
    }

    if (candidate_count == 0)
        return -1;

    return candidates[0]; // For now, just return first
}

// print instrument technical details
void instrument_print_details(InstrumentType type)
{
    const InstrumentSignature *inst = instrument_get_signature(type);
    if (inst == NULL)
    {
        printf("Invalid instrument type: %d\n", type);
        return;
    }

    printf("=== %s TECHNICAL DETAILS ===\n", inst->name);
    printf("Category: %s\n", inst->category);
    printf("Description: %s\n\n", inst->description);

    printf("OSCILLATOR LAYERS:\n");
    for (int i = 0; i < MAX_TONE_LAYERS; i++)
    {
        if (inst->tone.layers[i].base_wave.type != WAVE_NONE)
        {
            const char *wave_name = "Unknown";
            switch (inst->tone.layers[i].base_wave.type)
            {
            case WAVE_NONE:
                wave_name = "NONE";
                break;
            case WAVE_SINE:
                wave_name = "Sine Wave";
                break;
            case WAVE_NOISE:
                wave_name = "Noise Wave";
                break;
            case WAVE_SQUARE:
                wave_name = "Square Wave";
                break;
            case WAVE_SAWTOOTH:
                wave_name = "Sawtooth Wave";
                break;
            case WAVE_TRIANGLE:
                wave_name = "Triangle Wave";
                break;
            }

            printf("  Layer %d: %s\n", i + 1, wave_name);
            printf("    Detune: %.2f semitones\n", inst->tone.detune[i]);
            printf("    Mix Level: %.1f%%\n", inst->tone.mix_levels[i] * 100);
            printf("    Phase Offset: %.0f degrees\n", inst->tone.phase_diff[i]);
        }
    }

    printf("\nFILTER:\n");
    if (inst->tone.filter_opt.filter_type != FILTER_NONE)
    {
        const char *filter_name = "Unknown";
        switch (inst->tone.filter_opt.filter_type)
        {
        case FILTER_NONE:
            filter_name = "None";
            break;
        case FILTER_LOWPASS:
            filter_name = "Low Pass";
            break;
        case FILTER_HIGHPASS:
            filter_name = "High Pass";
            break;
        case FILTER_BANDPASS:
            filter_name = "Band Pass";
            break;
        case FILTER_NOTCH:
            filter_name = "Notch";
            break;
        }
        printf("  Type: %s\n", filter_name);
        printf("  Cutoff: %.0f Hz\n", inst->tone.filter_opt.cutoff);
        printf("  Resonance: %.2f\n", inst->tone.filter_opt.resonance);
    }
    else
    {
        printf("  No filter applied\n");
    }

    printf("\nENVELOPE:\n");
    printf("  Attack: %.1f%%\n", inst->tone.envelope_opt.attack_ratio * 100);
    printf("  Decay: %.1f%%\n", inst->tone.envelope_opt.decay_ratio * 100);
    printf("  Sustain Level: %.1f%%\n", inst->tone.envelope_opt.sustain_level * 100);
    printf("  Release: %.1f%%\n", inst->tone.envelope_opt.release_ratio * 100);

    double sustain_ratio = 1.0 - (inst->tone.envelope_opt.attack_ratio +
                                  inst->tone.envelope_opt.decay_ratio +
                                  inst->tone.envelope_opt.release_ratio);
    printf("  Sustain Duration: %.1f%%\n", sustain_ratio * 100);
    printf("\n");
}