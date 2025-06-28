#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

// Helper function to draw an arc between two angles
void draw_arc(Vector2 center, float radius, float start_angle_deg, float end_angle_deg, float thickness, Color color)
{
    // Ensure we're drawing in the correct direction
    if (start_angle_deg < end_angle_deg)
    {
        float temp = start_angle_deg;
        start_angle_deg = end_angle_deg;
        end_angle_deg = temp;
    }

    // Draw arc points
    for (float angle = start_angle_deg; angle >= end_angle_deg; angle -= 2.0f)
    {
        float angle_rad = angle * PI / 180.0f;
        Vector2 point = {
            center.x + cosf(angle_rad) * radius,
            center.y + sinf(angle_rad) * radius};
        DrawCircleV(point, thickness / 2, color);
    }
}

// Helper function to draw labels and value text
void draw_knob_labels(Rectangle bounds, const char *label, float value)
{
    // Label above knob
    Vector2 text_size = MeasureTextEx(GuiGetFont(), label, 16, 1);
    Vector2 label_pos = {
        bounds.x + (bounds.width - text_size.x) / 2,
        bounds.y - 20};
    DrawTextEx(GuiGetFont(), label, label_pos, 16, 1,
               GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

    // Simple value text below knob
    char value_text[16];
    snprintf(value_text, sizeof(value_text), "%.1f", value);

    Vector2 value_size = MeasureTextEx(GuiGetFont(), value_text, 16, 1);
    Vector2 value_pos = {
        bounds.x + (bounds.width - value_size.x) / 2,
        bounds.y + bounds.height};
    DrawTextEx(GuiGetFont(), value_text, value_pos, 16, 1,
               GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
}

void draw_knob(Rectangle bounds, const char *label, float value, float min_val, float max_val, bool *is_editing)
{
    Vector2 center = {bounds.x + bounds.width / 2, bounds.y + bounds.height / 2};
    float radius = fminf(bounds.width, bounds.height) / 2 - 8;

    // Constants for knob behavior
    const float START_ANGLE_DEG = -90.0f; // Starting position (top-left)
    const float END_ANGLE_DEG = -90.0f;   // Ending position (top-right)
    const float TOTAL_SWEEP = -360.0f;    // Total rotation range
    const float TRACK_THICKNESS = 3.0f;
    const float TRACK_RADIUS = radius - 6;

    // Draw outer ring (3D effect)
    DrawCircleV(center, radius + 1, GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED)));
    DrawCircleV(center, radius, GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)));

    // Calculate current value angle
    float normalized = (value - min_val) / (max_val - min_val);
    normalized = fmaxf(0.0f, fminf(1.0f, normalized)); // Clamp to [0,1]
    float current_angle = START_ANGLE_DEG - (normalized * TOTAL_SWEEP);

    draw_arc(center, TRACK_RADIUS, START_ANGLE_DEG, current_angle, TRACK_THICKNESS,
             GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED)));

    // Draw value indicator pointer
    float angle_rad = current_angle * PI / 180.0f;
    Vector2 indicator_start = {
        center.x + cosf(angle_rad) * (radius * 0.2f),
        center.y + sinf(angle_rad) * (radius * 0.2f)};
    Vector2 indicator_end = {
        center.x + cosf(angle_rad) * (radius - 4),
        center.y + sinf(angle_rad) * (radius - 4)};

    DrawLineEx(indicator_start, indicator_end, 4, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED)));

    // Draw center dot
    DrawCircleV(center, 4, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    DrawCircleV(center, 2, GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)));

    // Draw labels and value text
    draw_knob_labels(bounds, label, value);
}

// Helper function to handle knob interaction
float handle_knob_input(Rectangle bounds, float current_value, float min_val, float max_val, bool *is_editing)
{
    Vector2 mouse_pos = GetMousePosition();
    Vector2 center = {bounds.x + bounds.width / 2, bounds.y + bounds.height / 2};
    float radius = fminf(bounds.width, bounds.height) / 2 - 5;

    if (CheckCollisionPointCircle(mouse_pos, center, radius))
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            *is_editing = true;
        }
    }

    if (*is_editing)
    {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            // Get mouse delta for vertical drag
            Vector2 mouse_delta = GetMouseDelta();
            float sensitivity = (max_val - min_val) / 200.0f; // Adjust sensitivity as needed
            float new_value = current_value - mouse_delta.y * sensitivity;

            // Clamp value
            new_value = fmaxf(min_val, fminf(max_val, new_value));
            return new_value;
        }
        else
        {
            *is_editing = false;
        }
    }

    return current_value;
}

