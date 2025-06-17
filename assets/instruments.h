#pragma once

typedef enum {
    INST_LEAD_SQUARE = 0,
    INST_WARM_BASS,
    INST_ETHEREAL_PAD,
    INST_METALLIC_PLUCK,
    INST_WOBBLE_BASS,
    INST_BELL_LEAD,
    INST_DEEP_DRONE,

    INST_COUNT  // total number of instruments
} InstrumentType;

typedef struct {
    const char* name;
    const char* category;
    const char* description;    
} InstrumentInfo;

InstrumentInfo instrument_get_info(InstrumentType type);
void instrument_print_all();
void instrument_print_by_category(const char* category);
void instrument_print_categories();
InstrumentType instrument_find_by_name(const char* name);
InstrumentType instrument_get_random_from_category(const char* category);
void instrument_print_details(InstrumentType type);