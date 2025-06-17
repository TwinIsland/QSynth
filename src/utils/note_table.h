#pragma once

// Note frequency functions
void init_note_table(void);
double midi_to_frequency(int midi_note);
int frequency_to_midi(double frequency);
const char* midi_to_note_name(int midi_note);

