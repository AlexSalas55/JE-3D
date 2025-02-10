#pragma once
#include "framework/utils.h"

class Camera;
class Entity;
class EntityMesh;
class Player;

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

    float camera_yaw = 0.f;
    float camera_pitch = 0.f;
    float camera_speed = 2.0f;
    float mouse_speed = 0.25f;
    bool free_camera = false;
    bool use_first_person = true;

    void render();
    void update(double seconds_elapsed);

    // Scene management
    std::vector<Entity*> entities_to_destroy;
    void addEntity(Entity* entity);
    void destroyEntity(Entity* entity);
}; 