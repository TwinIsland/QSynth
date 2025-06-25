#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

void draw_help_window(UIState *ui)
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
