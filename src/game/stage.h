#pragma once
#include "framework/input.h"
#include "framework/entities/entityMesh.h"

class Entity;
class EntityMesh;
class Camera;

enum : uint8_t {
    STAGE_MENU,
    STAGE_PLAY
};

class Stage {
public:
    virtual void render() {}; // Empty body
    virtual void init() {}; // Empty body
    virtual void update(double seconds_elapsed) {}; // Empty body

    // Add callbacks for each stage to be
    virtual void onEnter(Stage* prev_stage) {};
    virtual void onLeave(Stage* next_stage) {};
    virtual void onKeyDown(SDL_KeyboardEvent event) {};
    virtual void onKeyUp(SDL_KeyboardEvent event) {};
    virtual void onMouseButtonDown(SDL_MouseButtonEvent event) {};
    virtual void onMouseButtonUp(SDL_MouseButtonEvent event) {};
    virtual void onMouseMove(SDL_MouseMotionEvent event) {};
    virtual void onMouseWheel(SDL_MouseWheelEvent event) {};
    virtual void onGamepadButtonDown(SDL_JoyButtonEvent event) {};
    virtual void onGamepadButtonUp(SDL_JoyButtonEvent event) {};
    virtual void onResize(int width, int height) {};
};

class MenuStage : public Stage {
public:
    MenuStage() {};
    void init() override;
    void render() override;
    void update(double seconds_elapsed) override;
    void onEnter(Stage* prev_stage) override;

private:
    EntityMesh* background;
    EntityMesh* play_button;
    EntityMesh* exit_button;
};

class PlayStage : public Stage {
public:
    PlayStage() {};
    void init() override;
    void render() override;
    void update(double seconds_elapsed) override;
    void onEnter(Stage* prev_stage) override;
    void onKeyDown(SDL_KeyboardEvent event) override;
}; 