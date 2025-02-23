#pragma once
#include "framework/entities/entityMesh.h"
#include "graphics/material.h"

class Player : public EntityMesh {
    float walk_speed = 20.0f;
    Vector3 velocity = Vector3(0.0f);
    EntityMesh* sword = nullptr;

    float slope_tolerance = 0.3f;
    float ground_friction = 0.1f; //smoother sliding
    float gravity = 1120.0f;
    bool is_grounded;
    Vector3 ground_normal;

    float bounce_force = 5.0f;

    float jump_strength = 10.0f;

    float top_speed = 90.0f;

    float air_control_factor = 0.5f;
    float air_gravity_multiplier = 10.25f;

    float max_speed = 80.0f;          
    float acceleration = 8.0f;        
    float base_deceleration = 0.8f;    
    float uphill_deceleration = 2000.0f;
    float current_speed = 0.0f;
    float last_speed = 0.0f;
    float last_acceleration_time = 0.0f;

    // Gravity related variables
    float gravity_force = 9.81f * 2.0f; // Gravity force
    float terminal_velocity = -80.0f;    // Maximum falling speed
    float vertical_velocity = 0.0f;      // Current vertical velocity

    Quaternion target_rotation;
    float rotation_smoothing = 0.2f;    //Controls how fast the player aligns with slope
    float min_rotation_angle = 0.01f;   //Minimum angle for rotation to occur
    float min_movement_speed = 0.1f;    //Minimum speed for direction-based rotation
    

public:
    Player() {};
    Player(Mesh* mesh, const Material& material, const std::string& name = "");
	void render(Camera* camera) override;
    void update(float seconds_elapsed) override;
	void testCollisions(const Vector3& target_position, float seconds_elapsed);
};