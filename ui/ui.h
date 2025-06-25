#pragma once

#include <stdbool.h>

#include "qsynth.h"
#include "raylib.h"

// Constants
#define WINDOW_WIDTH 1190
#define WINDOW_HEIGHT 700
#define WAVEFORM_SAMPLES 512

// UI State
typedef struct
{
    Synthesizer *synth;
    QSynthStat stat;

    // Control state
    int selected_instrument;
    float amplitude;
    int octave;
    float pan;
    int active_voices[512];

    // Visualization
    float waveform_data[WAVEFORM_SAMPLES];

    // UI state
    float master_volume;
    bool show_help;
    bool show_credit;
    int visualStyleActive;
    int prevVisualStyleActive;

    // Pedal state
    PedalInfo pedals_info[PEDAL_COUNT]; 
    int selected_pedal_type;
    int dragging_pedal_idx;
    Vector2 drag_offset;
    bool is_dragging;
} UIState;

// Key mappings for piano keyboard
static const char piano_keys_lower[] = "QWERTYU"; // 7 white keys: C D E F G A B
static const char piano_keys_upper[] = "23567";   // 5 black keys: C# D# F# G# A#

extern bool prevKeyState[512];

// Instrument names for UI
static const char *instrument_names[] = {
    "Lead Square",
    "Warm Bass",
    "Ethereal Pad",
    "Metallic Pluck",
    "Wobble Bass",
    "Bell Lead",
    "Deep Drone",
};

// utils
char key_to_keyname(int key);
int keyname_to_key(char keyname);
int get_midi_note_from_key(char keyname, int octave);

// Components
void draw_controls(UIState *ui);
void draw_waveform_visualizer(UIState *ui);
void draw_stats_panel(UIState *ui);
void draw_piano_keyboard(UIState *ui);
void draw_status_bar(UIState *ui);
void draw_help_window(UIState *ui);
void draw_credit_window(UIState *ui);
void draw_pedal_panel(UIState *ui);

// Components update methods
void update_waveform_visualization(UIState *ui);
void update_visual_style(UIState *ui);