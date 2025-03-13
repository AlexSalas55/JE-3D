#pragma once
#include "framework/input.h"
#include "framework/entities/entityMesh.h"
#include "framework/audio.h"

class Entity;
class EntityMesh;
class Camera;

enum : uint8_t {
    STAGE_MENU,
    STAGE_PLAY,
    STAGE_TRAINING
};

class Stage {
public:
    virtual void render() {};
    virtual void init() {}; 
    virtual void update(double seconds_elapsed) {};

    // Add callbacks for each stage to be
    virtual void onEnter(Stage* prev_stage) {};
    virtual void onLeave(Stage* next_stage) {};
    virtual void onKeyDown(SDL_KeyboardEvent event) {};
    virtual void onKeyUp(SDL_KeyboardEvent event) {};
    virtual void onMouseButtonDown(SDL_MouseButtonEvent event) {};
    virtual void onMouseButtonUp(SDL_MouseButtonEvent event) {};
    virtual void onMouseMove(SDL_MouseMotionEvent event) {};
    virtual void onMouseWheel(SDL_MouseWheelEvent event) {};
    virtual void onGamepadButtonDown(SDL_ControllerButtonEvent event) {};
    virtual void onGamepadButtonUp(SDL_ControllerButtonEvent event) {};
    virtual void onGamepadAxisMotion(SDL_ControllerAxisEvent event) {};
    virtual void onResize(int width, int height) {};
};

class MenuStage : public Stage {
public:
    MenuStage() {};
    void init() override;
    void render() override;
    void update(double seconds_elapsed) override;
    void onEnter(Stage* prev_stage) override;
    void onLeave(Stage* next_stage) override;

private:
    EntityMesh* background;
    EntityMesh* logo;
    EntityMesh* play_button;
    EntityMesh* multiplayer_button;
    EntityMesh* training_button;
    EntityMesh* exit_button;

    // normal textures
    Texture* play_normal;
    Texture* training_normal;
    Texture* exit_normal;
    Texture* multiplayer_normal;
    

    // hover textures
    Texture* play_hover_tex;
    Texture* multiplayer_hover_tex;
    Texture* training_hover_tex;
    Texture* exit_hover_tex;

    // audio channel for menu music
    HCHANNEL menu_music_channel;
};

class PlayStage : public Stage {
public:
    PlayStage() {};
    void init() override;
    void render() override;
    void update(double seconds_elapsed) override;
    void onEnter(Stage* prev_stage) override;
    void onLeave(Stage* next_stage) override;
    void onKeyDown(SDL_KeyboardEvent event) override;
    void restart();

private:
    HCHANNEL play_music_channel;    // audio channel for game music
    double chronometer1 = 0.0;
    double chronometer2 = 0.0;
    double checkpoint1_time = 0.0;
    double checkpoint2_time = 0.0;
    bool timer1_active = false;
    bool timer2_active = false;
    bool player1_finished = false;
    bool player2_finished = false;
    bool player1_checkpoint = false;
    bool player2_checkpoint = false;
    
    // End game UI state
    bool show_end_screen = false;
    bool player1_is_winner = false;  // For multiplayer mode
    Vector2 restart_button_pos;
    Vector2 menu_button_pos;
    Vector2 button_size;

    void renderEndScreen();
    void renderChronometer(double time, int x, int y); // Helper to render time
};

class TrainingStage : public Stage {
public:
    TrainingStage() {};
    void init() override;
    void render() override;
    void update(double seconds_elapsed) override;
    void onEnter(Stage* prev_stage) override;
    void onLeave(Stage* next_stage) override;
    void onKeyDown(SDL_KeyboardEvent event) override;
    void restart();

private:
    HCHANNEL training_music_channel = 0;    // audio channel for training music
    bool show_tutorial = true; // flag to show/hide tutorial image
    
    // tutorial UI elements
    EntityMesh* tutorial_background = nullptr;
}; 