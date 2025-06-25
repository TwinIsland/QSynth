#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

void draw_controls(UIState *ui)
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

    // Pan control
    DrawTextEx(GuiGetFont(), TextFormat("Balance: %.2f", ui->pan), (Vector2){controls_area.x + 20, y_pos}, 18, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += 25;

    Rectangle pan_rect = {controls_area.x + 20, y_pos, 350, 20};
    GuiSlider(pan_rect, "0", "1", &ui->pan, 0.0f, 1.0f);

    y_pos += 60;

    // Help button
    Rectangle help_rect = {controls_area.x + 20, y_pos, 100, 30};
    if (GuiButton(help_rect, "Help (F1)"))
    {
        ui->show_help = !ui->show_help;
    }

    // Credit Button
    Rectangle credit_rect = {controls_area.x + 130, y_pos, 100, 30};
    if (GuiButton(credit_rect, "Credit (F2)"))
    {
        ui->show_credit = !ui->show_credit;
    }

    // Visuals options
    GuiComboBox((Rectangle){controls_area.x + 240, y_pos, 120, 30}, "default;Jungle;Candy;Lavanda;Cyber;Terminal;Ashes;Bluish;Dark;Cherry;Sunny;Enefete", &ui->visualStyleActive);

    // draw dropdown last so it appears on top of other components
    static bool dropdown_active = false;
    if (GuiDropdownBox(combo_rect, "Lead Square;Warm Bass;Ethereal Pad;Metallic Pluck;Wobble Bass;Bell Lead;Deep Drone",
                       &ui->selected_instrument, dropdown_active))
    {
        dropdown_active = !dropdown_active;
    }
}
