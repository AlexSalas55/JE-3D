#include "input.h"

const Uint8* Input::keystate = NULL;
Uint8 Input::prev_keystate[SDL_NUM_SCANCODES]; //previous before

//mouse state
int Input::mouse_state; //tells which buttons are pressed
int Input::prev_mouse_state; //tells which buttons were pressed
Vector2 Input::mouse_position; //last mouse position
Vector2 Input::mouse_delta; //mouse movement in the last frame
float Input::mouse_wheel;
float Input::mouse_wheel_delta;

//gamepad state
GamepadState Input::gamepads[4];

//internal
SDL_GameController* _controllers[4];
SDL_Window* window = NULL;

//************************************************

void Input::init(SDL_Window* _window)
{
	int x, y;
	window = _window;
	SDL_GetMouseState(&x, &y);
	Input::mouse_position.set((float)x, (float)y);
	Input::keystate = SDL_GetKeyboardState(NULL);
	mouse_wheel = 0.0;

	// Initialize game controllers
	for (int i = 0; i < 4; ++i)
	{
		_controllers[i] = openGamepad(i);
		updateGamepadState(_controllers[i], gamepads[i]);
	}
}

void Input::update()
{
	//read keyboard state and stored in keystate
	memcpy((void*)&Input::prev_keystate, Input::keystate, SDL_NUM_SCANCODES);
	Input::keystate = SDL_GetKeyboardState(NULL);

	prev_mouse_state = mouse_state;
	Input::mouse_delta.set(0.0f, 0.0f);

	//gamepads
	for (int i = 0; i < 4; ++i)
		updateGamepadState(_controllers[i], gamepads[i]);
}

SDL_GameController* Input::openGamepad(int index)
{
	// Check if the index is valid
	if (SDL_NumJoysticks() <= index)
		return NULL;

	// Check if it's a game controller
	if (!SDL_IsGameController(index)) {
		std::cout << "Joystick " << index << " is not a game controller" << std::endl;
		return NULL;
	}

	// Open the game controller
	SDL_GameController* controller = SDL_GameControllerOpen(index);
	if (controller) {
		std::cout << " * Game Controller found: " << SDL_GameControllerName(controller) << std::endl;
	} else {
		std::cout << "Could not open game controller " << index << ": " << SDL_GetError() << std::endl;
	}

	return controller;
}

void Input::updateGamepadState(SDL_GameController* controller, GamepadState& state)
{
	//save old state
	int prev_direction = state.direction;
	char prev_button[16];
	memcpy(prev_button, state.button, 16);

	//reset all gamepad state
	memset(&state, 0, sizeof(GamepadState));
	state.connected = false;

	if (controller == NULL)
		return;

	state.connected = true;
	state.model = SDL_GameControllerName(controller);

	// Update axis values
	state.axis[LEFT_ANALOG_X] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f;
	state.axis[LEFT_ANALOG_Y] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f;
	state.axis[RIGHT_ANALOG_X] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) / 32768.0f;
	state.axis[RIGHT_ANALOG_Y] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / 32768.0f;
	state.axis[TRIGGER_LEFT] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32768.0f;
	state.axis[TRIGGER_RIGHT] = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32768.0f;
	state.axis[TRIGGERS] = state.axis[TRIGGER_RIGHT] - state.axis[TRIGGER_LEFT];

	// Update button states
	state.button[A_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
	state.button[B_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
	state.button[X_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
	state.button[Y_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);
	state.button[LB_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
	state.button[RB_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
	state.button[BACK_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);
	state.button[START_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
	state.button[LEFT_ANALOG_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK);
	state.button[RIGHT_ANALOG_BUTTON] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK);

	memcpy(state.prev_button, prev_button, 16); //copy prev buttons state

	// Update direction state based on left analog stick
	Vector2 axis_direction(state.axis[LEFT_ANALOG_X], state.axis[LEFT_ANALOG_Y]);
	state.prev_direction = prev_direction;
	state.direction = 0;
	float limit = 0.6f;
	if (axis_direction.x < -limit)
		state.direction |= PAD_LEFT;
	else if (axis_direction.x > limit)
		state.direction |= PAD_RIGHT;
	if (axis_direction.y < -limit)
		state.direction |= PAD_UP;
	else if (axis_direction.y > limit)
		state.direction |= PAD_DOWN;
}

void Input::setGamepadVibration(float low_freq_intensity, float high_freq_intensity, Uint32 duration_ms, int pad)
{
	if (pad < 0 || pad >= 4 || !_controllers[pad])
		return;

	// Convert 0-1 range to 0-65535 range
	Uint16 low = (Uint16)(low_freq_intensity * 65535.0f);
	Uint16 high = (Uint16)(high_freq_intensity * 65535.0f);
	
	SDL_GameControllerRumble(_controllers[pad], low, high, duration_ms);
}
