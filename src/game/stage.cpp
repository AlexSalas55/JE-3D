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