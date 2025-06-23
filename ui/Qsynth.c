#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "qsynth.h"
#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "style/style_cyber.h"

// Constants
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700
#define WAVEFORM_SAMPLES 512

static bool prevKeyState[512] = {false}; // raylib supports up to 512 keys

// UI State
typedef struct
{
    Synthesizer *synth;
    QSynthStat stat;

    // Control state
    int selected_instrument;
    float amplitude;
    int octave;
    // bool keys_pressed[512]; // raylib supports up to 512 keys
    int active_voices[512];

    // Visualization
    float waveform_data[WAVEFORM_SAMPLES];

    // UI state
    float master_volume;
    bool show_help;

} UIState;

// Instrument names for UI
static const char *instrument_names[] = {
    "Lead Square",
    "Warm Bass",
    "Ethereal Pad",
    "Metallic Pluck",
    "Wobble Bass",
    "Bell Lead",
    "Deep Drone"};

// Key mappings for piano keyboard
static const char piano_keys_lower[] = "QWERTYU"; // 7 white keys: C D E F G A B
static const char piano_keys_upper[] = "23567";   // 5 black keys: C# D# F# G# A#

// Convert character to raylib key code
static int keyname_to_key(char keyname)
{
    // Convert to uppercase for consistency
    if (keyname >= 'a' && keyname <= 'z')
    {
        keyname = keyname - 'a' + 'A';
    }

    // Letters A-Z
    if (keyname >= 'A' && keyname <= 'Z')
    {
        return KEY_A + (keyname - 'A');
    }

    // Numbers 0-9
    if (keyname >= '0' && keyname <= '9')
    {
        return KEY_ZERO + (keyname - '0');
    }

    // Special characters and keys
    switch (keyname)
    {
    case ' ':
        return KEY_SPACE;
    case '\n':
    case '\r':
        return KEY_ENTER;
    case '\t':
        return KEY_TAB;
    case '\b':
        return KEY_BACKSPACE;
    case 27:
        return KEY_ESCAPE; // ESC character
    case '.':
        return KEY_PERIOD;
    case ',':
        return KEY_COMMA;
    case ';':
        return KEY_SEMICOLON;
    case '\'':
        return KEY_APOSTROPHE;
    case '/':
        return KEY_SLASH;
    case '\\':
        return KEY_BACKSLASH;
    case '[':
        return KEY_LEFT_BRACKET;
    case ']':
        return KEY_RIGHT_BRACKET;
    case '=':
        return KEY_EQUAL;
    case '-':
        return KEY_MINUS;
    case '`':
        return KEY_GRAVE;
    default:
        return -1; // Unknown key
    }
}

// Convert raylib key code to character (returns 0 if not convertible)
static char key_to_keyname(int key)
{
    // Letters A-Z
    if (key >= KEY_A && key <= KEY_Z)
    {
        return 'A' + (key - KEY_A);
    }

    // Numbers 0-9
    if (key >= KEY_ZERO && key <= KEY_NINE)
    {
        return '0' + (key - KEY_ZERO);
    }

    // Special keys
    switch (key)
    {
    case KEY_SPACE:
        return ' ';
    case KEY_ENTER:
        return '\n';
    case KEY_TAB:
        return '\t';
    case KEY_BACKSPACE:
        return '\b';
    case KEY_ESCAPE:
        return 27; // ESC character
    case KEY_PERIOD:
        return '.';
    case KEY_COMMA:
        return ',';
    case KEY_SEMICOLON:
        return ';';
    case KEY_APOSTROPHE:
        return '\'';
    case KEY_SLASH:
        return '/';
    case KEY_BACKSLASH:
        return '\\';
    case KEY_LEFT_BRACKET:
        return '[';
    case KEY_RIGHT_BRACKET:
        return ']';
    case KEY_EQUAL:
        return '=';
    case KEY_MINUS:
        return '-';
    case KEY_GRAVE:
        return '`';
    default:
        return 0; // Non-printable or unknown key
    }
}

static int get_midi_note_from_key(char keyname, int octave)
{
    int base_note = 60 + (octave - 4) * 12;

    // Check lower row (white keys) - C D E F G A B
    for (int i = 0; i < 7; i++)
    {
        if (keyname == piano_keys_lower[i])
        {
            int note_offsets[] = {0, 2, 4, 5, 7, 9, 11}; // C D E F G A B
            return base_note + note_offsets[i];
        }
    }

    // Check upper row (black keys) - C# D# F# G# A#
    for (int i = 0; i < 5; i++)
    {
        if (keyname == piano_keys_upper[i])
        {
            int note_offsets[] = {1, 3, 6, 8, 10}; // C# D# F# G# A#
            return base_note + note_offsets[i];
        }
    }

    return -1;
}

