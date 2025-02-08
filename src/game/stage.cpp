#include "stage.h"
#include "world.h"
#include "game.h"

void PlayStage::init() {
    // Initialization code if needed
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