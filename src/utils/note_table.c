#include "note_table.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

static double note_frequencies[128];
static int table_initialized = 0;

static const char* note_names[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

void init_note_table(void) {
    if (table_initialized) return;
    
    for (int i = 0; i < 128; i++) {
        note_frequencies[i] = 440.0 * pow(2.0, (i - 69) / 12.0);
    }
    table_initialized = 1;
}

double midi_to_frequency(int midi_note) {
    if (!table_initialized) init_note_table();
    
    if (midi_note < 0 || midi_note > 127) return 0.0;
    return note_frequencies[midi_note];
}

int frequency_to_midi(double frequency) {
    if (frequency <= 0.0) return -1;
    
    // Convert using inverse formula: note = 69 + 12 * log2(freq/440)
    double note = 69.0 + 12.0 * log2(frequency / 440.0);
    return (int)(note + 0.5); // Round to nearest integer
}

const char* midi_to_note_name(int midi_note) {
    if (midi_note < 0 || midi_note > 127) return "Invalid";
    
    static char note_str[8];
    int octave = (midi_note / 12) - 1;
    int note_index = midi_note % 12;
    
    snprintf(note_str, sizeof(note_str), "%s%d", note_names[note_index], octave);
    return note_str;
}
