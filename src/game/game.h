/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#pragma once

#include "framework/includes.h"
#include "framework/camera.h"
#include "framework/utils.h"
#include "stage.h"
#include <map>

class Game
{
public:
	static Game* instance;

	//window
	SDL_Window* window;
	int window_width;
	int window_height;

	//some globals
	long frame;
    float time;
	float elapsed_time;
	int fps;
	bool must_exit;

	//some vars
	Camera* camera; //our global camera
	Camera* camera2; //second player camera
	bool mouse_locked; //tells if the mouse is locked (not seen)
	bool multiplayer_enabled = false; //flag for split-screen mode

	// Stage management
	std::map<uint8_t, Stage*> stages;
	Stage* current_stage;
	void goToStage(uint8_t stage_id);

	Game( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void doFrame( void ); //new method for split-screen rendering
	void update( double dt );

	void setMouseLocked(bool must_lock);

	//events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onGamepadButtonDown(SDL_JoyButtonEvent event);
	void onGamepadButtonUp(SDL_JoyButtonEvent event);
	void onResize(int width, int height);
};