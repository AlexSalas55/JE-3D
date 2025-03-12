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
    // instance world if not initialized
    if (!World::get_instance()->root) {
        World::get_instance();
    }

    // Initialize button positions and sizes
    Game* game = Game::instance;
    button_size = Vector2(160.0f, 90.0f);
    restart_button_pos = Vector2(game->window_width * 0.5f, game->window_height * 0.6f);
    menu_button_pos = Vector2(game->window_width * 0.5f, game->window_height * 0.7f);
}

void PlayStage::restart() {
    World* world = World::get_instance();
    Game* game = Game::instance;

    // reset all timers and states
    chronometer1 = 0.0;
    chronometer2 = 0.0;
    timer1_active = false;
    timer2_active = false;
    player1_finished = false;
    player2_finished = false;
    player1_checkpoint = false;
    player2_checkpoint = false;
    checkpoint1_time = 0.0;
    checkpoint2_time = 0.0;
    show_end_screen = false;
    player1_is_winner = false;


    // reset player positions
    Vector3 start_pos(345.0f, 184.0f, 37.0f);
    if (world->player) {
        world->player->model.setTranslation(start_pos.x, start_pos.y, start_pos.z);
        world->player->setRecoveryPosition(start_pos);

        // rotate player 1 180 degrees
        world->camera_yaw = M_PI;
    }

    if (world->player2) {
        world->player2->model.setTranslation(start_pos.x, start_pos.y, start_pos.z + 10.0f);
        world->player2->setRecoveryPosition(start_pos);

        // rotate player 2 180 degrees
        world->camera2_yaw = M_PI;
    }
}

