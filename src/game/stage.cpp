#include "stage.h"
#include "world.h"
#include "game.h"
#include "framework/ui.h"
#include "framework/entities/entity.h"
#include "framework/entities/entityMesh.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "graphics/shader.h"

void PlayStage::init() {
    // Asegurarse de que World está inicializado
    if (!World::get_instance()->root) {
        World::get_instance(); // Esto creará una nueva instancia de World si no existe
    }
}

void PlayStage::onEnter(Stage* prev_stage) {
    // Lock/unlock cursor depending on free camera
    bool must_lock = !World::get_instance()->free_camera;
    Game::instance->setMouseLocked(must_lock);
}

void PlayStage::render() {
    World* world = World::get_instance();
    world->render();
}

void PlayStage::update(double seconds_elapsed) {
    World* world = World::get_instance();
    world->update(seconds_elapsed);
}

void PlayStage::onKeyDown(SDL_KeyboardEvent event) {
    // Handle key events specific to play stage
}

void MenuStage::init() {
    int width = Game::instance->window_width;
    int height = Game::instance->window_height;

    // Create background
    Material background_mat;
    background_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    background_mat.diffuse = Texture::Get("data/textures/ui/menu.png");
    background_mat.color = Vector4(1,1,1,1);
    
    background = new EntityMesh();
    background->mesh = new Mesh();
    background->mesh->createQuad(width * 0.5f, height * 0.5f, width, height, true);
    background->material = background_mat;

    // Create play button
    Material play_mat;
    play_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    play_mat.diffuse = Texture::Get("data/textures/ui/play_button.png");
    play_mat.color = Vector4(1,1,1,1);
    
    play_button = new EntityMesh();
    play_button->mesh = new Mesh();
    play_button->mesh->createQuad(width * 0.5f, 450, 190, 49, true);
    play_button->material = play_mat;

    // Create exit button
    Material exit_mat;
    exit_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    exit_mat.diffuse = Texture::Get("data/textures/ui/exit_button.png");
    exit_mat.color = Vector4(1,1,1,1);
    
    exit_button = new EntityMesh();
    exit_button->mesh = new Mesh();
    exit_button->mesh->createQuad(width * 0.5f, 520, 190, 49, true);
    exit_button->material = exit_mat;

    background->addChild(play_button);
    background->addChild(exit_button);
}

void MenuStage::render() {
    // Disable depth testing for 2D rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Use 2D camera for menu
    Camera* camera2D = World::get_instance()->camera2D;
    background->render(camera2D);

    // Restore OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void MenuStage::update(double seconds_elapsed) {
    Vector2 mouse_pos = Input::mouse_position;
    
    // Check if mouse is over play button
    float play_x = Game::instance->window_width * 0.5f;
    float play_y = 450;
    
    if (mouse_pos.x > (play_x - 95) && mouse_pos.x < (play_x + 95) &&
        mouse_pos.y > (play_y - 24.5f) && mouse_pos.y < (play_y + 24.5f)) {
        play_button->material.color = Vector4::WHITE * 2.0f;
        if (Input::wasMousePressed(SDL_BUTTON_LEFT)) {
            Game::instance->goToStage(STAGE_PLAY);
        }
    } else {
        play_button->material.color = Vector4::WHITE;
    }
    
    background->update(seconds_elapsed);
}

void MenuStage::onEnter(Stage* prev_stage) {
    Game::instance->setMouseLocked(false);
} 