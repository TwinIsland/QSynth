#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PEDAL_MAX_PARAMS 12

typedef enum
{
    PEDAL_REVERB = 0,
    PEDAL_DISTORTION,
    PEDAL_PHASER,

    PEDAL_COUNT // total number of instruments
} PedalType;

typedef struct
{
    const char *name;
    const char *category;
    const char *description;

    double param_default[PEDAL_MAX_PARAMS];
    char *param_description[PEDAL_MAX_PARAMS];
    int param_n;
} PedalInfo;