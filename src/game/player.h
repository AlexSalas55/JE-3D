#pragma once
#include "framework/entities/entityMesh.h"
#include "graphics/material.h"
#include "framework/animation.h"

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

class Player : public EntityMesh {
    float walk_speed = 20.0f;
    Vector3 velocity = Vector3(0.0f);
    EntityMesh* sword = nullptr;

    float slope_tolerance = 0.3f;
    float ground_friction = 0.1f; //smoother sliding
    float gravity = 1120.0f;
    bool is_grounded;
    Vector3 ground_normal;

    // Animations
    eAnimationState animation_state = eAnimationState::IDLE;

    float bounce_force = 5.0f;

    float jump_strength = 10.0f;

    float top_speed = 50.0f;

    float air_control_factor = 0.5f;
    float air_gravity_multiplier = 0.15f;
    float air_time = 0.0f;

    float max_speed = 50.0f;          
    float acceleration = 5.0f;        
    float base_deceleration = 0.8f;    
    float uphill_deceleration = 5.0f;
    float current_speed = 0.0f;
    float last_speed = 0.0f;
    float last_acceleration_time = 0.0f;

    bool in_portal_fall = false;
    //Vector3 portal_target = Vector3(360.614f, -761.45f, 238.256f);
    Vector3 portal_target = Vector3(367.0f, -762.0f, 228.0f);

    Matrix44 render_model; // For visual tilting


    // Gravity related variables
    float gravity_force = 9.81f * 2.0f; // Gravity force
    float terminal_velocity = -80.0f;    // Maximum falling speed
    float vertical_velocity = 0.0f;      // Current vertical velocity

    Quaternion target_rotation;
    float rotation_smoothing = 0.2f;    //Controls how fast the player aligns with slope
    float min_rotation_angle = 0.01f;   //Minimum angle for rotation to occur
    float min_movement_speed = 0.1f;    //Minimum speed for direction-based rotation
    // Camera-specific variables
    // Player state
    float uphill_timer = 0.0f;
    float camera_yaw = 0.0f;
    float camera_pitch = 0.0f;
    float target_camera_angle = 0.0f;
    Vector3 eye;
    Vector3 center;
    float camera_smooth_factor = 0.01f;
    
    // Camera control parameters
    float rotation_speed = 2.0f;
    float camera_speed = 10.0f;
    float mouse_speed = 0.25f;

    
    // Helper methods for movement and slope calculations
    void updateCameraPitch(float target_angle);
    Vector3 calculateSlopeDirection() const;
    float calculateSlopeFactor() const;
    void handleUphillMovement(float seconds_elapsed, float slope_factor);
    void handleSlopeMovement(float seconds_elapsed, const Vector3& world_direction, const Vector3& slope_direction);
    

public:
    Player() {};
    Player(Mesh* mesh, const Material& material, const std::string& name = "");
	void render(Camera* camera) override;
    void update(float seconds_elapsed) override;
	void testCollisions(const Vector3& target_position, float seconds_elapsed);

private:
    /*
    float camera_pitch = 0.0f;  // Each player's own camera pitch
    float target_camera_angle = 0.0f;  // Target angle for camera transitions
*/

};