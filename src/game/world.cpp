#include "world.h"
#include "framework/camera.h"
#include "framework/input.h"
#include "framework/utils.h"
#include "framework/entities/entity.h"
#include "framework/entities/entityMesh.h"
#include "framework/entities/entity_collider.h"
#include "graphics/shader.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "scene_parser.h"
#include "player.h"
#include "game.h"

World* World::instance = nullptr;

World::World() {
    // Get camera from Game
    camera = Game::instance->camera;
    
    // Create root entity
    root = new Entity();

    // Create and setup player
    Material player_material;
    player_material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	player_material.diffuse = Texture::Get("data/meshes/playerColor.png");
    player = new Player(Mesh::Get("data/meshes/soldier.obj"), player_material, "player");
    player->model.setTranslation(0.0f, 250.0f, 0.0f); //mapa pepino
    //player->model.setTranslation(20.0f, 250.0f, 0.0f); //old map

    root->addChild(player);

    // Create heightmap - reducir el tamaño
    float size = 0.0f;  // Cambiado de 500.0f a 50.0f
    Mesh* heightmap_mesh = new Mesh();
    heightmap_mesh->createPlane(size);

    Material heightmap_material;
    heightmap_material.shader = Shader::Get("data/shaders/heightmap.vs", "data/shaders/heightmap.fs");
    heightmap_material.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

    EntityMesh* heightmap = new EntityMesh(heightmap_mesh, heightmap_material);
    heightmap->model.translate(-size * 0.5f, 0.0f, -size * 0.5f);
    root->addChild(heightmap);

    // Create skybox environment
    Texture* cube_texture = new Texture();
    cube_texture->loadCubemap("landscape", {
        "data/textures/skybox/right.png",
        "data/textures/skybox/left.png",
        "data/textures/skybox/bottom.png",
        "data/textures/skybox/top.png",
        "data/textures/skybox/front.png",
        "data/textures/skybox/back.png"
    });

    Material cubemap_material;
    cubemap_material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/cubemap.fs");
    cubemap_material.diffuse = cube_texture;
    
    skybox = new EntityMesh(Mesh::Get("data/meshes/cubemap.ASE"), cubemap_material);

    //timer
    time = 0.0f;

    // Load scene
    SceneParser parser;
    bool ok = parser.parse("data/myscene.scene", root);
    assert(ok);
}

void World::render() {
    // Set the camera as default
    camera->enable();
    
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    
    if(skybox)
       skybox->render(camera);
    
    glEnable(GL_DEPTH_TEST);
    
    // Draw the floor grid
    drawGrid();
    
    // Render all scene tree
    root->render(camera);
}

void World::update(double seconds_elapsed) {
    time += seconds_elapsed;

    if (free_camera) {
        float speed = seconds_elapsed * camera_speed;
        
        // Async input to move the camera around
        if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) 
            speed *= 10;
        if (Input::isKeyPressed(SDL_SCANCODE_W))
            camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
        if (Input::isKeyPressed(SDL_SCANCODE_S))
            camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
        if (Input::isKeyPressed(SDL_SCANCODE_A))
            camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
        if (Input::isKeyPressed(SDL_SCANCODE_D))
            camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
    }
    else {
        // Update player first
        player->update(seconds_elapsed);

        // Reemplazar el control del ratón con las flechas
        float rotation_speed = 1.5f; // Velocidad de rotación
        if (Input::isKeyPressed(SDL_SCANCODE_LEFT))
            camera_yaw -= seconds_elapsed * rotation_speed;
        if (Input::isKeyPressed(SDL_SCANCODE_RIGHT))
            camera_yaw += seconds_elapsed * rotation_speed;
        if (Input::isKeyPressed(SDL_SCANCODE_DOWN))
            camera_pitch -= seconds_elapsed * rotation_speed;
        if (Input::isKeyPressed(SDL_SCANCODE_UP))
            camera_pitch += seconds_elapsed * rotation_speed;

        // Mantener el pitch dentro de límites
        camera_pitch = clamp(camera_pitch, -M_PI * 0.4f, M_PI * 0.4f);

        Matrix44 mYaw;
        mYaw.setRotation(camera_yaw, Vector3(0, 1, 0));
        Matrix44 mPitch;
        mPitch.setRotation(camera_pitch, Vector3(-1, 0, 0));

        Vector3 front = (mPitch * mYaw).frontVector().normalize();
        Vector3 player_pos = player->model.getTranslation();

        if (use_first_person) {
            // Simplificar la lógica de primera persona
            Vector3 camera_height = Vector3(0.0f, 1.5f, 0.0f);
            eye = player_pos + camera_height;
            center = eye + front;
        }
        else {
            float orbit_dist = 6.0f;  // Aumentado de 1.0f a 3.0f para alejar la cámara
            eye = player_pos - front * orbit_dist + Vector3(0.0f, 1.5f, 0.0f); // Aumentada la altura de 0.5f a 1.5f
            center = player_pos + Vector3(0.f, 0.8f, 0.0f); // Ajustado el punto de mira
        }

        camera->lookAt(eye, center, Vector3(0, 1, 0));
    }

    // Comentamos la actualización del skybox
    skybox->model.setTranslation(camera->eye);

    // Delete pending entities
    for (Entity* entity : entities_to_destroy) {
        if (entity->parent)
            entity->parent->removeChild(entity);
        delete entity;
    }
    entities_to_destroy.clear();
}


void World::addEntity(Entity* entity) {
    root->addChild(entity);
}

void World::destroyEntity(Entity* entity) {
    entities_to_destroy.push_back(entity);
}

//raycast
sCollisionData World::raycast(const Vector3& origin, const Vector3& direction, int layer, bool closest, float max_ray_dist) {
	sCollisionData collision;
	//collision.distance = max_ray_dist;

    for (auto e : root->children) {

        EntityCollider* ec = dynamic_cast<EntityCollider*>(e);
        if (ec == nullptr || !(ec->getLayer() & layer)) {
            continue;
        }

        Vector3 col_point;
        Vector3 col_normal;

        if (!ec->mesh->testRayCollision(ec->model, origin, direction,
            col_point, col_normal, max_ray_dist)) {
            continue;
        }

        // There was a collision! Update if nearest..
        float new_distance = (col_point - origin).length();
        if (new_distance < collision.distance) {
            collision = { col_point, col_normal, new_distance, true, ec };
        }

        if (!closest) {
            return collision;
        }

    }
	return collision;
}

void World::test_scene_collisions(const Vector3& target_position, std::vector<sCollisionData>& collisions, std::vector<sCollisionData>& ground_collisions, eCollisionFilter filter)
{
	for (auto e : root->children)
	{
        EntityCollider* ec = dynamic_cast<EntityCollider*>(e);
        if (ec == nullptr) {
            continue;
        }
        ec->getCollisions(target_position, collisions, ground_collisions, filter);
	}
}