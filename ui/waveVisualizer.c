#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

void draw_waveform_visualizer(UIState *ui)
{
    Rectangle wave_area = {470, 50, 450, 200};

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

void update_waveform_visualization(UIState *ui)
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
