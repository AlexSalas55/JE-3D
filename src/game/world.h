#pragma once
#include "framework/utils.h"
#include "framework/entities/entity.h"
#include "graphics/mesh.h"

class Camera;
class Entity;
class EntityMesh;
class Player;
class EntityCollider;


class World {
    static World* instance;

public:
    static World* get_instance() {
        if (instance == nullptr)
            instance = new World();
        return instance;
    }

    World();
	
    Entity* root = nullptr;
    EntityMesh* skybox = nullptr;
    Player* player = nullptr;
    Camera* camera = nullptr;
    Vector3 eye;
    Vector3 center;

    double time = 0.f;
    float camera_yaw = 0.f;
    float camera_pitch = 0.f;
    float camera_speed = 2.0f;
    float mouse_speed = 0.25f;
    bool free_camera = false;
    bool use_first_person = false;
    
    float sphere_radius = 0.8f;
    float sphere_grow = .3f;
    float player_height = 1.5f;

    void render();
    void update(double seconds_elapsed);

    // Scene management
    std::vector<Entity*> entities_to_destroy;
    void addEntity(Entity* entity);
    void destroyEntity(Entity* entity);

	// Collision detection
	sCollisionData raycast(const Vector3& origin, const Vector3& direction, int layer = eCollisionFilter::ALL, bool closest = true, float max_ray_dist = 100000);
    void test_scene_collisions(const Vector3& target_position, std::vector<sCollisionData>& collisions, std::vector<sCollisionData>& ground_collisions, eCollisionFilter filter);
}; 