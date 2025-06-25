#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

void draw_credit_window(UIState *ui)
{
    if (!ui->show_credit)
        return;

    Rectangle credit_area = {200, 150, 600, 350};

    ui->show_credit = !GuiWindowBox(credit_area, "QSynth Credits - The Sound of Pure Genius");

    float y_pos = credit_area.y + 40;
    const char *credit_lines[] = {
        " QSynth - Making Computers Sing Since Today! ",
        "A high-performance multi-layered audio synthesizer written in C",
        "",
        " Platform Support:",
        "Windows/Linux/MacOS/WebAssembly(planned)",
        "",
        " Libraries Used",
        "- miniaudio by mackron - github.com/mackron/miniaudio",
        "- pthreads-win32 - sourceware.org/pthreads-win32",
        "- raylib by raysan5 - github.com/raysan5/raylib",
        "",
        "Created by: Tianyi Huang (twisland@outlook.com)",
        ""
        "GitHub: github.com/TwinIsland/QSynth",
    };

    for (size_t i = 0; i < sizeof(credit_lines) / sizeof(credit_lines[0]); i++)
    {
        Color text_color = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));

        if (i == 0 || i == 3 || i == 6 || i == 10 || i == 11 || i == 13)
        {
            text_color = GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED));
        }

        DrawTextEx(GuiGetFont(), credit_lines[i],
                   (Vector2){credit_area.x + 20, y_pos}, 14, 1, text_color);
        y_pos += 18;
    }

    // Close on click outside
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse_pos = GetMousePosition();
        if (!CheckCollisionPointRec(mouse_pos, credit_area))
        {
            ui->show_credit = false;
        }
    }
}