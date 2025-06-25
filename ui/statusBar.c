#include <stdio.h>

#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

void draw_status_bar(UIState *ui)
{
    Rectangle status_area = {0, WINDOW_HEIGHT - 30, WINDOW_WIDTH, 30};

    DrawRectangleRec(status_area, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    DrawLine(0, WINDOW_HEIGHT - 30, WINDOW_WIDTH, WINDOW_HEIGHT - 30, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

    // Status text
    char status_text[256];
    snprintf(status_text, sizeof(status_text),
             "Instrument: %s | Octave: %d | Amplitude: %.2f | Master: %.2f | Balance: %.2f | FPS: %d",
             instrument_names[ui->selected_instrument],
             ui->octave,
             ui->amplitude,
             ui->master_volume,
             ui->pan,
             GetFPS());

    DrawTextEx(GuiGetFont(), status_text, (Vector2){10, WINDOW_HEIGHT - 25}, 16, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
}
