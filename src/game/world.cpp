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
    
    // Create 2D camera for UI
    camera2D = new Camera();
    camera2D->setOrthographic(0, Game::instance->window_width, Game::instance->window_height, 0, -1, 1);
    
    // Create root entity
    root = new Entity();

    // Create and setup player
    Material player_material;
    // player_material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    player_material.shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");
	player_material.diffuse = Texture::Get("data/meshes/playerColor.png");
    // player = new Player(Mesh::Get("data/meshes/soldier.obj"), player_material, "player");
    player = new Player(Mesh::Get("data/meshes/player.mesh"), player_material, "player");

    //player->model.setTranslation(240.0f, 200.0f, -10.0f); //mapa pepino
    //player->model.setTranslation(20.0f, 250.0f, 0.0f); //old map
    //player->model.setTranslation(0.0f, 200.0f, 0.0f); //mapa pepino

    player->model.setTranslation(00.0f, 200.0f, 0.0f); //mapa pepino



    root->addChild(player);
    
    {
        // Create heightmap

        // Create heightmap - reducir el tamaño
        float size = 2500.0f;  // Cambiado de 500.0f a 50.0f

        Mesh* heightmap_mesh = new Mesh();
        heightmap_mesh->createSubdividedPlane(size, 256);

        Material heightmap_material = {
            Shader::Get("data/shaders/height_map.vs", "data/shaders/height_map.fs"),
            Vector4(1.0f, 1.0f, 1.0f, 1.0f),
            Texture::Get("data/textures/heightmap.png")
        };

        // EntityMesh* heightmap = new EntityMesh(heightmap_mesh, heightmap_material);
        // heightmap->model.translate(-size * 0.5f, 0.0f, -size * 0.5f);
        // root->addChild(heightmap);
    }
    
    {
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
        // skybox->model.scale(5000.0f, 5000.0f, 5000.0f); //Para escalar el mapa? TODO

    }

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
    glDisable(GL_DEPTH_TEST);
    
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

    // Toggle free camera with C key
    if (Input::wasKeyPressed(SDL_SCANCODE_C)) {
        free_camera = !free_camera;
        if (free_camera) {
            // Store current camera position and orientation when entering free camera
            camera_stored_eye = camera->eye;
            camera_stored_center = camera->center;
            camera_stored_up = camera->up;
        } else {
            // Restore camera position when exiting free camera mode
            camera->lookAt(camera_stored_eye, camera_stored_center, camera_stored_up);
        }
    }

    if (free_camera) {
        float speed = seconds_elapsed * camera_speed;
        
        // Async input to move the camera around
        if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) 
            speed *= 10;

        Vector3 forward = (camera->center - camera->eye).normalize();
        Vector3 right = forward.cross(camera->up).normalize();
        Vector3 up = Vector3(0, 1, 0);

        // Update camera movement based on its orientation (WASD y flechas)
        if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP))
            camera->eye = camera->eye + forward * speed;
        if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN))
            camera->eye = camera->eye - forward * speed;
        if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT))
            camera->eye = camera->eye - right * speed;
        if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT))
            camera->eye = camera->eye + right * speed;
        if (Input::isKeyPressed(SDL_SCANCODE_Q))
            camera->eye = camera->eye + up * speed;
        if (Input::isKeyPressed(SDL_SCANCODE_E))
            camera->eye = camera->eye - up * speed;

        // Mouse look - invertimos el movimiento vertical (pitch)
        camera_yaw -= Input::mouse_delta.x * 0.005f;
        camera_pitch += Input::mouse_delta.y * 0.005f; // Cambiado el signo aquí
        
        // Limit pitch to avoid camera flipping
        camera_pitch = clamp(camera_pitch, -M_PI * 0.4f, M_PI * 0.4f);
        
        // Calculate new front vector
        Vector3 front;
        front.x = cos(camera_yaw) * cos(camera_pitch);
        front.y = sin(camera_pitch);
        front.z = sin(camera_yaw) * cos(camera_pitch);
        front = front.normalize();
        
        camera->center = camera->eye + front;
        camera->updateViewMatrix();
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

    // move skybox to player
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