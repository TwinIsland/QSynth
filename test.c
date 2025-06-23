#include "raylib.h"
#include <stdio.h>

int main(void)
{
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 450;
    
    InitWindow(screenWidth, screenHeight, "Key Press Detection");
    SetTargetFPS(60);
    
    // Array to track previous key states
    bool prevKeyState[512] = {false}; // raylib supports up to 512 keys
    
    while (!WindowShouldClose())
    {
        // Check all possible keys
        for (int key = 0; key < 512; key++)
        {
            bool currentKeyState = IsKeyDown(key);
            
            // Key press detection (was up, now down)
            if (currentKeyState && !prevKeyState[key])
            {
                printf("Key PRESSED: %d", key);
                
                // Print key name if it's a common key
                if (key >= KEY_A && key <= KEY_Z)
                    printf(" (%c)", 'A' + (key - KEY_A));
                else if (key >= KEY_ZERO && key <= KEY_NINE)
                    printf(" (%c)", '0' + (key - KEY_ZERO));
                else if (key == KEY_SPACE)
                    printf(" (SPACE)");
                else if (key == KEY_ENTER)
                    printf(" (ENTER)");
                else if (key == KEY_ESCAPE)
                    printf(" (ESCAPE)");
                else if (key == KEY_BACKSPACE)
                    printf(" (BACKSPACE)");
                else if (key == KEY_TAB)
                    printf(" (TAB)");
                else if (key == KEY_LEFT)
                    printf(" (LEFT ARROW)");
                else if (key == KEY_RIGHT)
                    printf(" (RIGHT ARROW)");
                else if (key == KEY_UP)
                    printf(" (UP ARROW)");
                else if (key == KEY_DOWN)
                    printf(" (DOWN ARROW)");
                
                printf("\n");
            }
            
            // Key release detection (was down, now up)
            if (!currentKeyState && prevKeyState[key])
            {
                printf("Key RELEASED: %d", key);
                
                // Print key name if it's a common key
                if (key >= KEY_A && key <= KEY_Z)
                    printf(" (%c)", 'A' + (key - KEY_A));
                else if (key >= KEY_ZERO && key <= KEY_NINE)
                    printf(" (%c)", '0' + (key - KEY_ZERO));
                else if (key == KEY_SPACE)
                    printf(" (SPACE)");
                else if (key == KEY_ENTER)
                    printf(" (ENTER)");
                else if (key == KEY_ESCAPE)
                    printf(" (ESCAPE)");
                else if (key == KEY_BACKSPACE)
                    printf(" (BACKSPACE)");
                else if (key == KEY_TAB)
                    printf(" (TAB)");
                else if (key == KEY_LEFT)
                    printf(" (LEFT ARROW)");
                else if (key == KEY_RIGHT)
                    printf(" (RIGHT ARROW)");
                else if (key == KEY_UP)
                    printf(" (UP ARROW)");
                else if (key == KEY_DOWN)
                    printf(" (DOWN ARROW)");
                
                printf("\n");
            }
            
            // Update previous state
            prevKeyState[key] = currentKeyState;
        }
        
        // Drawing
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        DrawText("Press any key to see key events in console", 10, 10, 20, DARKGRAY);
        DrawText("Press ESC to close", 10, 40, 20, DARKGRAY);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}