#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

void draw_stats_panel(UIState *ui)
{
    Rectangle stats_area = {470, 270, 450, 200};

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