static void update_waveform_visualization(UIState *ui)
{
    const int16_t *samples = ui->stat.recent_samples;
    int sample_size = ui->stat.recent_sample_size;

    int step = (sample_size / 2) / WAVEFORM_SAMPLES;
    if (step < 1)
        step = 1;

    for (int i = 0; i < WAVEFORM_SAMPLES; i++)
    {
        int sample_idx = (i * step * 2) % sample_size;

        if (sample_idx < sample_size - 1)
        {
            float left = samples[sample_idx] / 32767.0f;
            float right = samples[sample_idx + 1] / 32767.0f;
            ui->waveform_data[i] = (left + right) * 0.5f;
        }
        else
        {
            ui->waveform_data[i] = 0.0f;
        }
    }
}

static void handle_key_press(UIState *ui, int key)
{

    char keyname = key_to_keyname(key);

    if (keyname == 'Z' && ui->octave > 1)
        ui->octave--;
    else if (keyname == 'X' && ui->octave < 7)
        ui->octave++;
    else if (key == KEY_F1)
        ui->show_help = !ui->show_help;
    else
    {
        int midi_note = get_midi_note_from_key(keyname, ui->octave);
        if (midi_note < 0)
            return;

        // handle keyboard press
        NoteCfg note_cfg = {
            .midi_note = midi_note,
            .amplitude = ui->amplitude,
            .pan = 0.5,
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

static void draw_piano_keyboard(UIState *ui)
{
    Rectangle piano_area = {50, 500, 900, 150};

    GuiGroupBox(piano_area, "Piano Keyboard");

    Rectangle piano_content = {piano_area.x + 10, piano_area.y + 30, piano_area.width - 20, piano_area.height - 40};

    DrawRectangleRec(piano_content, GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED)));

    // White keys (7 keys: C D E F G A B)
    float key_width = piano_content.width / 7.0f;
    for (int i = 0; i < 7; i++)
    {
        char keyname = piano_keys_lower[i];
        int key = keyname_to_key(keyname);

        bool is_pressed = prevKeyState[key];

        Rectangle key_rect = {
            piano_content.x + i * key_width + 2,
            piano_content.y + 5,
            key_width - 4,
            piano_content.height - 10};

        // Cyber style colors for white keys
        Color key_color;
        if (is_pressed)
        {
            key_color = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED));
        }
        else
        {
            key_color = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));
        }

        DrawRectangleRec(key_rect, key_color);
        DrawRectangleLinesEx(key_rect, 2, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

        char label[2] = {keyname, 0};
        Vector2 text_size = MeasureTextEx(GuiGetFont(), label, 20, 1);
        Vector2 text_pos = {
            key_rect.x + (key_rect.width - text_size.x) / 2,
            key_rect.y + key_rect.height - 35};

        Color text_color = is_pressed ? GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED)) : GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
        DrawTextEx(GuiGetFont(), label, text_pos, 20, 1, text_color);
    }

    // Black keys (5 keys: C# D# F# G# A#)
    int black_key_positions[] = {0, 1, 3, 4, 5}; // Between C-D, D-E, F-G, G-A, A-B
    for (int i = 0; i < 5; i++)
    {
        char keyname = piano_keys_upper[i];
        int key = keyname_to_key(keyname);

        bool is_pressed = prevKeyState[key];

        float black_key_x = piano_content.x + (black_key_positions[i] + 1) * key_width - 20;

        Rectangle black_key = {
            black_key_x,
            piano_content.y + 5,
            40,
            (piano_content.height - 10) * 0.6f,
        };

        Color key_color;
        if (is_pressed)
        {
            key_color = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED)); // Bright cyber color when pressed
        }
        else
        {
            key_color = GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_DISABLED));
        }

        DrawRectangleRec(black_key, key_color);
        DrawRectangleLinesEx(black_key, 2, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

        char label[2] = {keyname, 0};
        Vector2 text_size = MeasureTextEx(GuiGetFont(), label, 16, 1);
        Vector2 text_pos = {
            black_key.x + (black_key.width - text_size.x) / 2,
            black_key.y + black_key.height - 25};

        Color text_color = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED));
        DrawTextEx(GuiGetFont(), label, text_pos, 16, 1, text_color);
    }
}

