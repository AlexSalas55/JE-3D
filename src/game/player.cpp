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
    //last_acceleration_time = 0.0f;

    // Create sword for player
    /*sword = new EntityMesh(Mesh::Get("data/meshes/sword.obj"), material);
    addChild(sword);*/
}

void Player::render(Camera* camera)
{
	EntityMesh::render(camera);


    Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    Mesh* mesh = Mesh::Get("data/meshes/sphere.obj");
	Matrix44 m = getGlobalMatrix();

    shader->enable();

    {
        m.translate(0.0f, World::get_instance()->player_height, 0.0f);
        m.scale(World::get_instance()->sphere_radius, World::get_instance()->sphere_radius, World::get_instance()->sphere_radius);

        shader->setUniform("u_color", Vector4(0.0f, 1.0f, 0.0f, 1.0f));
        shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
        shader->setUniform("u_model", m);

        mesh->render(GL_LINES);
    }

    {
		m = getGlobalMatrix();
		m.translate(0.0f, World::get_instance()->sphere_grow, 0.0f);
		m.scale(World::get_instance()->sphere_grow, World::get_instance()->sphere_grow, World::get_instance()->sphere_grow);
		shader->setUniform("u_color", Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		shader->setUniform("u_model", m);
		mesh->render(GL_LINES);
    }


    shader->disable();
}


void Player::update(float seconds_elapsed)
{
    float camera_yaw = World::get_instance()->camera_yaw;

    Matrix44 mYaw;
    mYaw.setRotation(camera_yaw, Vector3(0, 1, 0));

    Vector3 front = mYaw.frontVector();
    Vector3 right = mYaw.rightVector();
    Vector3 position = model.getTranslation();

    //Get ground normal from collision test
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;
    World::get_instance()->test_scene_collisions(position, collisions, ground_collisions, eCollisionFilter::ALL);

    Vector3 ground_normal = Vector3::UP;
    bool is_grounded = false;

    //Find ground normal
    for (const sCollisionData& collision : ground_collisions) {
        float up_factor = fabs(collision.colNormal.dot(Vector3::UP));
        if (up_factor > 0.8) {
            is_grounded = true;
            ground_normal = collision.colNormal;
            break;
        }
    }

    //Calc slope angle
    float slope_angle = acos(ground_normal.dot(Vector3::UP));
    float slope_factor = sin(slope_angle); //How steep the slope is (0 to 1)

    //Calc forward direction based on slope
    Vector3 slope_direction = (Vector3::UP - ground_normal * (Vector3::UP.dot(ground_normal))).normalize();
    
    //Acceleration when pressing W
    if (Input::isKeyPressed(SDL_SCANCODE_W) && is_grounded) {
        current_speed += acceleration * seconds_elapsed;
        current_speed = std::min(current_speed, max_speed);
    }
    
    //Apply slope effects
    if (is_grounded) {
        //Base acceleration when on flat ground
        float base_acceleration = acceleration;
        
        //Increased slope acceleration for better downhill response
        float slope_acceleration = slope_factor * 50.0f;
        
        //Add extra downhill acceleration when slope is steep
        if (slope_factor > 0.3f) { //If slope steep enough
            slope_acceleration *= 1.5f; //downhill acceleration
        }
        
        //Calculate final acceleration based on slope and input
        if (Input::isKeyPressed(SDL_SCANCODE_W)) {
            current_speed += (base_acceleration + slope_acceleration) * seconds_elapsed;
        } else {
            //Always apply slope acceleration when going downhill
            if (slope_factor > 0) {
                current_speed += slope_acceleration * seconds_elapsed;
            }
            
            //Apply deceleration
            if (slope_factor < 0) { // Going uphill
                current_speed -= uphill_deceleration * seconds_elapsed;
            } else {
                current_speed -= base_deceleration * seconds_elapsed * 0.5f; // Reduced deceleration
            }
        }

        //Clamp speed with higher max speed for steep slopes
        float slope_max_speed = max_speed;
        if (slope_factor > 0.3f) {
            slope_max_speed *= (1.0f + slope_factor); //Increase max speed on steep slopes
        }
        
        current_speed = std::max(0.0f, current_speed);
        current_speed = std::min(current_speed, slope_max_speed);

        //Apply movement with stronger slope influence
        Vector3 move_direction = (front + slope_direction * 1.2f).normalize(); // Increased slope influence
        velocity = move_direction * current_speed;
        
        //Store current velocity magnitude for preserving momentum
        float speed = velocity.length();
        if (speed > 0.1f) {
            current_speed = speed;
        }
            
    }

    //collision handling code
    testCollisions(position, seconds_elapsed);

    //Update position and rotation
    position += velocity * seconds_elapsed;
    model.setTranslation(position.x, position.y, position.z);
    model.rotate(camera_yaw, Vector3(0, 1, 0));

    //Print current speed on screen
    std::string speed_text = "Speed: " + std::to_string(current_speed);

    //if speed has changed more than 0.5f prints it
    if (fabs(current_speed - last_speed) > 0.5f) {
        std::cout << speed_text << std::endl;
        last_speed = current_speed;
    }

    //print timer
    //std::string timer_text = "Time: " + std::to_string(World::get_instance()->time);
    //std::cout << timer_text << std::endl;

    EntityMesh::update(seconds_elapsed);
}

void Player::testCollisions(const Vector3& target_position, float seconds_elapsed)
{
    //comprobar colisiones con las entidades del mundo
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;

    float ground_height = 0.0f; //Variable para almacenar la altura del suelo
    World::get_instance()->test_scene_collisions(target_position, collisions, ground_collisions, eCollisionFilter::ALL);

    bool is_grounded = false;
    Vector3 ground_normal;

    //ground collisions
    for (const sCollisionData& collision : ground_collisions) {
        float up_factor = fabs(collision.colNormal.dot(Vector3::UP));
        if (up_factor > 0.8) {
            is_grounded = true;
            ground_height = collision.colPoint.y; // Almacenar la altura del suelo
            ground_normal = collision.colNormal; // Almacenar la normal del suelo
        }
    }

    
    /*
    //collisions with objects
    for (const sCollisionData& collision : collisions){
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
	*/
    
    
    //collisions with objects
    for (const sCollisionData& collision : collisions) {
        float up_factor = collision.colNormal.dot(Vector3::UP);
        if (fabs(up_factor) <= 0.1f) {
            std::cout << "Collision detected at: " << collision.colPoint << " with normal: " << collision.colNormal << std::endl;
        }
        //Only handle wall collisions
        if (fabs(up_factor) > 0.1f) {
            continue;
        }
        //Reduce speed if collision
        velocity *= 0.2f;

        //Adjust position to move out of collision object
        Vector3 offset = collision.colNormal * 0.1f;
        model.setTranslation(model.getTranslation() + offset);
    }
    
        

    //gravity
    if (!is_grounded) {
        //Preserve horizontal momentum when in air
        Vector3 horizontal_velocity = Vector3(velocity.x, 0, velocity.z);
        //Apply gravity acceleration to vertical velocity
        vertical_velocity -= gravity_force * seconds_elapsed;
        vertical_velocity = std::max(vertical_velocity, terminal_velocity);

        //Combine horizontal momentum with vertical velocity
        velocity = horizontal_velocity + Vector3(0, vertical_velocity, 0);
    } else {
        vertical_velocity = 0.0f;

        //Only apply slope physics when grounded
        Vector3 gravity(0, -gravity_force, 0);
        Vector3 acceleration = gravity - (ground_normal * gravity.dot(ground_normal));

        float slope_dot = ground_normal.dot(Vector3::UP);
        if (slope_dot < 0.9f && current_speed > 5.0f) {
            Vector3 launch_dir = velocity.normalize();
            launch_dir.y = 1.0f - slope_dot;
            velocity = launch_dir * current_speed;
            is_grounded = false; //set the skier flying
        }
    }
    //Update pos
    Vector3 updated_position = target_position + velocity * seconds_elapsed;
    model.setTranslation(updated_position.x, updated_position.y, updated_position.z);
}