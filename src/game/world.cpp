#include "world.h"
#include "framework/camera.h"
#include "framework/input.h"
#include "framework/utils.h"
#include "framework/entities/entity.h"
#include "framework/entities/entityMesh.h"
#include "graphics/shader.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "scene_parser.h"
#include "player.h"

World* World::instance = nullptr;

World::World() {
    camera = new Camera();
    
    // Instantiate parent root
    root = new Entity();

    // Create heightmap
    float size = 500.0f;
    Mesh* heightmap_mesh = new Mesh();
    heightmap_mesh->createPlane(size);

    Material heightmap_material;
    heightmap_material.shader = Shader::Get("data/shaders/heightmap.vs", "data/shaders/heightmap.fs");
    heightmap_material.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

    EntityMesh* heightmap = new EntityMesh(heightmap_mesh, heightmap_material);
    heightmap->model.translate(-size * 0.5f, 0.0f, -size * 0.5f);
    root->addChild(heightmap);

    // Create skybox environment
    std::vector<std::string> cubemap_files = {
        "data/textures/skybox/right.png",
        "data/textures/skybox/left.png",
        "data/textures/skybox/top.png",
        "data/textures/skybox/bottom.png",
        "data/textures/skybox/front.png",
        "data/textures/skybox/back.png"
    };

    Material cubemap_material;
    cubemap_material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/cubemap.fs");
    cubemap_material.diffuse = Texture::Get("data/textures/skybox"); // Asumiendo que el Texture Manager sabe cargar cubemaps
    cubemap_material.color = Vector4(1,1,1,1);
    
    skybox = new EntityMesh(Mesh::Get("data/meshes/cubemap.ASE"), cubemap_material);

    // Create player
    Material player_material;
    player_material.diffuse = Texture::Get("data/meshes/colormap.png");
    player_material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    player = new Player(Mesh::Get("data/meshes/soldier.obj"), player_material, "player");

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
    
    skybox->render(camera);
    
    glEnable(GL_DEPTH_TEST);
    
    // Draw the floor grid
    drawGrid();
    
    // Render all scene tree
    root->render(camera);
}

void World::update(double seconds_elapsed) {
    if (free_camera) {
        float speed = seconds_elapsed * camera_speed;
        
        // Async input to move the camera around
        if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) 
            speed *= 10;
        if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP))
            camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
        if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN))
            camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
        if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT))
            camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
        if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT))
            camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
    }
    else {
        // Update scene and player
        root->update(seconds_elapsed);
        player->update(seconds_elapsed);

        // Camera control
        camera_yaw += Input::mouse_delta.x * seconds_elapsed * mouse_speed;
        camera_pitch += Input::mouse_delta.y * seconds_elapsed * mouse_speed;

        // Restrict pitch angle
        camera_pitch = clamp(camera_pitch, -M_PI * 0.4f, M_PI * 0.4f);

        Matrix44 mYaw;
        mYaw.setRotation(camera_yaw, Vector3(0, 1, 0));
        Matrix44 mPitch;
        mPitch.setRotation(camera_pitch, Vector3(-1, 0, 0));

        Vector3 front = (mPitch * mYaw).frontVector().normalize();
        Vector3 eye, center;

        if (use_first_person) {
            eye = player->model.getTranslation() + Vector3(0.f, 0.5f, 0.0f) + front * 0.1f;
            center = eye + front;
        }
        else {
            float orbit_dist = 1.5f;
            eye = player->model.getTranslation() - front * orbit_dist;
            center = player->model.getTranslation() + Vector3(0.f, 0.5f, 0.0f);
        }

        camera->lookAt(eye, center, Vector3(0, 1, 0));
    }

    // Move skybox to camera position
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