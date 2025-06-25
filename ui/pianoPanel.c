#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

void draw_piano_keyboard(UIState *ui)
{
    Rectangle piano_area = {50, 500, 870, 150};

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
