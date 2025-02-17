#pragma once
#include "framework/entities/entityMesh.h"
#include "graphics/material.h"

class Player : public EntityMesh {
    float walk_speed = 5.0f;
    Vector3 velocity = Vector3(0.0f);
    EntityMesh* sword = nullptr;

    // New skiing variables
    float max_speed = 20.0f;
    float acceleration = 8.0f;
    float base_deceleration = 3.5f;
    float uphill_deceleration = 12.0f;
    float current_speed = 0.0f;
    float last_speed = 0.0f;
    float last_acceleration_time = 0.0f;

    // Gravity related variables
    float gravity_force = 9.81f * 2.0f; // Base gravity force
    float terminal_velocity = -50.0f;    // Maximum falling speed
    float vertical_velocity = 0.0f;      // Current vertical velocity

    Quaternion target_rotation;
    float rotation_smoothing = 0.2f;    // Increased from 0.1f
    float min_rotation_angle = 0.01f;   // Minimum angle for rotation to occur
    float min_movement_speed = 0.1f;    // Minimum speed for direction-based rotation
        

public:
    Player() {};
    Player(Mesh* mesh, const Material& material, const std::string& name = "");
	void render(Camera* camera) override;
    void update(float seconds_elapsed) override;
	void testCollisions(const Vector3& target_position, float seconds_elapsed);
}; 