void PlayStage::onEnter(Stage* prev_stage) {

    restart();

    // reset chronometers and states when entering stage
    chronometer1 = 0.0;
    chronometer2 = 0.0;
    timer1_active = false;
    timer2_active = false;
    player1_finished = false;
    player2_finished = false;

    // initialize player 2 if multiplayer is enabled
    Game* game = Game::instance;
    World* world = World::get_instance();

    // Establecer que NO estamos en el escenario de entrenamiento
    world->setTrainingStage(false);

    // rotate player 1 180 degrees
    world->camera_yaw = M_PI; // 180 degrees in radians
    
    // rotate player 2 180 degrees
    if (game->multiplayer_enabled) {
        world->camera2_yaw = M_PI;
    }
    
    if (game->multiplayer_enabled && !world->player2) {
        // create player 2 and its camera
        Material player_material;
        player_material.shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");
        player_material.diffuse = Texture::Get("data/meshes/playerColor.png");
        
        // create player2 with same mesh
        Mesh* player_mesh = world->player->mesh;
        world->player2 = new Player(player_mesh, player_material, "player2");
        
        // set position to same as player 1 +10.0f at z
        Vector3 start_pos(345.0f, 184.0f, 37.0f);
        world->player2->model.setTranslation(start_pos.x, start_pos.y, start_pos.z + 10.0f);
        world->player2->setRecoveryPosition(start_pos);
        
        // ensure animations are properly initialized before adding to scene
        world->player2->isAnimated = world->player->isAnimated;
        
        // initialize animation with same state as player1
        Animation* currentAnim = world->player->animator.getCurrentAnimation();
        if (currentAnim) {
            world->player2->animator.playAnimation(currentAnim->name.c_str(), true);
        } else {
            world->player2->animator.playAnimation("data/meshes/animations/idle.skanim", true);
        }
        
        // add to scene after full initialization
        world->root->addChild(world->player2);
        
        game->camera2 = new Camera();
        game->camera2->lookAt(Vector3(0.f, 2.f, -5.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
        game->camera2->setPerspective(60.f, game->window_width/(float)game->window_height, 0.1f, 3000.f);

        // create skybox2 for player 2
        if (!world->skybox2) {
            world->skybox2 = new EntityMesh(Mesh::Get("data/meshes/cubemap.ASE"), world->skybox->material);
        }
    }
    
    

    // lock/unlock cursor depending on free camera
    bool must_lock = !World::get_instance()->free_camera;
    Game::instance->setMouseLocked(must_lock);

    // play game music with BASS_SAMPLE_LOOP to repeat
    play_music_channel = Audio::Play("data/assets/audio/music/music_play1.wav", 0.085f, BASS_SAMPLE_LOOP);
}

void PlayStage::render() {
    World* world = World::get_instance();
    world->render();

    Game* game = Game::instance;
    int window_width = game->window_width;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // show active race timer (moved 100 pixels from edge)
    renderChronometer(chronometer1, window_width - 300, 30);
    
    // show checkpoint and final times
    int y_offset = 60;
    
    if (player1_checkpoint) {
        char p1_checkpoint[64];
        sprintf(p1_checkpoint, "P1 Checkpoint: %02d:%02d.%03d", 
            (int)(checkpoint1_time / 60.0),
            (int)(checkpoint1_time) % 60,
            (int)((checkpoint1_time - (int)checkpoint1_time) * 1000));
        drawText(window_width - 300, y_offset, p1_checkpoint, Vector3(0.5,1,0.5), 2);
        y_offset += 30;
    }
    
    if (player1_finished) {
        char p1_time[64];
        sprintf(p1_time, "P1 Final: %02d:%02d.%03d", 
            (int)(chronometer1 / 60.0),
            (int)(chronometer1) % 60,
            (int)((chronometer1 - (int)chronometer1) * 1000));
        drawText(window_width - 300, y_offset, p1_time, Vector3(1,1,0), 2);
        y_offset += 30;
    }
    
    if (game->multiplayer_enabled) {
        if (player2_checkpoint) {
            char p2_checkpoint[64];
            sprintf(p2_checkpoint, "P2 Checkpoint: %02d:%02d.%03d", 
                (int)(checkpoint2_time / 60.0),
                (int)(checkpoint2_time) % 60,
                (int)((checkpoint2_time - (int)checkpoint2_time) * 1000));
            drawText(window_width - 300, y_offset, p2_checkpoint, Vector3(0.5,0.5,1), 2);
            y_offset += 30;
        }
        
        if (player2_finished) {
            char p2_time[64];
            sprintf(p2_time, "P2 Final: %02d:%02d.%03d", 
                (int)(chronometer2 / 60.0),
                (int)(chronometer2) % 60,
                (int)((chronometer2 - (int)chronometer2) * 1000));
            drawText(window_width - 300, y_offset, p2_time, Vector3(0,1,1), 2);
        }

    }

    // render end screen if game is finished
    if (show_end_screen) {
        renderEndScreen();
    }
    
    glDisable(GL_BLEND);
}

void PlayStage::renderEndScreen() {
    Game* game = Game::instance;
    int window_width = game->window_width;
    int window_height = game->window_height;

    // semi-transparent background
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

    // title and times
    char title[128];
    Vector3 title_color;
    
    if (game->multiplayer_enabled) {
        if (player1_is_winner) {
            sprintf(title, "Player 1 Wins!");
            title_color = Vector3(1,1,0);
        } else {
            sprintf(title, "Player 2 Wins!");
            title_color = Vector3(0,1,1);
        }
    } else {
        sprintf(title, "Race Complete!");
        title_color = Vector3(1,1,1);
    }

    // draw title
    drawText(window_width * 0.5f - strlen(title) * 10, window_height * 0.35f, title, title_color, 3);

    // draw times
    char time_str[64];
    if (game->multiplayer_enabled) {
        sprintf(time_str, "Final Time: %02d:%02d.%03d", 
            (int)((player1_is_winner ? chronometer1 : chronometer2) / 60.0),
            (int)(player1_is_winner ? chronometer1 : chronometer2) % 60,
            (int)((player1_is_winner ? chronometer1 : chronometer2) - (int)(player1_is_winner ? chronometer1 : chronometer2)) * 1000);
    } else {
        sprintf(time_str, "Final Time: %02d:%02d.%03d", 
            (int)(chronometer1 / 60.0),
            (int)chronometer1 % 60,
            (int)((chronometer1 - (int)chronometer1) * 1000));
    }
    drawText(window_width * 0.5f - strlen(time_str) * 8, window_height * 0.45f, time_str, Vector3(1,1,1), 2);

    // show checkpoint time
    if (game->multiplayer_enabled) {
        if ((player1_is_winner && player1_checkpoint) || (!player1_is_winner && player2_checkpoint)) {
            sprintf(time_str, "Checkpoint: %02d:%02d.%03d", 
                (int)((player1_is_winner ? checkpoint1_time : checkpoint2_time) / 60.0),
                (int)(player1_is_winner ? checkpoint1_time : checkpoint2_time) % 60,
                (int)((player1_is_winner ? checkpoint1_time : checkpoint2_time) - (int)(player1_is_winner ? checkpoint1_time : checkpoint2_time)) * 1000);
            drawText(window_width * 0.5f - strlen(time_str) * 8, window_height * 0.5f, time_str, Vector3(0.5,1,0.5), 2);
        }
    } else if (player1_checkpoint) {
        sprintf(time_str, "Checkpoint: %02d:%02d.%03d", 
            (int)(checkpoint1_time / 60.0),
            (int)checkpoint1_time % 60,
            (int)((checkpoint1_time - (int)checkpoint1_time) * 1000));
        drawText(window_width * 0.5f - strlen(time_str) * 8, window_height * 0.5f, time_str, Vector3(0.5,1,0.5), 2);
    }

    // draw instructions
    drawText(window_width * 0.5f - 120, window_height * 0.6f, "Press ENTER to restart", Vector3(1,1,1), 2);
    drawText(window_width * 0.5f - 120, window_height * 0.65f, "Press DELETE to exit", Vector3(1,1,1), 2);
}

void PlayStage::update(double seconds_elapsed) {
    World* world = World::get_instance();
    Game* game = Game::instance;
    
    // checkpoint and finish line positions
    Vector3 checkpoint_pos(367.0f, -762.0f, 228.0f);
    Vector3 finish_line_pos(-330.057f, -1408.09f, -667.831f);
    float checkpoint_threshold = 15.0f;
    float finish_threshold = 20.0f;
    
    // player 1 logic
    Vector3 player1_pos = world->player->model.getTranslation();
    Vector3 start_pos(345.0f, 184.0f, 37.0f);
    if (!player1_finished) {
        if (!timer1_active && (player1_pos - start_pos).length() > 0.1f) {
            timer1_active = true;
        }
        
        if (timer1_active) {
            chronometer1 += seconds_elapsed;
        }
        
        if (!player1_checkpoint && (player1_pos - checkpoint_pos).length() < checkpoint_threshold) {
            checkpoint1_time = chronometer1;
            player1_checkpoint = true;
            world->player->setRecoveryPosition(checkpoint_pos);
        }
        
        if ((player1_pos - finish_line_pos).length() < finish_threshold) {
            timer1_active = false;
            player1_finished = true;
            if (!game->multiplayer_enabled) {
                show_end_screen = true;
                chronometer1 = chronometer1;  // Save final time
            } else if (!player2_finished) {
                player1_is_winner = true;
                show_end_screen = true;
            }
        }
    }
    
    if (game->multiplayer_enabled && world->player2) {
        Vector3 player2_pos = world->player2->model.getTranslation();
        if (!player2_finished) {
            if (!timer2_active && (player2_pos - start_pos).length() > 0.1f) {
                timer2_active = true;
            }
            
            if (timer2_active) {
                chronometer2 += seconds_elapsed;
            }
            
            if (!player2_checkpoint && (player2_pos - checkpoint_pos).length() < checkpoint_threshold) {
                checkpoint2_time = chronometer2;
                player2_checkpoint = true;
                world->player2->setRecoveryPosition(checkpoint_pos);
            }
            
            if ((player2_pos - finish_line_pos).length() < finish_threshold) {
                timer2_active = false;
                player2_finished = true;
                if (!player1_finished) {
                    player1_is_winner = false;
                    show_end_screen = true;
                }
            }
        }
    }

    //if air_time > 3.0f, restart play stage
    if (world->player->air_time > 3.0f) {
        restart();
        world->player->air_time = 0.0f;
    }
    
    world->update(seconds_elapsed);
}

void PlayStage::renderChronometer(double time, int x, int y) {
    // Format time
    int minutes = (int)(time / 60.0);
    int seconds = (int)(time) % 60;
    int milliseconds = (int)((time - (int)time) * 1000);
    
    char time_str[32];
    sprintf(time_str, "%02d:%02d.%03d", minutes, seconds, milliseconds);
    
    // draw text
    drawText(x-1, y, time_str, Vector3(0,0,0), 2);
    drawText(x+1, y, time_str, Vector3(0,0,0), 2);
    drawText(x, y-1, time_str, Vector3(0,0,0), 2);
    drawText(x, y+1, time_str, Vector3(0,0,0), 2);
    drawText(x, y, time_str, Vector3(1,1,1), 2);
}

void PlayStage::onKeyDown(SDL_KeyboardEvent event) {
    if (show_end_screen) {
        if (Input::wasKeyPressed(SDL_SCANCODE_BACKSPACE)) {
            Game* game = Game::instance;
            World* world = World::get_instance();
            // clean up player 2 if enabled
            if (game->multiplayer_enabled) {
                if (world->player2) {
                    // stop animations
                    world->player2->animator.stopAnimation();
                    Player* temp_player = world->player2;
                    world->player2 = nullptr;  // set to null first to avoid any potential access
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
                // disable multiplayer mode
                game->multiplayer_enabled = false;
            }
            // reset and restart game
            Game::instance->goToStage(STAGE_MENU);
            show_end_screen = false;
        }
        else if (Input::wasKeyPressed(SDL_SCANCODE_RETURN)) {
            Game::instance->goToStage(STAGE_PLAY);
            show_end_screen = false;
        }
        return;
    }

    if (event.keysym.scancode == SDL_SCANCODE_P) {
        Game* game = Game::instance;
        World* world = World::get_instance();
        
        // toggle multiplayer mode
        game->multiplayer_enabled = !game->multiplayer_enabled;
        
        if (game->multiplayer_enabled) {
            // create player 2 and its camera
            Material player_material;
            player_material.shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");
            player_material.diffuse = Texture::Get("data/meshes/playerColor.png");
            
            // create player2 with same mesh
            Mesh* player_mesh = world->player->mesh;
            world->player2 = new Player(player_mesh, player_material, "player2");

            //set position 
            world->player2->model.setTranslation(world->player->model.m[12], world->player->model.m[13], world->player->model.m[14] + 10.0f);

            
            // ensure animation initialized
            world->player2->isAnimated = world->player->isAnimated;
            
            // initialize animation
            Animation* currentAnim = world->player->animator.getCurrentAnimation();
            if (currentAnim) {
                world->player2->animator.playAnimation(currentAnim->name.c_str(), true);
            } else {
                world->player2->animator.playAnimation("data/meshes/animations/idle.skanim", true);
            }
            
            // add to scene
            world->root->addChild(world->player2);
            
            game->camera2 = new Camera();
            game->camera2->lookAt(Vector3(0.f, 2.f, -5.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
            game->camera2->setPerspective(60.f, game->window_width/(float)game->window_height, 0.1f, 3000.f);

            // create skybox
            if (!world->skybox2) {
                world->skybox2 = new EntityMesh(Mesh::Get("data/meshes/cubemap.ASE"), world->skybox->material);
            }
        } else {
            // clean up player 2 and its camera
            if (world->player2) {
                // stop animations
                world->player2->animator.stopAnimation();
                
                Player* temp_player = world->player2;
                world->player2 = nullptr;  // set to null first to avoid any potential access
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

    // background
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

    // logo
    Material logo_mat;
    logo_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    logo_mat.diffuse = Texture::Get("data/textures/ui/logo.png");
    logo_mat.color = Vector4(1,1,1,1);
    
    logo = new EntityMesh();
    logo->mesh = new Mesh();
    float logo_size = height * 0.35f;
    logo->mesh->createQuad(width * 0.5f, height * 0.3f, logo_size, logo_size, true);
    logo->material = logo_mat;

    // play button
    Material play_mat;
    play_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    play_mat.diffuse = Texture::Get("data/textures/ui/play_button.png");
    Texture* play_hover = Texture::Get("data/textures/ui/play_button_hover.png");
    play_mat.color = Vector4(1,1,1,1);
    
    play_button = new EntityMesh();
    play_button->mesh = new Mesh();
    play_button->mesh->createQuad(width * 0.5f - BUTTON_WIDTH * 0.6f, height * 0.55f, BUTTON_WIDTH, BUTTON_HEIGHT, true);
    play_button->material = play_mat;

    // multiplayer button
    Material multiplayer_mat;
    multiplayer_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    multiplayer_mat.diffuse = Texture::Get("data/textures/ui/play_multi_button.png");
    Texture* multiplayer_hover = Texture::Get("data/textures/ui/play_multi_button_hover.png");
    multiplayer_mat.color = Vector4(1,1,1,1);
    
    multiplayer_button = new EntityMesh();
    multiplayer_button->mesh = new Mesh();
    multiplayer_button->mesh->createQuad(width * 0.5f + BUTTON_WIDTH * 0.6f, height * 0.55f, BUTTON_WIDTH, BUTTON_HEIGHT, true);
    multiplayer_button->material = multiplayer_mat;

    // training button
    Material training_mat;
    training_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    training_mat.diffuse = Texture::Get("data/textures/ui/training_button.png");
    Texture* training_hover = Texture::Get("data/textures/ui/training_button_hover.png");
    training_mat.color = Vector4(1,1,1,1);
    
    training_button = new EntityMesh();
    training_button->mesh = new Mesh();
    training_button->mesh->createQuad(width * 0.5f, height * 0.7f, BUTTON_WIDTH, BUTTON_HEIGHT, true);
    training_button->material = training_mat;

    // exit button
    Material exit_mat;
    exit_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    exit_mat.diffuse = Texture::Get("data/textures/ui/exit_button.png");
    Texture* exit_hover = Texture::Get("data/textures/ui/exit_button_hover.png");
    exit_mat.color = Vector4(1,1,1,1);
    
    exit_button = new EntityMesh();
    exit_button->mesh = new Mesh();
    exit_button->mesh->createQuad(width * 0.5f, height * 0.85f, BUTTON_WIDTH, BUTTON_HEIGHT, true);
    exit_button->material = exit_mat;

    // save normal and hover textures
    play_normal = play_mat.diffuse;
    multiplayer_normal = multiplayer_mat.diffuse;
    training_normal = training_mat.diffuse;
    exit_normal = exit_mat.diffuse;
    
    play_hover_tex = play_hover;
    multiplayer_hover_tex = multiplayer_hover;
    training_hover_tex = training_hover;
    exit_hover_tex = exit_hover;

    background->addChild(logo);
    background->addChild(play_button);
    background->addChild(multiplayer_button);
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

    // updated positions for parallel buttons
    float play_x = center_x - BUTTON_WIDTH * 0.6f;
    
    float play_y = height * 0.55f;
    bool mouse_over_play = mouse_pos.x >= (play_x - BUTTON_WIDTH/2) && 
                          mouse_pos.x <= (play_x + BUTTON_WIDTH/2) &&
                          mouse_pos.y >= (play_y - BUTTON_HEIGHT/2) && 
                          mouse_pos.y <= (play_y + BUTTON_HEIGHT/2);

    float multiplayer_x = center_x + BUTTON_WIDTH * 0.6f;
    float multiplayer_y = height * 0.55f;
    bool mouse_over_multiplayer = mouse_pos.x >= (multiplayer_x - BUTTON_WIDTH/2) && 
                                 mouse_pos.x <= (multiplayer_x + BUTTON_WIDTH/2) &&
                                 mouse_pos.y >= (multiplayer_y - BUTTON_HEIGHT/2) && 
                                 mouse_pos.y <= (multiplayer_y + BUTTON_HEIGHT/2); 
    
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
            Game::instance->multiplayer_enabled = false; // ensure multiplayer is disabled
            Game::instance->goToStage(STAGE_PLAY);
        }
    } else {
        play_button->material.diffuse = play_normal;
    }

    if (mouse_over_multiplayer) {
        multiplayer_button->material.diffuse = multiplayer_hover_tex;
        if (Input::wasMousePressed(SDL_BUTTON_LEFT)) {
            Game::instance->multiplayer_enabled = true; // activate multiplayer
            Game::instance->goToStage(STAGE_PLAY);
        }
    } else {
        multiplayer_button->material.diffuse = multiplayer_normal;
    }
    
    if (mouse_over_training) {
        training_button->material.diffuse = training_hover_tex;
        if (Input::wasMousePressed(SDL_BUTTON_LEFT)) {
            Game::instance->goToStage(STAGE_TRAINING);
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
    
    // play menu music
    menu_music_channel = Audio::Play("data/assets/audio/music/music_menu.wav", 0.4f, BASS_SAMPLE_LOOP);
}

void MenuStage::onLeave(Stage* next_stage) {
    // stop menu music
    if (menu_music_channel) {
        Audio::Stop(menu_music_channel);
        menu_music_channel = 0;
    }
}

void PlayStage::onLeave(Stage* next_stage) {
    // stop game music
    if (play_music_channel) {
        Audio::Stop(play_music_channel);
        play_music_channel = 0;
    }
}

////////////////////////////////////// TrainingStage //////////////////////////////////////

void TrainingStage::init() {
    // ensure world is initialized
    if (!World::get_instance()->root) {
        World::get_instance();
    }
    
    Game* game = Game::instance;
    int width = game->window_width;
    int height = game->window_height;
    
    // create tutorial image
    Material tutorial_mat;
    tutorial_mat.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    tutorial_mat.diffuse = Texture::Get("data/textures/ui/tutorial.png");
    tutorial_mat.color = Vector4(1,1,1,1);
    
    tutorial_background = new EntityMesh();
    tutorial_background->mesh = new Mesh();
    tutorial_background->mesh->createQuad(width * 0.5f, height * 0.5f, width * 0.8f, height * 0.8f, true);
    tutorial_background->material = tutorial_mat;
}

void TrainingStage::restart() {
    World* world = World::get_instance();
    
    // initial position for training
    Vector3 start_pos(1151.01f, -189.161f, 525.752f);
    
    if (world->player) {
        world->player->model.setTranslation(start_pos.x, start_pos.y, start_pos.z);
        world->player->setRecoveryPosition(start_pos);
        world->player->resetVelocity(); // reset velocity
    }
    
    // show tutorial
    show_tutorial = true;
}

void TrainingStage::onEnter(Stage* prev_stage) {    
    Game* game = Game::instance;
    World* world = World::get_instance();

    // set that we are in the training stage
    world->setTrainingStage(true);
    
    // show tutorial
    show_tutorial = true;
    
    // unlock mouse to interact with the interface
    game->setMouseLocked(false);
    
    // position player at the training start point
    Vector3 start_pos(1151.01f, -189.161f, 525.752f);
    if (world->player) {
        world->player->model.setTranslation(start_pos.x, start_pos.y, start_pos.z);
        world->player->setRecoveryPosition(start_pos);
        world->player->resetVelocity(); // Resetear la velocidad a cero
    }
    
    // play training music
    training_music_channel = Audio::Play("data/assets/audio/music/music_training.wav", 0.4f, BASS_SAMPLE_LOOP);
}

void TrainingStage::render() {
    // first render the world
    World* world = World::get_instance();
    world->render();
    
    // configuration for 2D rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // show tutorial image if active
    if (show_tutorial && tutorial_background) {
        Camera* camera2D = world->camera2D;
        tutorial_background->render(camera2D);
        
        // show message to indicate how to close the tutorial
        Game* game = Game::instance;
        drawText(game->window_width * 0.5f - 150, game->window_height * 0.9f, "Press SPACE to continue", Vector3(1,1,1), 2);
    } else {
        // show HUD if not in tutorial
        drawText(20, 30, "Training Mode", Vector3(1,1,0), 2);
        drawText(20, 60, "R - Return to starting position", Vector3(1,1,1), 2);
        drawText(20, 90, "ENTER - Return to main menu", Vector3(1,1,1), 2);
        drawText(20, 120, "SPACE - Show tutorial", Vector3(1,1,1), 2);
    }
    
    // restore configuration
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

void TrainingStage::update(double seconds_elapsed) {
    World* world = World::get_instance();
    world->update(seconds_elapsed);
}

void TrainingStage::onKeyDown(SDL_KeyboardEvent event) {
    World* world = World::get_instance();
    
    // toggle tutorial
    if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) {
        show_tutorial = !show_tutorial;
    }
    
    // return to initial position
    if (Input::wasKeyPressed(SDL_SCANCODE_R)) {
        Vector3 start_pos(1151.01f, -189.161f, 525.752f);
        if (world->player) {
            world->player->model.setTranslation(start_pos.x, start_pos.y, start_pos.z);
            world->player->setRecoveryPosition(start_pos);
            world->player->resetVelocity(); // Resetear la velocidad a cero
        }
    }
    
    // return to main menu
    if (Input::wasKeyPressed(SDL_SCANCODE_RETURN)) {
        Game::instance->goToStage(STAGE_MENU);
    }
}

void TrainingStage::onLeave(Stage* next_stage) {
    // stop training music
    if (training_music_channel) {
        Audio::Stop(training_music_channel);
        training_music_channel = 0;
    }
}