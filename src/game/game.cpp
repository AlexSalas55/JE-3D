#include "game.h"
#include "framework/utils.h"
#include "framework/entities/entityMesh.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/fbo.h"
#include "graphics/shader.h"
#include "framework/audio.h"
#include "framework/input.h"
#include "stage.h"
#include "world.h"

#include <cmath>

//some globals
Mesh* mesh = NULL;
Texture* texture = NULL;
Shader* shader = NULL;
float angle = 0;
float mouse_speed = 100.0f;

Game* Game::instance = NULL;

EntityMesh* entity_mesh = nullptr;

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;
	current_stage = nullptr;

	// Create and setup camera first
	camera = new Camera();
	camera->lookAt(Vector3(0.f, 2.f, -5.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(65.f, window_width/(float)window_height, 0.1f, 3000.f);

	// OpenGL flags
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Stages
	stages[STAGE_MENU] = new MenuStage();
	stages[STAGE_PLAY] = new PlayStage();
	
	for (auto entry : stages) {
		Stage* stage = entry.second;
		stage->init();
	}

	// inicializar audio
	if (!Audio::Init()) {
		std::cout << "Failed to initialize audio system!" << std::endl;
	}


	
	goToStage(STAGE_MENU);

	// Load one texture using the Texture Manager
	texture = Texture::Get("data/textures/texture.tga");

	// Example of loading Mesh from Mesh Manager
	mesh = Mesh::Get("data/meshes/box.ASE");

	// Example of shader loading using the shaders manager
	shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");

	Material material;
	material.diffuse = texture;
	material.shader = shader;
	material.color = Vector4::RED;

	entity_mesh = new EntityMesh(mesh, material);
	
	// Hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

void Game::render(void)
{
	if (current_stage)
	{
		current_stage->render();
	}

	// Render the FPS, Draw Calls, etc
	drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);
}

void Game::doFrame(void)
{
	// Set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (multiplayer_enabled && camera2 && World::get_instance()->player2)
	{
	    // Set aspect ratio for split screen (half width)
	    float split_aspect = (window_width * 0.5f) / window_height;
	    camera->aspect = split_aspect;
	    camera2->aspect = split_aspect;

	    // Left viewport (Player 1)
	    glViewport(0, 0, window_width/2, window_height);
	    render();

	    // Right viewport (Player 2)
	    glViewport(window_width/2, 0, window_width/2, window_height);
	    render();
	}
	else
	{
	    // Full viewport for single player
	    camera->aspect = window_width / (float)window_height;
	    glViewport(0, 0, window_width, window_height);
	    render();
	}

	// Swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
}

void Game::update(double seconds_elapsed)
{
	if (current_stage) {
		current_stage->update(seconds_elapsed);
	}
}

void Game::goToStage(uint8_t stage_id)
{
	auto it = stages.find(stage_id);
	if (it == stages.end()) {
		std::cerr << "Stage " << (int)stage_id << " not found!" << std::endl;
		return;
	}

	Stage* new_stage = it->second;
	if (!new_stage) {
		std::cerr << "New stage is null!" << std::endl;
		return;
	}

	if (current_stage) {
		current_stage->onLeave(new_stage);
	}
	
	new_stage->onEnter(current_stage);
	current_stage = new_stage;
}

void Game::setMouseLocked(bool must_lock)
{
	mouse_locked = must_lock;
	SDL_ShowCursor(!mouse_locked);
	SDL_SetRelativeMouseMode((SDL_bool)mouse_locked);
}

//Keyboard event handler (sync input)
void Game::onKeyDown(SDL_KeyboardEvent event)
{
	if (current_stage) {
		current_stage->onKeyDown(event);
	}

	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break;
		case SDLK_F1: Shader::ReloadAll(); break;
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
	if (current_stage) {
		current_stage->onKeyUp(event);
	}
}

void Game::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (current_stage) {
		current_stage->onMouseButtonDown(event);
	}

	if (event.button == SDL_BUTTON_MIDDLE)
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
		SDL_SetRelativeMouseMode((SDL_bool)mouse_locked);
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
	if (current_stage) {
		current_stage->onMouseButtonUp(event);
	}
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	if (current_stage) {
		current_stage->onMouseWheel(event);
	}
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{
	if (current_stage) {
		current_stage->onGamepadButtonDown(event);
	}
}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{
	if (current_stage) {
		current_stage->onGamepadButtonUp(event);
	}
}

void Game::onResize(int width, int height)
{
    // Enforce minimum size
    width = std::max(width, 1280);
    height = std::max(height, 720);
    
    std::cout << "window resized: " << width << "," << height << std::endl;
    window_width = width;
    window_height = height;

    // Update camera aspect ratios based on split-screen state
    if (multiplayer_enabled && camera2) {
        float split_aspect = (width * 0.5f) / (float)height;
        camera->aspect = split_aspect;
        if (camera2) camera2->aspect = split_aspect;
    } else {
        camera->aspect = width / (float)height;
    }

    if (current_stage) {
        current_stage->onResize(width, height);
    }
}

