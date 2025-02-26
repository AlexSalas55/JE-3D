#pragma once
#include "framework/includes.h"
#include "framework/framework.h"
#include "framework/input.h"
#include <SDL2/SDL.h>

enum eUIButtonID {
    UI_BUTTON_UNDEFINED = -1,
    UI_BUTTON_PLAY,
    UI_BUTTON_EXIT
};

class UI {
public:
    static bool addButton(Vector2 position, Vector2 size, const char* texture_path);
    
    // Mouse event handlers
    static void onMouseMove(SDL_MouseMotionEvent event) {};
    static void onMouseButtonDown(SDL_MouseButtonEvent event) {};
    static void onMouseButtonUp(SDL_MouseButtonEvent event) {};
    static void wasMousePressed(SDL_MouseButtonEvent event) {};
}; 