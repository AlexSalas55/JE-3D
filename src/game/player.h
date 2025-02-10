#pragma once
#include "framework/entities/entityMesh.h"
#include "graphics/material.h"

class Player : public EntityMesh {
    float walk_speed = 1.0f;
    Vector3 velocity = Vector3(0.0f);
    EntityMesh* sword = nullptr;

public:
    Player() {};
    Player(Mesh* mesh, const Material& material, const std::string& name = "");
    void update(float seconds_elapsed) override;
	void testCollisions(const Vector3& target_position, float seconds_elapsed);
}; 