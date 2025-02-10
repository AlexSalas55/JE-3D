#include "player.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include "game/game.h"
#include "game/world.h"

Player::Player(Mesh* mesh, const Material& material, const std::string& name)
    : EntityMesh(mesh, material)
{
    this->name = name;
    walk_speed = 3.0f;

    // Create sword for player
    /*sword = new EntityMesh(Mesh::Get("data/meshes/sword.obj"), material);
    addChild(sword);*/
}

void Player::update(float seconds_elapsed)
{
    float camera_yaw = World::get_instance()->camera_yaw;
    
    Matrix44 mYaw;
    mYaw.setRotation(camera_yaw, Vector3(0, 1, 0));
    
    Vector3 front = mYaw.frontVector();
    Vector3 right = mYaw.rightVector();
    Vector3 position = model.getTranslation();
    
    Vector3 move_dir;
    
    if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP))
        move_dir += front;
    if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN))
        move_dir -= front;
    if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT))
        move_dir += right;
    if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT))
        move_dir -= right;

    if (move_dir.length() > 0) {
        move_dir.normalize();
        move_dir = move_dir * walk_speed;
        velocity += move_dir;
    }

    // Actualizar posición
    position += velocity * seconds_elapsed;
    
    // Aplicar la nueva posición
    model.setTranslation(position.x, position.y, position.z);
    
    // Reducir velocidad
    velocity = velocity * 0.5f;

    model.rotate(camera_yaw, Vector3(0, 1, 0));

    // Update sword position if exists
    /*if (sword) {
        sword->model.setTranslation(1.0f, 0.35f, 0.14f);
        sword->model.rotate(PI * 0.33f, Vector3(0.0f, 1.0f, 0.0f));
    }*/

    EntityMesh::update(seconds_elapsed);
} 