#include <raylib.h>
#include <raymath.h>


#include <functional>
#include <iostream>
#include <string>
#include <vector>

// Struct to encapsulate our UI library
struct UiLibrary
{
    // ID of the currently hot widget
    int hot = -1;

    // ID of the currently active widget
    int active = -1;

    // Creates a button with the specified text and bounds
    // Returns true if this button was clicked in this frame
    bool Button(int id, const std::string& text, const Rectangle& bounds, const Color& color)
    {
        bool result = false;
        Color buttonColor = color;
        Color textColor = BLACK;

        // If this button is the currently active widget, that means
        // the user is currently interacting with this widget
        if (id == active)
        {
            // If the user released the mouse button while we are active,
            // register as a click
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                // Make sure that we are still the hot widget
                // (User might have dragged mouse to another widget, in which case we will not count as a button click)
                if (id == hot)
                {
                    result = true;
                }

                // Set ourselves to not be active anymore
                active = -1;
            }
        }

        // If we are currently the hot widget
        if (id == hot)
        {
            // If the user pressed the left mouse button, that means the user started
            // interacting with this widget, so we set this widget as active
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                active = id;
            }
        }

        // If the mouse cursor is hovering within our boundaries
        if (CheckCollisionPointRec(GetMousePosition(), bounds))
        {
            // Set this widget to be the hot widget
            hot = id;
            buttonColor = YELLOW;
        }
        // If the mouse cursor is not on top of this widget, and this widget
        // was previously the hot widget, set the hot widget to -1
        // We check if this widget was the previously hot widget since there is a possibility that
        // the hot widget is now a different widget, and we don't want to overwrite that with -1
        else if (hot == id)
        {
            hot = -1;
        }

        if (id == active) {
            buttonColor = GREEN;
            textColor = WHITE;
        }

        // Draw our button regardless of what happens
        DrawRectangleRec(bounds, buttonColor);
        DrawText(text.c_str(), bounds.x, bounds.y, 14, textColor);

        return result;
    }
};

struct checkbox {
    bool button(int id, const Rectangle& bounds, bool& checked) {
        if(CheckCollisionPointRec(GetMousePosition(), bounds)) {
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                checked = !checked;
            }
        }

        DrawRectangleRec(bounds, checked ? RED : GREEN);
        return checked;
    }
};

bool isDisabled = false;

void ChangeResolution(int width, int height) {
    if (!isDisabled) {
        SetWindowSize(width, height);
    }
}

void DisableButton() {
    isDisabled = true;
}



int main()
{
    int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 600;
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Immediate Mode");
    SetTargetFPS(60);
    checkbox cb;
    UiLibrary uiLibrary;
    bool checked = false;
    
    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);
        BeginDrawing();
        DrawText("Resolution:", 10, 10, 20, BLACK);
        //Draw Button
        if (uiLibrary.Button(0, "800x600", { 140, 10, 80, 40 }, GRAY))
        {
            ChangeResolution(800, 600);
        }
        if (uiLibrary.Button(1, "1280x720", { 230, 10, 80, 40 }, GRAY))
        {
            ChangeResolution(1280, 720);
        }
        if (uiLibrary.Button(2, "1366x768", { 320, 10, 80, 40 }, GRAY))
        {
            ChangeResolution(1366, 768);
        }

        if(cb.button(1, {10, 60, 20, 20}, checked)) {
            DisableButton();
        } else {
            isDisabled = false;
        }
        DrawText("Lock Screen Size", 40, 60, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
