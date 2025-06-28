#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PEDAL_MAX_PARAMS 12
#define PEDALCHAIN_MAX_PEDAL 6

typedef enum
{
    PEDAL_REVERB = 0,
    PEDAL_DISTORTION,
    PEDAL_PHASER,

    PEDAL_COUNT // total number of instruments
} PedalType;

typedef struct
{
    char name[64];        // Parameter name (e.g., "Gain", "Frequency")
    double min_value;     // Minimum parameter value
    double max_value;     // Maximum parameter value
    double default_value; // Default parameter value
    double current_value;
    char unit[16];        // Unit string (e.g., "dB", "Hz", "%")
} PedalParam;

typedef struct
{
    char name[64];                       // Pedal name
    char description[256];               // Pedal description
    int param_count;                     // Number of parameters
    PedalParam params[PEDAL_MAX_PARAMS]; // Parameter array
} PedalInfo;