static void draw_waveform_visualizer(UIState *ui)
{
    Rectangle wave_area = {500, 50, 450, 200};

    // Background
    GuiGroupBox(wave_area, "Audio Waveform");

    // Center line
    float center_y = wave_area.y + wave_area.height / 2;
    DrawLine(wave_area.x, center_y, wave_area.x + wave_area.width, center_y, GRAY);

    // Waveform
    float x_step = wave_area.width / (float)(WAVEFORM_SAMPLES - 1);

    for (int i = 0; i < WAVEFORM_SAMPLES - 1; i++)
    {
        float x1 = wave_area.x + i * x_step;
        float x2 = wave_area.x + (i + 1) * x_step;
        float y1 = center_y - ui->waveform_data[i] * wave_area.height * 0.4f;
        float y2 = center_y - ui->waveform_data[i + 1] * wave_area.height * 0.4f;

        DrawLine(x1, y1, x2, y2, LIME);
    }
}

static void draw_stats_panel(UIState *ui)
{
    Rectangle stats_area = {500, 270, 450, 200};

    // Background
    GuiGroupBox(stats_area, "Synthesizer Statistics");

    float y_pos = stats_area.y + 15;
    float line_height = 22;

    // Update stats
    ui->stat = synth_get_stat(ui->synth);

    // Display statistics
    DrawTextEx(GuiGetFont(), TextFormat("Frame per Read: %.1f", ui->stat.frame_per_read),
               (Vector2){stats_area.x + 15, y_pos}, 16, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += line_height;

    DrawTextEx(GuiGetFont(), TextFormat("Voice Buffer: %d", ui->stat.voice_buffer),
               (Vector2){stats_area.x + 15, y_pos}, 16, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += line_height;

    DrawTextEx(GuiGetFont(), TextFormat("Active Voices: %d / %d", ui->stat.voice_active, ui->stat.max_voice),
               (Vector2){stats_area.x + 15, y_pos}, 16, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += line_height;

    DrawTextEx(GuiGetFont(), TextFormat("Latency: %d ms", ui->stat.latency_ms),
               (Vector2){stats_area.x + 15, y_pos}, 16, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += line_height;

    DrawTextEx(GuiGetFont(), TextFormat("Samples Processed: %d", ui->stat.sample_processed),
               (Vector2){stats_area.x + 15, y_pos}, 16, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += line_height;

    DrawTextEx(GuiGetFont(), TextFormat("Recent Sample Size: %d", ui->stat.recent_sample_size),
               (Vector2){stats_area.x + 15, y_pos}, 16, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
}

static void draw_controls(UIState *ui)
{
    Rectangle controls_area = {50, 50, 400, 430};

    GuiGroupBox(controls_area, "Controls");

    float y_pos = controls_area.y + 30;

    // Instrument selection label
    DrawTextEx(GuiGetFont(), "Instrument:", (Vector2){controls_area.x + 20, y_pos}, 18, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += 25; // Space for dropdown (will be drawn at the end)

    Rectangle combo_rect = {controls_area.x + 20, y_pos, 350, 25};
    y_pos += 40; // Space after dropdown

    // Amplitude control
    DrawTextEx(GuiGetFont(), TextFormat("Amplitude: %.2f", ui->amplitude), (Vector2){controls_area.x + 20, y_pos}, 18, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += 25;

    Rectangle amp_rect = {controls_area.x + 20, y_pos, 350, 20};
    GuiSlider(amp_rect, "0", "1", &ui->amplitude, 0.0f, 1.0f);
    y_pos += 40;

    // Master volume
    DrawTextEx(GuiGetFont(), TextFormat("Master Volume: %.2f", ui->master_volume), (Vector2){controls_area.x + 20, y_pos}, 18, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += 25;

    Rectangle vol_rect = {controls_area.x + 20, y_pos, 350, 20};
    if (GuiSlider(vol_rect, "0", "1", &ui->master_volume, 0.0f, 1.0f))
    {
        synth_set_master_volume(ui->synth, ui->master_volume);
    }
    y_pos += 40;

    // Octave control
    DrawTextEx(GuiGetFont(), TextFormat("Octave: %d", ui->octave), (Vector2){controls_area.x + 20, y_pos}, 18, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += 25;

    Rectangle octave_rect = {controls_area.x + 20, y_pos, 350, 20};
    float octave_float = (float)ui->octave;
    if (GuiSlider(octave_rect, "1", "7", &octave_float, 1.0f, 7.0f))
    {
        ui->octave = (int)octave_float;
    }
    y_pos += 40;

    // Help button
    Rectangle help_rect = {controls_area.x + 20, y_pos, 100, 30};
    if (GuiButton(help_rect, "Help (F1)"))
    {
        ui->show_help = !ui->show_help;
    }

    // draw dropdown last so it appears on top of other components
    static bool dropdown_active = false;
    if (GuiDropdownBox(combo_rect, "Lead Square;Warm Bass;Ethereal Pad;Metallic Pluck;Wobble Bass;Bell Lead;Deep Drone",
                       &ui->selected_instrument, dropdown_active))
    {
        dropdown_active = !dropdown_active;
    }
}

static void draw_help_window(UIState *ui)
{
    if (!ui->show_help)
        return;

    Rectangle help_area = {200, 150, 600, 400};

    ui->show_help = !GuiWindowBox(help_area, "QSynth Help");

    float y_pos = help_area.y + 40;
    const char *help_lines[] = {
        "Piano Keys:",
        "  QWERTYUIOP - Play white keys (C D E F G A B C D E)",
        "  23567890   - Play black keys (C# D# F# G# A#)",
        "",
        "Controls:",
        "  Z/X        - Change octave down/up",
        "  F1         - Toggle this help",
        "",
        "Mouse:",
        "  Use sliders to adjust amplitude and volume",
        "  Select instruments from dropdown",
        "  Click piano keys to play notes",
        "",
        "Press F1 or click outside to close this help."};

    for (size_t i = 0; i < sizeof(help_lines) / sizeof(help_lines[0]); i++)
    {
        DrawTextEx(GuiGetFont(), help_lines[i], (Vector2){help_area.x + 20, y_pos}, 16, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
        y_pos += 25;
    }

    // Close on click outside
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse_pos = GetMousePosition();
        if (!CheckCollisionPointRec(mouse_pos, help_area))
        {
            ui->show_help = false;
        }
    }
}

static void draw_status_bar(UIState *ui)
{
    Rectangle status_area = {0, WINDOW_HEIGHT - 30, WINDOW_WIDTH, 30};

    DrawRectangleRec(status_area, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    DrawLine(0, WINDOW_HEIGHT - 30, WINDOW_WIDTH, WINDOW_HEIGHT - 30, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

    // Status text
    char status_text[256];
    snprintf(status_text, sizeof(status_text),
             "Instrument: %s | Octave: %d | Amplitude: %.2f | Master: %.2f | FPS: %d",
             instrument_names[ui->selected_instrument],
             ui->octave,
             ui->amplitude,
             ui->master_volume,
             GetFPS());

    DrawTextEx(GuiGetFont(), status_text, (Vector2){10, WINDOW_HEIGHT - 25}, 16, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
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
    ui.master_volume = 0.5f;
    ui.show_help = false;
    ui.stat = synth_get_stat(ui.synth);

    for (int i = 0; i < 256; i++)
    {
        ui.active_voices[i] = -1;
    }

    // Initialize Raylib
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "QSynth");
    SetTargetFPS(60);

    printf("QSynth UI started successfully!\n");
    printf("Press F1 for help\n");

    // Load style
    GuiLoadStyleCyber();
    // GuiSetFont(LoadFont("./Kyrou7Wide.ttf"));

    // Main loop
    while (!WindowShouldClose())
    {
        // Handle keyboard input
        for (int key = 0; key < 512; key++)
        {
            bool currentKeyState = IsKeyDown(key);

            // Key press detection
            if (currentKeyState && !prevKeyState[key])
            {
                // printf("key pressed: %c\n", key_to_keyname(key));
                handle_key_press(&ui, key);
            }

            else if (!currentKeyState && prevKeyState[key])
            {
                // printf("key released: %c\n", key_to_keyname(key));
                handle_key_release(&ui, key);
            }

            // Update previous state
            prevKeyState[key] = currentKeyState;
        }

        // Update waveform
        update_waveform_visualization(&ui);

        // Draw everything
        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        draw_controls(&ui);
        draw_waveform_visualizer(&ui);
        draw_stats_panel(&ui);
        draw_piano_keyboard(&ui);
        draw_status_bar(&ui);
        draw_help_window(&ui);

        EndDrawing();
    }

    // Cleanup
    CloseWindow();
    synth_stop(ui.synth);
    synth_cleanup(ui.synth);

    return 0;
}