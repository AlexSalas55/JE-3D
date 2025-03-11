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
    void onLeave(Stage* next_stage) override;

private:
    EntityMesh* background;
    EntityMesh* logo;
    EntityMesh* play_button;
    EntityMesh* multiplayer_button;
    EntityMesh* training_button;
    EntityMesh* exit_button;

    // Texturas normales
    Texture* play_normal;
    Texture* training_normal;
    Texture* exit_normal;
    Texture* multiplayer_normal;
    

    // Texturas hover
    Texture* play_hover_tex;
    Texture* multiplayer_hover_tex;
    Texture* training_hover_tex;
    Texture* exit_hover_tex;

    // Canal de audio para la música del menú
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
    HCHANNEL play_music_channel;    // Canal de audio para la música del modo de juego
    double chronometer1 = 0.0;  // Timer for player 1
    double chronometer2 = 0.0;  // Timer for player 2
    double checkpoint1_time = 0.0;  // Checkpoint time for player 1
    double checkpoint2_time = 0.0;  // Checkpoint time for player 2
    bool timer1_active = false; // Track if player 1's timer is running
    bool timer2_active = false; // Track if player 2's timer is running
    bool player1_finished = false; // Track if player 1 has finished
    bool player2_finished = false; // Track if player 2 has finished
    bool player1_checkpoint = false; // Track if player 1 passed checkpoint
    bool player2_checkpoint = false; // Track if player 2 passed checkpoint
    
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
    HCHANNEL training_music_channel = 0;    // Canal de audio para la música del modo de entrenamiento
    bool show_tutorial = true; // Flag to show/hide tutorial image
    
    // Tutorial UI elements
    EntityMesh* tutorial_background = nullptr;
}; 