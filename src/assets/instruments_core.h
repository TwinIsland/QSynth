#pragma once

#include "../core/tone.h"

typedef struct {
    Tone tone;
    
    // Metadata
    const char* name;
    const char* category;
    const char* description;
} InstrumentSignature;


const InstrumentSignature* instrument_get_signature(InstrumentType type);
