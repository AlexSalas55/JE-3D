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
    // Animations
    isAnimated = true;
    
    // Initialize animator with idle animation but don't play it yet
    // This ensures the skeleton is properly initialized
    Animation* idle = Animation::Get("data/meshes/animations/idle.skanim");
    if (idle) {
        animator.playAnimation("data/meshes/animations/idle.skanim", true);
    }
    
    // Create sword for player (commented out for now)
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
    float camera_yaw = (this == World::get_instance()->player2) ? 
        World::get_instance()->camera2_yaw : World::get_instance()->camera_yaw;
    Matrix44 mYaw;
    mYaw.setRotation(camera_yaw, Vector3(0, 1, 0));
    Vector3 front = mYaw.frontVector().normalize();
    Vector3 right = mYaw.rightVector().normalize();
    Vector3 position = model.getTranslation();

    // if (position.z >= 269.0f) { //trigger to reach the end of the map
    //     std::cout << "Skier has reached the finish line! Position: " << position.z << std::endl;
    //     //reset player
    //     position = Vector3(0.0f, 200.0f, 0.0f);
    //     model.setTranslation(position.x, position.y, position.z);
    //     current_speed = 0.0f;
    //     velocity = Vector3(0.0f);
    //     vertical_velocity = 0.0f;

    // }

    //if 1 is pressed spawn the player in a specific position
    if (Input::isKeyPressed(SDL_SCANCODE_1)) {
        position = Vector3(6.0f, 200.0f, 64.0f);
        model.setTranslation(position.x, position.y, position.z);
        current_speed = 0.0f;
        velocity = Vector3(0.0f);
        vertical_velocity = 0.0f;

    }
    //if 2 is pressed spawn the player in map2
    if (Input::isKeyPressed(SDL_SCANCODE_2)) {
        position = Vector3(250.0f, 200.0f, -25.0f);
        model.setTranslation(position.x, position.y, position.z);
        current_speed = 0.0f;
        velocity = Vector3(0.0f);
        vertical_velocity = 0.0f;

    }

    //debug position
    //std::cout << "Skier Position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
    //Ground detection (handling collisions and ground check)
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;
    World::get_instance()->test_scene_collisions(position, collisions, ground_collisions, eCollisionFilter::ALL);

    is_grounded = false;

    // Find ground normal and update grounded state
    for (const sCollisionData& collision : ground_collisions) {
        float up_factor = collision.colNormal.dot(Vector3::UP);
        if (up_factor > slope_tolerance) {
            is_grounded = true;
            ground_normal = collision.colNormal;
            position.y = collision.colPoint.y;
            break;
        }
    }

    if (is_grounded) {
        float slope_angle = acos(ground_normal.dot(Vector3::UP));
        Vector3 slope_direction = (Vector3::UP - ground_normal * (Vector3::UP.dot(ground_normal))).normalize();

        Vector3 move_direction(0.0f);
        // Different movement controls for each player
        if (this == World::get_instance()->player2) {
            if (Input::isKeyPressed(SDL_SCANCODE_U)) move_direction.z = 1.0f;
        } else {
            if (Input::isKeyPressed(SDL_SCANCODE_W)) move_direction.z = 1.0f;
        }

        if (move_direction.length() > 0.01f) {
            //move based on user input
            move_direction.normalize();
            Vector3 world_direction;
            world_direction.x = move_direction.x * right.x + move_direction.z * front.x;
            world_direction.z = move_direction.x * right.z + move_direction.z * front.z;
    
            //movement onto the slope
            Vector3 slope_move = (world_direction - ground_normal * world_direction.dot(ground_normal)).normalize();
    
            //calculate slope factor
            float slope_factor = sin(slope_angle);
            float current_acceleration = acceleration;
    
            if (slope_factor > 0) { //Downhill acceleration
                velocity += -slope_direction * gravity_force * slope_factor * seconds_elapsed;

                //Calculate the slope angle in degrees
                float slope_angle_deg = slope_factor * RAD2DEG;
                //desired camera angle based on the slope
                float target_camera_angle = slope_angle_deg * 0.02f;
                target_camera_angle = clamp(target_camera_angle, 0.45f, 5.0f);
                //Smooth interpolation
                float camera_smooth_factor = 0.01f;
                //Interpolation between camera pitch and target camera angle
                float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
                //Update camera pitch
                World::get_instance()->camera_pitch = camera_angle;
                
                // Also update Player 2's camera pitch if this is Player 1
                if (this == World::get_instance()->player) {
                    World::get_instance()->camera2_pitch = camera_angle;
                }
                
                // Also update Player 2's camera pitch if this is Player 1
                if (this == World::get_instance()->player) {
                    World::get_instance()->camera2_pitch = camera_angle;
                }

            } else if (slope_factor < 0) { //Uphill Deceleration
                current_acceleration -= uphill_deceleration * abs(slope_factor);
            }
    
            //acceleration to speed
            current_speed += current_acceleration * seconds_elapsed;
            //current_speed = clamp(current_speed, 0.0f, max_speed);
    
            //update velocity
            velocity = slope_move * current_speed;


        }
        else {
            //When no input let gravity and friction will affect speed
            Vector3 camera_direction = front;
            camera_direction.y = 0.0f; //Ignore the vertical component
    
            //Project camera forward dir onto the ground
            Vector3 slope_move = (camera_direction - ground_normal * camera_direction.dot(ground_normal)).normalize();
    
            //Calculate slope factor
            float slope_factor = sin(slope_angle);

    
            //apply gravity(only downhill)
            if (slope_factor > 0.01f) { 

                //velocity += -slope_direction * gravity_force * slope_factor * seconds_elapsed;
                velocity += -slope_direction * (gravity_force * slope_factor * seconds_elapsed);
                //increase velocity over time
                current_speed += acceleration * seconds_elapsed;
                //current_speed = clamp(current_speed, 0.0f, max_speed); 
                
                //velocity = velocity.normalize() * current_speed;            
            }
            else if (slope_factor < 0) {
                //Apply uphill deceleration based on slope
                //current_speed *= (1.0f - uphill_deceleration * seconds_elapsed);
                //current_speed -= uphill_deceleration * slope_factor;

                //current_speed = clamp(current_speed, current_speed, 0.0f); 
				//current speed 0.0f
				//current_speed = std::max(current_speed, 0.0f);
                
                //disabled now for testing
                //current_speed += -uphill_deceleration * seconds_elapsed;
				//velocity -= -slope_direction * gravity_force * slope_factor * seconds_elapsed * uphill_deceleration;

                //decrease speed over time but direction on slope
                
            } else { //Flat ground (Decelerate gradually)
                current_speed *= (1.0f - base_deceleration * seconds_elapsed);

                //Calculate the slope angle in degrees
                float slope_angle_deg = slope_factor * RAD2DEG;
                //desired camera angle based on the slope
                float target_camera_angle = slope_angle_deg * 0.005f;
                target_camera_angle = clamp(target_camera_angle, 0.015f, 5.0f);
                //Smooth interpolation
                float camera_smooth_factor = 0.01f;
                //Interpolation between camera pitch and target camera angle
                float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
                //Update camera pitch
                World::get_instance()->camera_pitch = camera_angle;
                // Only update Player 2's camera pitch if this is Player 1
                if (this == World::get_instance()->player) {
                    World::get_instance()->camera2_pitch = camera_angle;
                }
                // Only update Player 2's camera pitch if this is Player 1
                if (this == World::get_instance()->player && Game::instance->multiplayer_enabled) {
                    World::get_instance()->camera2_pitch = camera_angle;
                }
            }


            //Calculate the slope angle in degrees
            float slope_angle_deg = slope_factor * RAD2DEG;
            //desired camera angle based on the slope
            float target_camera_angle = slope_angle_deg * 0.02f;
            target_camera_angle = clamp(target_camera_angle, 0.45f, 5.0f);
            //Smooth interpolation
            float camera_smooth_factor = 0.01f;
            //Interpolation between  camera pitch and target camera angle
            float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
            //Update camera pitch
            World::get_instance()->camera_pitch = camera_angle;
            // Only update Player 2's camera pitch if multiplayer is enabled
                if (this == World::get_instance()->player && Game::instance->multiplayer_enabled) {
                    World::get_instance()->camera2_pitch = camera_angle;
                }

            //Normalize velocity to current speed
            if (velocity.length() > 0) {
                velocity = velocity.normalize() * current_speed;
            }
    
            //Update velocity based on slope-movement direction and speed
            velocity = slope_move * current_speed;
            //velocity = velocity + slope_move * current_speed;
            //velocity = (velocity - ground_normal * velocity.dot(ground_normal)).normalize() * current_speed + (-slope_direction * gravity_force * sin(slope_angle) * seconds_elapsed);
        }
        
        //Apply friction dynamically on ground to reduce speed over time
        float effective_friction = ground_friction;
        float slope_factor = sin(slope_angle);

        if (slope_factor > 0) {
            effective_friction *= (1.0f - slope_factor * 0.5f); //downhill less Friction

        } else if (slope_factor < 0) {
            //a lot more when uphill
            effective_friction *= (1.0f + abs(slope_factor) * 0.5f); //uphill more friction
        }
        //effective friction based on the slope
        current_speed *= (1.0f - effective_friction * seconds_elapsed);
    }
    

    //air physics
    if (!is_grounded) {
        //gravity in the correct direction
        Vector3 gravity_direction = Vector3::UP - ground_normal * (ground_normal.dot(Vector3::UP));
        gravity_direction.normalize();
        
        //Increase gravity to make the fall faster
        velocity.y -= gravity * air_gravity_multiplier * seconds_elapsed;
        velocity += gravity_direction * gravity_force * seconds_elapsed;

        //push the skier forward
        velocity += front * current_speed * seconds_elapsed;

        //terminal velocity to prevent floating
        velocity.y = std::max(velocity.y, terminal_velocity);


        float slope_angle = acos(ground_normal.dot(Vector3::UP));
        float slope_factor = sin(slope_angle);
        //Calculate the slope angle in degrees
        float slope_angle_deg = slope_factor * RAD2DEG;
        //desired camera angle based on the slope
        float target_camera_angle = slope_angle_deg * 4.0f;
        target_camera_angle = clamp(target_camera_angle, 0.0f, 1.0f);
        //Smooth interpolation
        float camera_smooth_factor = 0.01f;
        //Interpolation between  camera pitch and target camera angle
        float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
        //Update camera pitch
        World::get_instance()->camera_pitch = camera_angle;
        // Only update Player 2's camera pitch if this is Player 1
        if (this == World::get_instance()->player) {
            World::get_instance()->camera2_pitch = camera_angle;
        }
    }
    //Update position and check collisions
    Vector3 desired_position = position + velocity * seconds_elapsed;
    testCollisions(desired_position, seconds_elapsed);
    model.rotate(camera_yaw, Vector3(0, 1, 0));


    //snapping the player Y pos to ground (if on ground)
    if (is_grounded) {
        position.y = ground_normal.y;
    }
    
    
    Vector3 forward = model.frontVector();
    Vector3 new_forward = (forward - ground_normal * forward.dot(ground_normal)).normalize();
    model.setFrontAndOrthonormalize(new_forward);
    

    // Check if skier is facing uphill
    Vector3 slope_direction = (Vector3::UP - ground_normal * (Vector3::UP.dot(ground_normal))).normalize();

    float facing_slope_dot = new_forward.dot(-slope_direction); // Negative means uphill
    float slope_angle = acos(ground_normal.dot(Vector3::UP));
    float slope_factor = sin(slope_angle);
    
    static float uphill_timer = 0.0f;
    if (facing_slope_dot < 0) {
        uphill_timer += seconds_elapsed;

        //Calculate the slope angle in degrees
        float slope_angle_deg = slope_factor * RAD2DEG;
        //desired camera angle based on the slope
        float target_camera_angle = -slope_angle_deg * 0.045f;
        target_camera_angle = clamp(target_camera_angle, -15.0f, 0.0f);
        //Smooth interpolation
        float camera_smooth_factor = 0.01f;
        //Interpolation between  camera pitch and target camera angle
        float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
        //Update camera pitch
        World::get_instance()->camera_pitch = camera_angle;
        // Only update Player 2's camera pitch if multiplayer is enabled
                if (this == World::get_instance()->player && Game::instance->multiplayer_enabled) {
                    World::get_instance()->camera2_pitch = camera_angle;
                }
        if (uphill_timer > 0.3f && !Input::isKeyPressed(SDL_SCANCODE_W)) {
            // Skier is facing uphill for more than 2 seconds -> apply extra deceleration
            //this works but doesnt stop it
            current_speed *= (1.0f - uphill_deceleration * abs(slope_factor) * seconds_elapsed);
            //current_speed -= uphill_deceleration * abs(slope_factor) * seconds_elapsed;

            if (current_speed < 5.0f && !Input::isKeyPressed(SDL_SCANCODE_W)) {
                //uphill_timer = 0.25f; // Reset to half time when speed is very low
                current_speed -= uphill_deceleration * 5.0f * seconds_elapsed;
            }
            /*
            float slope_angle_deg = slope_factor * RAD2DEG;
            float target_camera_angle = -slope_angle_deg * 0.2f; // Adjust factor for a better view
            target_camera_angle = clamp(target_camera_angle, -2.0f, 5.0f); // Allow a wider range of angles
            float camera_smooth_factor = 0.05f; // Smooth interpolation factor
            float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
            World::get_instance()->camera_pitch = camera_angle;
            */

            float slope_angle_deg = slope_factor * RAD2DEG;
            //desired camera angle based on the slope
            float target_camera_angle = -slope_angle_deg * 5.9f;
            target_camera_angle = clamp(target_camera_angle, 0.0f, -10.5f);
            //Smooth interpolation
            float camera_smooth_factor = 0.01f;
            //Interpolation between  camera pitch and target camera angle
            float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
            //Update camera pitch
            World::get_instance()->camera_pitch = camera_angle;
        }

    } else {
        uphill_timer = 0.0f; // Reset timer when not facing uphill
    }



    /////////////////////////////////// ANIMATION STATE SYSTEM ///////////////////////////////////
    // TODO: collisions animations
    // if (animation_state != eAnimationState::JUMP) {
        // if key W was pressed, play the impulse animation
    if (is_grounded) {
        if (velocity.length() > 0.0f) {
            // Check if this is player2 and handle its controls
            if (this == World::get_instance()->player2) {
                if (Input::isKeyPressed(SDL_SCANCODE_U)) {
                    if (animation_state != eAnimationState::IMPULSE) {
                        animator.playAnimation("data/meshes/animations/impulse.skanim");
                        animation_state = eAnimationState::IMPULSE;
                    }
                } else if (Input::isKeyPressed(SDL_SCANCODE_J)) {
                    if (animation_state != eAnimationState::BRAKE) {
                        animator.playAnimation("data/meshes/animations/brake.skanim");
                        animation_state = eAnimationState::BRAKE;
                    }
                } else {
                    if (animation_state != eAnimationState::MOVE) {
                        animator.playAnimation("data/meshes/animations/move.skanim");
                        animation_state = eAnimationState::MOVE;
                    }
                }
            } else { // Player 1 controls
                if (Input::isKeyPressed(SDL_SCANCODE_W)) {
                    if (animation_state != eAnimationState::IMPULSE) {
                        animator.playAnimation("data/meshes/animations/impulse.skanim");
                        animation_state = eAnimationState::IMPULSE;
                    }
                } else if (Input::isKeyPressed(SDL_SCANCODE_S)) {
                    if (animation_state != eAnimationState::BRAKE) {
                        animator.playAnimation("data/meshes/animations/brake.skanim");
                        animation_state = eAnimationState::BRAKE;
                    }
                } else {
                    if (animation_state != eAnimationState::MOVE) {
                        animator.playAnimation("data/meshes/animations/move.skanim");
                        animation_state = eAnimationState::MOVE;
                    }
                }
            }
        } else if (velocity.length() == 0.0f) {
            // Celebrate animation with different keys for each player
            if (this == World::get_instance()->player2) {
                if (Input::isKeyPressed(SDL_SCANCODE_M)) {
                    if (animation_state != eAnimationState::CELEBRATE) {
                        animator.playAnimation("data/meshes/animations/celebrate.skanim");
                        animation_state = eAnimationState::CELEBRATE;
                    }
                } else {
                    if (animation_state != eAnimationState::IDLE) {
                        animator.playAnimation("data/meshes/animations/idle.skanim");
                        animation_state = eAnimationState::IDLE;
                    }
                }
            } else { // Player 1
                if (Input::isKeyPressed(SDL_SCANCODE_V)) {
                    if (animation_state != eAnimationState::CELEBRATE) {
                        animator.playAnimation("data/meshes/animations/celebrate.skanim");
                        animation_state = eAnimationState::CELEBRATE;
                    }
                } else {
                    if (animation_state != eAnimationState::IDLE) {
                        animator.playAnimation("data/meshes/animations/idle.skanim");
                        animation_state = eAnimationState::IDLE;
                    }
                }
            }
        }
    } else if (!is_grounded) {
        if (animation_state != eAnimationState::JUMP) {
            animator.playAnimation("data/meshes/animations/fall.skanim");
            animation_state = eAnimationState::JUMP;
        }
    }
    /////////////////////////////////// ANIMATION STATE SYSTEM ///////////////////////////////////




    //if player in the air make the player rotate
    if (!is_grounded) {
        if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
            model.rotate(velocity.length() * 0.2f, Vector3(0, 1, 0));
        }
    }


    //animations
    animator.update(seconds_elapsed);

    EntityMesh::update(seconds_elapsed);
}



