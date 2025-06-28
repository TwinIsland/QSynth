#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

#include "ui.h"

bool prevKeyState[512] = {false}; // raylib supports up to 512 keys

static void handle_key_press(UIState *ui, int key)
{

    char keyname = key_to_keyname(key);

    if (keyname == 'Z' && ui->octave > 1)
        ui->octave--;
    else if (keyname == 'X' && ui->octave < 7)
        ui->octave++;
    else if (key == KEY_F1)
        ui->show_help = !ui->show_help;
    else if (key == KEY_F2)
        ui->show_credit = !ui->show_credit;

    else
    {
        int midi_note = get_midi_note_from_key(keyname, ui->octave);
        if (midi_note < 0)
            return;

        // handle keyboard press
        NoteCfg note_cfg = {
            .midi_note = midi_note,
            .amplitude = ui->amplitude,
            .pan = ui->pan,
        };

        int voice_id = synth_play_note(ui->synth, ui->selected_instrument,
                                       NOTE_CONTROL_MANUAL, &note_cfg);
        ui->active_voices[key] = voice_id;
    }
}

static void handle_key_release(UIState *ui, int key)
{
    if (ui->active_voices[key] >= 0)
    {
        synth_end_note(ui->synth, ui->active_voices[key]);
        ui->active_voices[key] = -1;
    }
}

int main()
{
    UIState ui = {0};

    // Initialize synthesizer
    if (!synth_init(&ui.synth, 44100.0, 2))
    {
        printf("Failed to initialize synthesizer\n");
        return 1;
    }

    if (!synth_start(ui.synth))
    {
        printf("Failed to start synthesizer: %s\n",
               synth_get_error_string(synth_get_last_error()));
        synth_cleanup(ui.synth);
        return 1;
    }

    // Initialize UI state
    ui.selected_instrument = INST_LEAD_SQUARE;
    ui.amplitude = 0.8f;
    ui.octave = 4;
    ui.pan = 0.5f;
    ui.master_volume = 0.5f;
    ui.show_help = false;
    ui.show_credit = false;
    ui.stat = synth_get_stat(ui.synth);
    ui.selected_pedal_type = PEDAL_REVERB;
    ui.dragging_pedal_idx = -1;
    ui.is_dragging = false;
    ui.clicking_pedal_idx = -1;  // No pedal selected initially

    ui.visualStyleActive = 4;
    ui.prevVisualStyleActive = 0;

    // Initialize all pedal assets
    for (int i = 0; i < (int)PEDAL_COUNT; ++i)
    {
        ui.pedals_info[i] = synth_pedal_info((PedalType)i);
    }

    for (int i = 0; i < 256; i++)
    {
        ui.active_voices[i] = -1;
    }

    // Initialize Raylib
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "QSynth");
    SetTargetFPS(60);

    printf("QSynth UI started successfully!\n");

    // Main loop
    while (!WindowShouldClose())
    {
        // update new style
        update_visual_style(&ui);

        for (int key = 0; key < 512; key++)
        {
            bool currentKeyState = IsKeyDown(key);

            // Key press detection
            if (currentKeyState && !prevKeyState[key])
                handle_key_press(&ui, key);

            // Key release detection
            else if (!currentKeyState && prevKeyState[key])
                handle_key_release(&ui, key);

            prevKeyState[key] = currentKeyState;
        }

        update_waveform_visualization(&ui);

        // draw everything
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        draw_controls(&ui);
        draw_waveform_visualizer(&ui);
        draw_stats_panel(&ui);
        draw_piano_keyboard(&ui);
        draw_status_bar(&ui);
        draw_pedal_panel(&ui);
        draw_pedal_adj_panel(&ui);
        draw_help_window(&ui);
        draw_credit_window(&ui);

        EndDrawing();
    }

    // Cleanup
    CloseWindow();
    synth_stop(ui.synth);
    synth_cleanup(ui.synth);

    return 0;
}