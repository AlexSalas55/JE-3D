#include "player.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include "game/game.h"
#include "game/world.h"
#include "framework/entities/entity_collider.h"

Player::Player(Mesh* mesh, const Material& material, const std::string& name)
    : EntityMesh(mesh, material)
{
    this->name = name;
    walk_speed = 5.0f;

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

    // Comprobar colisiones con las entidades del mundo
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;

	testCollisions(position, seconds_elapsed);

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


void Player::testCollisions(const Vector3& target_position, float seconds_elapsed)
{
    // Comprobar colisiones con las entidades del mundo
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;

    float ground_height = 0.0f; // Variable para almacenar la altura del suelo
    World::get_instance()->test_scene_collisions(target_position, collisions, ground_collisions, eCollisionFilter::ALL);

    for (const sCollisionData& collision : collisions) {
		//if normal points up, it's a ground collision
        float up_factor = collision.colNormal.dot(Vector3::UP);
		if (up_factor > 0.8) {
			continue;
		}
        //Move along wall when colliding
        Vector3 newDir = velocity.dot(collision.colNormal) * collision.colNormal;
        velocity.x -= newDir.x;
        //velocity.y -= newDir.y;
        velocity.z -= newDir.z;
    }

	bool is_grounded = false;
    Vector3 ground_normal;

    for (const sCollisionData& collision : ground_collisions) {
		float up_factor = fabs(collision.colNormal.dot(Vector3::UP));
        if (up_factor > 0.8) {
            is_grounded = true;
            ground_height = collision.colPoint.y; // Almacenar la altura del suelo
            ground_normal = collision.colNormal; // Almacenar la normal del suelo

        }
    }

    //gravity
	if (!is_grounded) {
		velocity.y -= 600.0f * seconds_elapsed;
	} else {
        //Ajustar la posición del jugador para que esté justo encima del suelo
		velocity.y = ground_height + target_position.y;
        //Vector3 gravity(0, -9.8f, 0);
		//Vector3 gravity_dir = gravity.normalize();
        Vector3 gravity(0, -100.8f, 0);
        Vector3 acceleration = gravity - (ground_normal * gravity.dot(ground_normal));
        velocity += acceleration * seconds_elapsed;
		Vector3 forward_acceleration = Vector3(velocity.x, 0, velocity.z) * acceleration.length();
        velocity += forward_acceleration * seconds_elapsed;
    }
    // Actualizar posición
    Vector3 updated_position = target_position + velocity * seconds_elapsed;
    // Aplicar la nueva posición
    model.setTranslation(updated_position.x, updated_position.y, updated_position.z);
    // Reducir velocidad
    velocity = velocity * 0.5f;
}
