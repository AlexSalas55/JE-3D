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
    EntityMesh* skybox2 = nullptr;  // Second skybox for player 2
    Player* player = nullptr;
    Player* player2 = nullptr;  // Second player for multiplayer
    Camera* camera;      // 3D camera for player 1
    Camera* camera2D;    // 2D camera for UI
    Vector3 eye;
    Vector3 center;

    float camera_roll = 0.0f;
    float camera2_roll = 0.0f;
    
    // Player 2 camera variables
    float camera2_yaw = 0.f;
    float camera2_pitch = 0.f;
    Vector3 eye2;       // Camera position for player 2
    Vector3 center2;    // Look-at point for player 2

    double time = 0.f;
    float camera_yaw = 0.f;
    float camera_pitch = 0.f;
    float camera_speed = 10.0f;
    float mouse_speed = 0.25f;
    bool free_camera = false;
    bool use_first_person = false;
    float rotation_speed = 2.0f;
    
    float sphere_radius = 0.8f;
    float sphere_grow = .3f;
    float player_height = 1.5f;

    // Variables para almacenar la posición de la cámara
    Vector3 camera_stored_eye;
    Vector3 camera_stored_center;
    Vector3 camera_stored_up;

    // Phong lighting
    Shader* phong_shader;
    Vector3 light_position;
    Vector3 light_color;
    Vector3 light2_position;  // Second light position
    Vector3 light2_color;     // Second light color

    bool is_training_stage = true;  // Por defecto, asumimos que estamos en entrenamiento

    void render();
    void update(double seconds_elapsed);

    // Scene management
    std::vector<Entity*> entities_to_destroy;
    void addEntity(Entity* entity);
    void destroyEntity(Entity* entity);

	// Collision detection
	sCollisionData raycast(const Vector3& origin, const Vector3& direction, int layer = eCollisionFilter::ALL, bool closest = true, float max_ray_dist = 100000);
    void test_scene_collisions(const Vector3& target_position, std::vector<sCollisionData>& collisions, std::vector<sCollisionData>& ground_collisions, eCollisionFilter filter);

    // Camera collision handling
    Vector3 adjustCameraPosition(const Vector3& target_eye, const Vector3& target_center, float min_distance = 0.5f);
    bool isPointInsideMesh(const Vector3& point, float radius = 0.1f);
    
    // Método para cambiar entre escenario de entrenamiento y juego
    void setTrainingStage(bool is_training) { is_training_stage = is_training; }
};