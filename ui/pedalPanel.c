#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

void draw_pedal_panel(UIState *ui)
{
    static const char *pedal_names[PEDAL_COUNT];
    static bool is_pedal_names_init = false;
    static char dropdown_options[256];
    static bool pedal_dropdown_active = false;

    if (!is_pedal_names_init)
    {
        is_pedal_names_init = true;

        // Initialize pedal names from pedal info
        for (int i = 0; i < PEDAL_COUNT; ++i)
            pedal_names[i] = ui->pedals_info[i].name;

        // Build dropdown options string (semicolon separated)
        strcpy(dropdown_options, "");
        for (int i = 0; i < PEDAL_COUNT; ++i)
        {
            if (i > 0)
                strcat(dropdown_options, ";");
            strcat(dropdown_options, pedal_names[i]);
        }
    }

    // Fixed panel on the right side
    Rectangle panel_area = {940, 50, 200, 600};

    GuiGroupBox(panel_area, "Pedal Chain");

    float y_pos = panel_area.y + 30;

    // Add pedal section
    DrawTextEx(GuiGetFont(), "Add Pedal:",
               (Vector2){panel_area.x + 10, y_pos}, 18, 1,
               GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += 25;

    // Store dropdown rectangle for later rendering
    Rectangle dropdown_rect = {panel_area.x + 10, y_pos, panel_area.width - 20, 25};

    y_pos += 35; // Leave space for dropdown

    // Add and Clear buttons
    Rectangle add_btn_rect = {panel_area.x + 10, y_pos, 85, 25};
    Rectangle clear_btn_rect = {panel_area.x + panel_area.width - 95, y_pos, 85, 25};

    // the reason to add pedal_dropdown_active is to prevent components overlap
    if (GuiButton(add_btn_rect, "Add") && !pedal_dropdown_active)
    {
        synth_pedalchain_append(ui->synth, ui->selected_pedal_type);
    }

    if (GuiButton(clear_btn_rect, "Clear") && !pedal_dropdown_active)
    {
        size_t chain_size = synth_pedalchain_size(ui->synth);
        for (int i = chain_size - 1; i >= 0; i--)
        {
            synth_pedalchain_remove(ui->synth, i);
        }
    }
    y_pos += 40;

    // Chain title
    DrawTextEx(GuiGetFont(), "Current Chain:",
               (Vector2){panel_area.x + 10, y_pos}, 18, 1,
               GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    y_pos += 30;

    // Draw current pedal chain vertically
    size_t chain_size = synth_pedalchain_size(ui->synth);

    if (chain_size == 0)
    {
        DrawTextEx(GuiGetFont(), "No pedals",
                   (Vector2){panel_area.x + 15, y_pos}, 16, 2, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_DISABLED)));
    }
    else
    {
        // Draw pedals vertically with signal flow arrows
        float pedal_height = 50;
        float spacing = 15;
        float chain_start_y = y_pos;
        float pedal_width = panel_area.width - 30;

        // Track if any pedal was removed to break the loop
        bool pedal_removed = false;

        for (size_t i = 0; i < chain_size && !pedal_removed; i++)
        {
            Rectangle pedal_rect = {
                panel_area.x + 15,
                chain_start_y + i * (pedal_height + spacing),
                pedal_width,
                pedal_height};

            // Store original position for non-dragged pedals
            Rectangle original_pedal_rect = pedal_rect;

            bool is_this_dragging = (ui->is_dragging && ui->dragging_pedal_idx == (int)i);

            if (is_this_dragging)
            {
                Vector2 mouse_pos = GetMousePosition();
                pedal_rect.y = mouse_pos.y + ui->drag_offset.y;
            }

            // Draw pedal box
            Color pedal_color = is_this_dragging ? GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED)) : GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));

            DrawRectangleRec(pedal_rect, pedal_color);
            DrawRectangleLinesEx(pedal_rect, 2, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

            const char *name = synth_pedalchain_get(ui->synth, i).name;

            // Pedal name
            Vector2 text_size = MeasureTextEx(GuiGetFont(), name, 14, 1);
            Vector2 text_pos = {
                pedal_rect.x + (pedal_rect.width - text_size.x) / 2,
                pedal_rect.y + 10};
            DrawTextEx(GuiGetFont(), name, text_pos, 14, 1,
                       GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

            // Pedal index
            char index_text[16];
            snprintf(index_text, sizeof(index_text), "#%zu", i + 1);
            DrawTextEx(GuiGetFont(), index_text,
                       (Vector2){pedal_rect.x + 5, pedal_rect.y + pedal_rect.height - 20},
                       12, 1, GRAY);

            // Remove button (moved to top-right corner)
            Rectangle remove_btn = {
                pedal_rect.x + pedal_rect.width - 25,
                pedal_rect.y + 5,
                20, 20};

            if (GuiButton(remove_btn, "X") && !pedal_dropdown_active)
            {
                synth_pedalchain_remove(ui->synth, i);
                // Reset drag/click state
                if (ui->is_dragging)
                {
                    ui->is_dragging = false;
                    ui->dragging_pedal_idx = -1;
                }
                pedal_removed = true;
                continue; // Skip the rest of this iteration
            }

            // Draw signal flow arrow (except for last pedal) - use original position
            if (i < chain_size - 1 && !is_this_dragging)
            {
                Vector2 arrow_start = {
                    original_pedal_rect.x + original_pedal_rect.width / 2,
                    original_pedal_rect.y + original_pedal_rect.height};
                Vector2 arrow_end = {
                    original_pedal_rect.x + original_pedal_rect.width / 2,
                    original_pedal_rect.y + original_pedal_rect.height + spacing};

                DrawLineV(arrow_start, arrow_end, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED)));
            }

            // Handle mouse interaction for dragging (exclude remove button area)
            if (!ui->is_dragging && !pedal_dropdown_active)
            {
                Vector2 mouse_pos = GetMousePosition();

                // Create draggable area (exclude remove button)
                Rectangle drag_area = {
                    pedal_rect.x,
                    pedal_rect.y,
                    pedal_rect.width - 30, // Exclude remove button area
                    pedal_rect.height};

                if (CheckCollisionPointRec(mouse_pos, drag_area) &&
                    IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    ui->is_dragging = true;
                    ui->dragging_pedal_idx = i;
                    ui->clicking_pedal_idx = i;
                    ui->drag_offset.y = pedal_rect.y - mouse_pos.y;
                    printf("Started dragging pedal %zu\n", i); // Debug
                }
            }
        }

        // Handle drag release for reordering
        if (ui->is_dragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            Vector2 mouse_pos = GetMousePosition();
            int drop_idx = -1;

            printf("Drag released at mouse pos: %.1f, %.1f\n", mouse_pos.x, mouse_pos.y); // Debug

            // Find which position to drop at
            for (int i = 0; i < chain_size; i++)
            {
                if (i == ui->dragging_pedal_idx)
                    continue;

                float target_y = chain_start_y + i * (pedal_height + spacing);
                Rectangle target_rect = {
                    panel_area.x + 15,
                    target_y,
                    pedal_width,
                    pedal_height};

                if (CheckCollisionPointRec(mouse_pos, target_rect))
                {
                    drop_idx = i;
                    break;
                }
            }

            printf("Drop index: %d, dragging index: %d\n", drop_idx, ui->dragging_pedal_idx);

            // Perform swap if valid drop target
            if (drop_idx >= 0 && drop_idx != ui->dragging_pedal_idx)
            {
                printf("Swapping pedals %d and %d\n", ui->dragging_pedal_idx, drop_idx);
                synth_pedalchain_swap(ui->synth, ui->dragging_pedal_idx, drop_idx);
            }

            ui->is_dragging = false;
            ui->dragging_pedal_idx = -1;
        }
    }

    // RENDER DROPDOWN LAST to appear on top
    if (GuiDropdownBox(dropdown_rect, dropdown_options,
                       &ui->selected_pedal_type, pedal_dropdown_active))
    {
        pedal_dropdown_active = !pedal_dropdown_active;
    }
}