void draw_pedal_adj_panel(UIState *ui)
{
    static bool knob_editing[PEDAL_MAX_PARAMS] = {false};

    Rectangle panel_area = {1160, 50, 280, 600};
    GuiGroupBox(panel_area, "Pedal Configuration");

    size_t chain_size = synth_pedalchain_size(ui->synth);

    if (chain_size == 0)
    {
        // No pedals in chain
        Vector2 no_pedals_pos = {panel_area.x + 20, panel_area.y + 40};
        DrawTextEx(GuiGetFont(), "No pedals in chain", no_pedals_pos, 18, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
        return;
    }
    if (ui->clicking_pedal_idx == -1)
    {
        Vector2 no_pedals_pos = {panel_area.x + 20, panel_area.y + 40};
        DrawTextEx(GuiGetFont(), "Select a pedal to continue", no_pedals_pos, 18, 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
        return;
    }
    if (ui->clicking_pedal_idx >= (int)chain_size)
    {
        ui->clicking_pedal_idx = (int)chain_size - 1; 
    }

    float y_pos = panel_area.y + 30;

    PedalInfo pedal_info = synth_pedalchain_get(ui->synth, ui->clicking_pedal_idx);

    // Pedal name header
    char header_text[64];
    snprintf(header_text, sizeof(header_text), "Pedal #%d: %s",
             ui->clicking_pedal_idx + 1, pedal_info.name);

    DrawTextEx(GuiGetFont(), header_text,
               (Vector2){panel_area.x + 10, y_pos}, 16, 1,
               GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED)));
    y_pos += 30;

    // Bypass toggle
    Rectangle bypass_rect = {panel_area.x + 15, y_pos, 100, 30};
    bool is_bypassed = synth_pedalchain_is_bypass(ui->synth, ui->clicking_pedal_idx);

    if (GuiButton(bypass_rect, is_bypassed ? "BYPASSED" : "ACTIVE"))
    {
        synth_pedalchain_set_bypass(ui->synth, ui->clicking_pedal_idx, !is_bypassed);
    }

    DrawCircle(panel_area.x + 130, y_pos + 15, 5, GetColor(GuiGetStyle(DEFAULT, is_bypassed ? BASE_COLOR_DISABLED : BASE_COLOR_PRESSED)));

    y_pos += 50;

    // Parameters section
    DrawTextEx(GuiGetFont(), "Parameters:",
               (Vector2){panel_area.x + 15, y_pos}, 16, 1,
               GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += 50;

    // Draw parameter knobs with better layout
    int params_per_row = 2;
    float knob_size = 70;
    float knob_spacing_x = 140;
    float knob_spacing_y = 120;

    for (int param_idx = 0; param_idx < pedal_info.param_count && param_idx < PEDAL_MAX_PARAMS; param_idx++)
    {
        int row = param_idx / params_per_row;
        int col = param_idx % params_per_row;

        Rectangle knob_bounds = {
            panel_area.x + 30 + col * knob_spacing_x,
            y_pos + row * knob_spacing_y,
            knob_size,
            knob_size,
        };

        double current_value = pedal_info.params[param_idx].current_value;
        float min_val = pedal_info.params[param_idx].min_value;
        float max_val = pedal_info.params[param_idx].max_value;

        // Draw knob
        draw_knob(knob_bounds, pedal_info.params[param_idx].name,
                  current_value, min_val, max_val, &knob_editing[param_idx]);

        // Handle knob input
        float new_value = handle_knob_input(knob_bounds, current_value, min_val, max_val,
                                            &knob_editing[param_idx]);

        if (fabs(new_value - current_value) > 0.001)
        {
            synth_pedalchain_set(ui->synth, ui->clicking_pedal_idx, param_idx, new_value);
        }
    }

    // Additional controls at the bottom
    y_pos += ((pedal_info.param_count + 1) / params_per_row) * knob_spacing_y + 20;

    // Reset to default button with better styling
    Rectangle reset_rect = {panel_area.x + 15, y_pos, 140, 35};
    if (GuiButton(reset_rect, "Reset to Default"))
    {
        for (int i = 0; i < pedal_info.param_count; i++)
        {
            double default_val = pedal_info.params[i].default_value;
            synth_pedalchain_set(ui->synth, ui->clicking_pedal_idx, i, default_val);
        }
    }
}