void Player::testCollisions(const Vector3& target_position, float seconds_elapsed) {
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;

    float ground_height = 0.0f;
    World::get_instance()->test_scene_collisions(target_position, collisions, ground_collisions, eCollisionFilter::ALL);

    bool was_grounded = is_grounded;
    is_grounded = false;
    Vector3 new_ground_normal = Vector3::UP;

    //Ground collisions
    for (const sCollisionData& collision : ground_collisions) {
        float up_factor = fabs(collision.colNormal.dot(Vector3::UP));
        if (up_factor > 0.3) {  //Allow landing on sloped surfaces
            is_grounded = true;
            ground_height = collision.colPoint.y;
            new_ground_normal = collision.colNormal; //Store normal of the ground
        }
    }

    //If grounded, update the ground_normal
    if (is_grounded) {
        this->ground_normal = new_ground_normal;
    }

    //Collision with objects
    for (const sCollisionData& collision : collisions) {
        float up_factor = collision.colNormal.dot(Vector3::UP);

        //Skip collisions that are flat (ramps,etc
        if (up_factor > 0.6f) {
            continue;
        }

        //collisions with non-ground objects
        if (up_factor < 0.3f) { //steep objects or walls
            //Calculate sliding along the side wall
            Vector3 wall_normal = collision.colNormal;

            //Check if skier is moving sideways into the wall
            Vector3 velocity_direction = velocity.normalize();
            if (velocity_direction.dot(wall_normal) < -0.5f) { //If skier is moving into the wall
                //Apply sliding along the wall
                Vector3 slide_direction = velocity_direction - wall_normal * (velocity_direction.dot(wall_normal));
                velocity = slide_direction * current_speed;
            } else {
                //Reduce speed drastically when colliding with an object
                velocity *= 0.1f;
            }
        }
    }

    //Apply gravity & movement logic
    if (!is_grounded) {
        Vector3 horizontal_velocity = Vector3(velocity.x, 0, velocity.z);
        vertical_velocity -= gravity_force * 2.8f * seconds_elapsed;
        vertical_velocity = std::max(vertical_velocity, terminal_velocity);
        velocity = horizontal_velocity + Vector3(0, vertical_velocity, 0);
    } else {
        if (!was_grounded) {
            vertical_velocity *= 0.1f; //Small bounce when landing
        } else {
            vertical_velocity = 0.0f; //Reset vertical velocity when grounded
        }

        //Apply slope physics (for ramps)
        Vector3 gravity(0, -gravity_force, 0);
        Vector3 acceleration = gravity - (ground_normal * gravity.dot(ground_normal));

        float slope_dot = ground_normal.dot(Vector3::UP);

        //Allow climbing easy slopes or ramps
        if (slope_dot < 0.9f && current_speed > 5.0f) {
            if (slope_dot < 0.0f) {  //Handle uphill slopes
                Vector3 launch_dir = velocity.normalize();
                launch_dir.y = 1.0f - slope_dot;  //Adjust for steepness
                velocity = launch_dir * current_speed;
                is_grounded = false;  //Set as not grounded when moving up ramps
            }
        } else {
            //Normal grounded movement, reset vertical velocity to 0 when grounded
            vertical_velocity = 0.0f;
        }
    }

    //Update position
    Vector3 updated_position = target_position + velocity * seconds_elapsed;
    model.setTranslation(updated_position.x, updated_position.y, updated_position.z);
}