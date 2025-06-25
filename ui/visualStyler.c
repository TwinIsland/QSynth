#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ui.h"
#include "qsynth.h"
#include "raylib.h"

#include "raygui.h"

#include "styles/style_jungle.h"   // raygui style: jungle
#include "styles/style_candy.h"    // raygui style: candy
#include "styles/style_lavanda.h"  // raygui style: lavanda
#include "styles/style_cyber.h"    // raygui style: cyber
#include "styles/style_terminal.h" // raygui style: terminal
#include "styles/style_ashes.h"    // raygui style: ashes
#include "styles/style_bluish.h"   // raygui style: bluish
#include "styles/style_dark.h"     // raygui style: dark
#include "styles/style_cherry.h"   // raygui style: cherry
#include "styles/style_sunny.h"    // raygui style: sunny
#include "styles/style_enefete.h"  // raygui style: enefete

void update_visual_style(UIState *ui)
{
    // Load style
    if (ui->visualStyleActive != ui->prevVisualStyleActive)
    {
        printf("switch theme\n");

        // Reset to default internal style
        GuiLoadStyleDefault();

        switch (ui->visualStyleActive)
        {
        case 1:
            GuiLoadStyleJungle();
            break;
        case 2:
            GuiLoadStyleCandy();
            break;
        case 3:
            GuiLoadStyleLavanda();
            break;
        case 4:
            GuiLoadStyleCyber();
            break;
        case 5:
            GuiLoadStyleTerminal();
            break;
        case 6:
            GuiLoadStyleAshes();
            break;
        case 7:
            GuiLoadStyleBluish();
            break;
        case 8:
            GuiLoadStyleDark();
            break;
        case 9:
            GuiLoadStyleCherry();
            break;
        case 10:
            GuiLoadStyleSunny();
            break;
        case 11:
            GuiLoadStyleEnefete();
            break;
        default:
            break;
        }

        ui->prevVisualStyleActive = ui->visualStyleActive;
    }
}