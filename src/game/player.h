#pragma once
#include "framework/entities/entityMesh.h"
#include "graphics/material.h"
#include "framework/animation.h"
#include "framework/audio.h"

enum eAnimationState {
    IDLE,
    MOVE,
    BRAKE,
    IMPULSE,
    JUMP,
    FALL,
    COLLISION,
    CELEBRATE
};

#define MAX_FALLING_SNOW 500  //constant for falling snow

struct FallingSnow {
    Vector3 position;
    float offset;      //cosine movement
    float speed;       
    float alpha;
    bool active;
};

class Player : public EntityMesh {
public:
    Player() {};
    Player(Mesh* mesh, const Material& material, const std::string& name = "");
	void render(Camera* camera) override;
    void update(float seconds_elapsed) override;
	void testCollisions(const Vector3& target_position, float seconds_elapsed);
    void setRecoveryPosition(const Vector3& pos) { recovery_position = pos; }
    void resetVelocity() { velocity = Vector3(0.0f); }
    float air_time = 0.0f;
    //members for falling snow
    FallingSnow falling_snow[MAX_FALLING_SNOW];
    void updateFallingSnow(float dt, const Vector3& camera_pos);
    void renderFallingSnow(Camera* camera);
    void spawnFallingSnowParticle(int index);

private:
    float walk_speed = 20.0f;
    Vector3 velocity = Vector3(0.0f);
    EntityMesh* sword = nullptr;

    //collision recovery system
    int collision_count = 0;
    double last_collision_time = 0.0;
    Vector3 recovery_position = Vector3(345.0f, 184.0f, 37.0f); //start position by default
    
    float slope_tolerance = 0.3f;
    float ground_friction = 0.1f; //smoother sliding
    float gravity = 1120.0f;
    bool is_grounded;
    Vector3 ground_normal;

    //animations
    eAnimationState animation_state = eAnimationState::IDLE;

    //movement variables
    float bounce_force = 5.0f;
    float jump_strength = 10.0f;
    float top_speed = 50.0f;
    float air_control_factor = 0.5f;
    float air_gravity_multiplier = 0.15f;
    float max_speed = 50.0f;          
    float acceleration = 5.0f;        
    float base_deceleration = 0.8f;    
    float uphill_deceleration = 5.0f;
    float current_speed = 0.0f;
    float last_speed = 0.0f;
    float last_acceleration_time = 0.0f;
    bool in_portal_fall = false;
    Vector3 portal_target = Vector3(367.0f, -762.0f, 228.0f);
    Matrix44 render_model; //for visual tilting

    //gravity related variables
    float gravity_force = 9.81f * 2.0f; 
    float terminal_velocity = -80.0f;    //maximum falling speed
    float vertical_velocity = 0.0f;     

    //rotation variables
    Quaternion target_rotation;
    float rotation_smoothing = 0.2f;    
    float min_rotation_angle = 0.01f;   
    float min_movement_speed = 0.1f; 

    //camera-specific variables
    float camera_yaw = 0.0f;
    float camera_pitch = 0.0f;
    float target_camera_angle = 0.0f;
    Vector3 eye;
    Vector3 center;
    float camera_smooth_factor = 0.01f;
    float rotation_speed = 2.0f;
    float camera_speed = 10.0f;
    float mouse_speed = 0.25f;

    //player state
    float uphill_timer = 0.0f;

    //helper methods for movement and slope calculations
    void updateCameraPitch(float target_angle);
    Vector3 calculateSlopeDirection() const;
    float calculateSlopeFactor() const;
    void handleUphillMovement(float seconds_elapsed, float slope_factor);
    void handleSlopeMovement(float seconds_elapsed, const Vector3& world_direction, const Vector3& slope_direction);

    //audio variables
    HCHANNEL move_sound_channel = 0;
    HCHANNEL brake_sound_channel = 0;
    HCHANNEL wind_sound_channel = 0;
    bool is_move_sound_playing = false;
    bool is_brake_sound_playing = false;
    bool is_wind_sound_playing = false;
};