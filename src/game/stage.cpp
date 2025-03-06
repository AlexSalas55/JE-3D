#include "stage.h"
#include "world.h"
#include "game.h"
#include "player.h"
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
    if (event.keysym.scancode == SDL_SCANCODE_P) {
        Game* game = Game::instance;
        World* world = World::get_instance();
        
        // Toggle multiplayer mode
        game->multiplayer_enabled = !game->multiplayer_enabled;
        
        if (game->multiplayer_enabled) {
            // Create player 2 and its camera
            Material player_material;
            player_material.shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");
            player_material.diffuse = Texture::Get("data/meshes/playerColor.png");
            
            // Create player2 with the same mesh as player1 to ensure skeleton compatibility
            Mesh* player_mesh = world->player->mesh;
            world->player2 = new Player(player_mesh, player_material, "player2");
            world->player2->model.setTranslation(5.0f, 200.0f, 0.0f);
            
            // Ensure animations are properly initialized before adding to scene
            world->player2->isAnimated = world->player->isAnimated;
            
            // Initialize animation with the same state as player1
            Animation* currentAnim = world->player->animator.getCurrentAnimation();
            if (currentAnim) {
                world->player2->animator.playAnimation(currentAnim->name.c_str(), true);
            } else {
                world->player2->animator.playAnimation("data/meshes/animations/idle.skanim", true);
            }
            
            // Add to scene after full initialization
            world->root->addChild(world->player2);
            
            game->camera2 = new Camera();
            game->camera2->lookAt(Vector3(0.f, 2.f, -5.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
            game->camera2->setPerspective(60.f, game->window_width/(float)game->window_height, 0.1f, 3000.f);

            // Create skybox2 for player 2
            if (!world->skybox2) {
                world->skybox2 = new EntityMesh(Mesh::Get("data/meshes/cubemap.ASE"), world->skybox->material);
            }
        } else {
            // Clean up player 2 and its camera
            if (world->player2) {
                // Stop any ongoing animations first
                world->player2->animator.stopAnimation();
                
                Player* temp_player = world->player2;
                world->player2 = nullptr;  // Set to null first to avoid any potential access
                world->root->removeChild(temp_player);
                delete temp_player;
            }
            if (game->camera2) {
                Camera* temp_camera = game->camera2;
                game->camera2 = nullptr;
                delete temp_camera;
            }
            if (world->skybox2) {
                EntityMesh* temp_skybox = world->skybox2;
                world->skybox2 = nullptr;
                delete temp_skybox;
            }
        }
    }
}

void MenuStage::init() {
    int width = Game::instance->window_width;
    int height = Game::instance->window_height;

    // Background
    Material background_mat;
    background_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    background_mat.diffuse = Texture::Get("data/textures/ui/menu.png");
    background_mat.color = Vector4(1,1,1,1);
    
    background = new EntityMesh();
    background->mesh = new Mesh();
    background->mesh->createQuad(width * 0.5f, height * 0.5f, width, height, true);
    background->material = background_mat;

    const float BUTTON_WIDTH = 160.0f;
    const float BUTTON_HEIGHT = BUTTON_WIDTH / (16.0f/9.0f);

    // Logo
    Material logo_mat;
    logo_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    logo_mat.diffuse = Texture::Get("data/textures/ui/logo.png");
    logo_mat.color = Vector4(1,1,1,1);
    
    logo = new EntityMesh();
    logo->mesh = new Mesh();
    float logo_size = height * 0.35f;
    logo->mesh->createQuad(width * 0.5f, height * 0.3f, logo_size, logo_size, true);
    logo->material = logo_mat;

    // Play button
    Material play_mat;
    play_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    play_mat.diffuse = Texture::Get("data/textures/ui/play_button.png");
    Texture* play_hover = Texture::Get("data/textures/ui/play_button_hover.png");
    play_mat.color = Vector4(1,1,1,1);
    
    play_button = new EntityMesh();
    play_button->mesh = new Mesh();
    play_button->mesh->createQuad(width * 0.5f, height * 0.55f, BUTTON_WIDTH, BUTTON_HEIGHT, true);
    play_button->material = play_mat;

    // Training button
    Material training_mat;
    training_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    training_mat.diffuse = Texture::Get("data/textures/ui/training_button.png");
    Texture* training_hover = Texture::Get("data/textures/ui/training_button_hover.png");
    training_mat.color = Vector4(1,1,1,1);
    
    training_button = new EntityMesh();
    training_button->mesh = new Mesh();
    training_button->mesh->createQuad(width * 0.5f, height * 0.7f, BUTTON_WIDTH, BUTTON_HEIGHT, true);
    training_button->material = training_mat;

    // Exit button
    Material exit_mat;
    exit_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    exit_mat.diffuse = Texture::Get("data/textures/ui/exit_button.png");
    Texture* exit_hover = Texture::Get("data/textures/ui/exit_button_hover.png");
    exit_mat.color = Vector4(1,1,1,1);
    
    exit_button = new EntityMesh();
    exit_button->mesh = new Mesh();
    exit_button->mesh->createQuad(width * 0.5f, height * 0.85f, BUTTON_WIDTH, BUTTON_HEIGHT, true);
    exit_button->material = exit_mat;

    // Guardar las texturas normales y hover
    play_normal = play_mat.diffuse;
    training_normal = training_mat.diffuse;
    exit_normal = exit_mat.diffuse;
    
    play_hover_tex = play_hover;
    training_hover_tex = training_hover;
    exit_hover_tex = exit_hover;

    background->addChild(logo);
    background->addChild(play_button);
    background->addChild(training_button);
    background->addChild(exit_button);
}

void MenuStage::render() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Camera* camera2D = World::get_instance()->camera2D;
    background->render(camera2D);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void MenuStage::update(double seconds_elapsed) {
    Vector2 mouse_pos = Input::mouse_position;
    float width = Game::instance->window_width;
    float height = Game::instance->window_height;
    
    const float BUTTON_WIDTH = 160.0f;
    const float BUTTON_HEIGHT = BUTTON_WIDTH / (16.0f/9.0f);
    float center_x = width * 0.5f;
    
    float play_y = height * 0.55f;
    bool mouse_over_play = mouse_pos.x >= (center_x - BUTTON_WIDTH/2) && 
                          mouse_pos.x <= (center_x + BUTTON_WIDTH/2) &&
                          mouse_pos.y >= (play_y - BUTTON_HEIGHT/2) && 
                          mouse_pos.y <= (play_y + BUTTON_HEIGHT/2);
    
    float training_y = height * 0.7f;
    bool mouse_over_training = mouse_pos.x >= (center_x - BUTTON_WIDTH/2) && 
                              mouse_pos.x <= (center_x + BUTTON_WIDTH/2) &&
                              mouse_pos.y >= (training_y - BUTTON_HEIGHT/2) && 
                              mouse_pos.y <= (training_y + BUTTON_HEIGHT/2);
    
    float exit_y = height * 0.85f;
    bool mouse_over_exit = mouse_pos.x >= (center_x - BUTTON_WIDTH/2) && 
                          mouse_pos.x <= (center_x + BUTTON_WIDTH/2) &&
                          mouse_pos.y >= (exit_y - BUTTON_HEIGHT/2) && 
                          mouse_pos.y <= (exit_y + BUTTON_HEIGHT/2);
    
    if (mouse_over_play) {
        play_button->material.diffuse = play_hover_tex;
        if (Input::wasMousePressed(SDL_BUTTON_LEFT)) {
            Game::instance->goToStage(STAGE_PLAY);
        }
    } else {
        play_button->material.diffuse = play_normal;
    }
    
    if (mouse_over_training) {
        training_button->material.diffuse = training_hover_tex;
        if (Input::wasMousePressed(SDL_BUTTON_LEFT)) {
            Game::instance->goToStage(STAGE_PLAY);
        }
    } else {
        training_button->material.diffuse = training_normal;
    }
    
    if (mouse_over_exit) {
        exit_button->material.diffuse = exit_hover_tex;
        if (Input::wasMousePressed(SDL_BUTTON_LEFT)) {
            Game::instance->must_exit = true;
        }
    } else {
        exit_button->material.diffuse = exit_normal;
    }
    
    background->update(seconds_elapsed);
}

void MenuStage::onEnter(Stage* prev_stage) {
    Game::instance->setMouseLocked(false);
}