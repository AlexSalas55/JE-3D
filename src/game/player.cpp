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
}

void Player::updateSnowParticles(float dt) {
    // Get current position once for all updates
    Vector3 pos = model.getTranslation();

    // Update existing particles
    for (int i = 0; i < MAX_SNOW_PARTICLES; i++) {
        SnowParticle& p = particles[i];
        if (p.lifetime > 0) {
            // Update particle without affecting player movement
            p.lifetime -= dt;

            Vector3 displacement = p.position - p.initial_pos;
            float dist_factor = clamp(displacement.length() / 2.0f, 0.0f, 1.0f);

            // Spread effect
            Vector3 outward_dir = displacement.length() > 0 ? displacement.normalize() : Vector3(0, 1, 0);
            float spread_force = current_speed * 0.03f * (1.0f - dist_factor);
            p.velocity += outward_dir * spread_force * dt;

            // Update position
            p.position += p.velocity * dt;

            // Gradually slow down vertical velocity
            float height_above_initial = p.position.y - p.initial_pos.y;
            p.velocity.y *= (1.0f - dt * (1.5f + height_above_initial * 0.5f));

            // Fade out effect
            float life_factor = p.lifetime / p.max_lifetime;
            float height_factor = clamp(1.0f - height_above_initial * 0.3f, 0.0f, 1.0f);
            float distance_factor = clamp(1.0f - dist_factor, 0.0f, 1.0f);
            //p.alpha = life_factor * height_factor * distance_factor;
            p.alpha = std::max(0.2f, life_factor * height_factor * distance_factor);

        }
    }

    // **Ensure Continuous Particle Generation Without Overloading**
    float base_spawn_rate = 50.0f;  
    float spawn_amount = base_spawn_rate * (current_speed / 3.0f);
    //spawn_amount = std::min(spawn_amount, static_cast<float>(MAX_SNOW_PARTICLES) / 5.0f); // Prevent lag spikes
    spawn_amount = std::max(spawn_amount, 55.0f);  // At least 5 particles per frame


    for (int i = 0; i < MAX_SNOW_PARTICLES && spawn_amount > 0; i++) {
        if (particles[i].lifetime <= 0) {
            spawn_amount--; // Reduce counter as we spawn new particles

            //Vector3 velocity_dir = velocity.normalize();
            Vector3 velocity_copy = velocity;  // Make a copy
            Vector3 velocity_dir = velocity_copy.normalize();

            float turning_factor = abs(velocity_dir.x);
            float lateral_spread = turning_factor * ((rand() % 200) - 100) * 0.01f;
            float back_spread = ((rand() % 100) * 0.01f);

            //float lifetime = 3.0f + (rand() % 200) * 0.01f;
            float lifetime = (2.5f + (rand() % 150) * 0.01f) * (1.0f / (1.0f + current_speed * 0.1f));

            particles[i].lifetime = lifetime;
            particles[i].max_lifetime = lifetime;
            particles[i].alpha = 1.0f;

            float ground_offset = 0.05f;
            particles[i].initial_pos = Vector3(
                pos.x + lateral_spread * velocity_dir.x * 1.5f,
                pos.y - 0.5f + ground_offset, 
                pos.z - back_spread * 2.0f
            );
            particles[i].position = particles[i].initial_pos;

            float up_speed = 1.0f + (rand() % 100) * 0.015f;
            float side_speed = turning_factor * current_speed * 0.4f;
            Vector3 base_velocity = velocity_dir * current_speed * 0.25f;

            particles[i].velocity = base_velocity + Vector3(
                side_speed * (lateral_spread > 0 ? 1 : -1),
                up_speed,
                -back_spread * current_speed * 0.3f
            );
        }
    }

    // **Render Particles**
    glPointSize(6.0f + (rand() % 100) * 0.02f);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_SNOW_PARTICLES; i++) {
        if (particles[i].lifetime > 0) {
            glColor4f(1.0f, 1.0f, 1.0f, particles[i].alpha);
            glVertex3f(particles[i].position.x, particles[i].position.y, particles[i].position.z);
        }
    }
    glEnd();

    // Restore GL states
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}




void Player::renderSnowParticles(Camera* camera) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);  // Disable depth writes for particles

    // Render snow particles without updating
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_SNOW_PARTICLES; i++) {
        if (particles[i].lifetime > 0) {
            glColor4f(1.0f, 1.0f, 1.0f, particles[i].alpha);
            glVertex3f(particles[i].position.x, particles[i].position.y, particles[i].position.z);
        }
    }
    glEnd();

    // Restore OpenGL states
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}


void Player::render(Camera* camera)
{
    // First render the animated player mesh
    EntityMesh::render(camera);

    //Render snow particles
    //renderSnowParticles(camera);

    // Debug visualization
    Shader* debug_shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    Mesh* sphere_mesh = Mesh::Get("data/meshes/sphere.obj");
    Matrix44 m = getGlobalMatrix();
    
    if (debug_shader && sphere_mesh)
    {
        debug_shader->enable();

        {
            m.translate(0.0f, World::get_instance()->player_height, 0.0f);
            m.scale(World::get_instance()->sphere_radius, World::get_instance()->sphere_radius, World::get_instance()->sphere_radius);

            debug_shader->setUniform("u_color", Vector4(0.0f, 1.0f, 0.0f, 1.0f));
            debug_shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
            debug_shader->setUniform("u_model", m);

            sphere_mesh->render(GL_LINES);
        }
        {
            m = getGlobalMatrix();
            m.translate(0.0f, World::get_instance()->sphere_grow, 0.0f);
            m.scale(World::get_instance()->sphere_grow, World::get_instance()->sphere_grow, World::get_instance()->sphere_grow);
            debug_shader->setUniform("u_color", Vector4(1.0f, 0.0f, 0.0f, 1.0f));
            debug_shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
            debug_shader->setUniform("u_model", m);
            sphere_mesh->render(GL_LINES);
        }

        debug_shader->disable();
    }
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

    // Portal fall check
    float x_tolerance = 25.0f;  // Smaller tolerance for X
    float y_tolerance = 32.0f;  // Smaller tolerance for Y
    float z_tolerance = 35.0f; // Larger tolerance for Z

    if (!in_portal_fall && 
        abs(position.x - 366.614f) < x_tolerance && 
        abs(position.y - (-637.898f)) < y_tolerance &&
        abs(position.z - portal_target.z) < z_tolerance) {
        
        // Enter portal fall mode
        in_portal_fall = true;
        current_speed = 0.0f;
        velocity = Vector3(0.0f);
        vertical_velocity = 0.0f;
        is_grounded = false;
        
        // Lock X position during fall
        position.x = 360.614f;
        model.setTranslation(position.x, position.y, position.z);
    }

    // Handle portal falling
    if (in_portal_fall) {
        // Only apply gravity
        velocity.y -= gravity * air_gravity_multiplier * seconds_elapsed;
        velocity.y = std::max(velocity.y, terminal_velocity);
        
        // Move towards target Z position
        float z_diff = portal_target.z - position.z;
        velocity.z = (z_diff >= 0.0f ? 1.0f : -1.0f) * std::min(abs(z_diff), 30.0f); // Smooth Z transition
        // Check if we've reached target height
        if (position.y <= portal_target.y) {
            in_portal_fall = false;
            position.y = portal_target.y;
            position.z = portal_target.z;
            model.setTranslation(position.x, position.y, position.z);
            is_grounded = true;
        }
        
        // Update position
        position = position + velocity * seconds_elapsed;
        model.setTranslation(position.x, position.y, position.z);
        
        // Skip rest of normal movement code
        return;
    }

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
        //position = Vector3(6.0f, 200.0f, 64.0f);
        position = Vector3(345.0f, 184.0f, 37.0f);
        model.setTranslation(position.x, position.y, position.z);
        current_speed = 0.0f;
        velocity = Vector3(0.0f);
        vertical_velocity = 0.0f;

    }
    //if 2 is pressed spawn the player in map2
    if (Input::isKeyPressed(SDL_SCANCODE_2)) {
        //position = Vector3(250.0f, 200.0f, -25.0f);
        position = Vector3(340.614, -647.0f, 245.0f);
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
    Vector3 best_ground_normal = Vector3::UP;
    float best_up_factor = 0.0f;

    // Find ground normal and update grounded state
    // Check all ground collisions to find the most suitable slope
    for (const sCollisionData& collision : ground_collisions) {
        float up_factor = collision.colNormal.dot(Vector3::UP);
        if (up_factor > slope_tolerance) {
            is_grounded = true;
            // Update if this is a better slope (more vertical)
            if (up_factor > best_up_factor) {
                best_up_factor = up_factor;
                best_ground_normal = collision.colNormal;
                position.y = collision.colPoint.y;
            }
        }
    }

    // Update ground normal if we found a valid ground collision
    if (is_grounded) {
        ground_normal = best_ground_normal;
    }

    if (is_grounded) {
        // Reset air time when landing
        air_time = 0.0f;

        float slope_angle = acos(ground_normal.dot(Vector3::UP));
        Vector3 slope_direction = (Vector3::UP - ground_normal * (Vector3::UP.dot(ground_normal))).normalize();

        Vector3 move_direction(0.0f);
        // Different movement controls for each player
        if (this == World::get_instance()->player2) {
            if (Input::isKeyPressed(Input::isKeyPressed(SDL_SCANCODE_UP))) move_direction.z = 1.0f;
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
                float target_camera_angle = slope_angle_deg * 0.01f;
                target_camera_angle = clamp(target_camera_angle, 0.45f, 5.0f);
                //Smooth interpolation
                float camera_smooth_factor = 0.01f;

               // Add lateral tilt calculation
                Vector3 right = model.rightVector();
                Vector3 projected_right = (right - ground_normal * right.dot(ground_normal)).normalize();
                float lateral_slope = right.dot(ground_normal); // How much the ground slopes sideways
                
                // Calculate camera roll based on lateral slope
                float target_roll = lateral_slope * 25.0f; // Convert to degrees and amplify
                target_roll = clamp(target_roll, -15.0f, 15.0f); // Limit maximum roll

                // Smooth interpolation for roll
                if (this == World::get_instance()->player2) {
                    World::get_instance()->camera2_roll = lerp(World::get_instance()->camera2_roll, target_roll, camera_smooth_factor);
                } else {
                    World::get_instance()->camera_roll = lerp(World::get_instance()->camera_roll, target_roll, camera_smooth_factor);
                }

                // Existing pitch interpolation
                if (this == World::get_instance()->player2) {
                    float camera_angle = lerp(World::get_instance()->camera2_pitch, target_camera_angle, camera_smooth_factor);
                    World::get_instance()->camera2_pitch = camera_angle;
                } else {
                    float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
                    World::get_instance()->camera_pitch = camera_angle;
                }
                

            } else if (slope_factor < 0) { //Uphill Deceleration
                current_acceleration -= uphill_deceleration * abs(slope_factor);
            }
    
            //acceleration to speed
            //current_speed += current_acceleration * seconds_elapsed;

            if (current_speed < max_speed) {
                current_speed += current_acceleration * seconds_elapsed;
            }

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
                if (current_speed > 0.1f) {
                    current_speed *= (1.0f - base_deceleration * seconds_elapsed);
                    
                    // Get the current movement direction
                    Vector3 current_dir = velocity.normalize();
                    
                    // Only update velocity if we have a valid direction
                    if (current_dir.length() > 0.01f) {
                        velocity = current_dir * current_speed;
                    } else {
                        // If no clear direction, just stop
                        current_speed = 0.0f;
                        velocity = Vector3(0.0f);
                    }
                } else {
                    // Below threshold, just stop
                    current_speed = 0.0f;
                    velocity = Vector3(0.0f);
                }

                // Calculate camera angle based on player-specific slope and facing direction
                Vector3 player_forward = model.frontVector().normalize();
                Vector3 slope_dir = calculateSlopeDirection();
                float facing_factor = player_forward.dot(-slope_dir);
                float slope_angle_deg = slope_factor * RAD2DEG;
                
                float target_camera_angle;
                if (facing_factor < 0) { // Facing uphill
                    target_camera_angle = -slope_angle_deg * 0.03f;
                    target_camera_angle = clamp(target_camera_angle, -5.0f, -0.01f);
                } else { // Facing downhill
                    target_camera_angle = slope_angle_deg * 0.03f;
                    target_camera_angle = clamp(target_camera_angle, 0.01f, 5.0f);
                }
                
                updateCameraPitch(target_camera_angle);

            }

            //Calculate the slope angle in degrees
            float slope_angle_deg = slope_factor * RAD2DEG;
            //desired camera angle based on the slope
            float target_camera_angle = slope_angle_deg * 0.02f;
            target_camera_angle = clamp(target_camera_angle, 0.45f, 5.0f);
            //Smooth interpolation
            float camera_smooth_factor = 0.01f;
            //Interpolation between  camera pitch and target camera angle
            if (this == World::get_instance()->player2) {
            float camera_angle = lerp(World::get_instance()->camera2_pitch, target_camera_angle, camera_smooth_factor);
            World::get_instance()->camera2_pitch = camera_angle;
            } else {
                float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
                World::get_instance()->camera_pitch = camera_angle;
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
        // Update air time
        air_time += seconds_elapsed;
        
        // Check if in air for more than 1 second
        if (air_time > 0.9f) {
            //std::cout << "Player has been in the air for more than 1 second!" << std::endl;
            // Get player-specific slope and facing information for air time camera
            Vector3 player_forward = model.frontVector().normalize();
            Vector3 slope_dir = calculateSlopeDirection();
            float facing_factor = player_forward.dot(-slope_dir);
            float slope_factor = calculateSlopeFactor();
            float slope_angle_deg = slope_factor * RAD2DEG;
            
            
            // Adjust camera angle based on player's facing direction in air
            float target_camera_angle;
            if (facing_factor < 0) { // Facing uphill in air
                target_camera_angle = -slope_angle_deg * 4.0f;
                target_camera_angle = clamp(target_camera_angle, 1.0, 5.0f);
            } else { // Facing downhill in air
                target_camera_angle = slope_angle_deg * 4.0f;
                target_camera_angle = clamp(target_camera_angle, 1.0f, 5.0f);
            }
            
            updateCameraPitch(target_camera_angle);
        }

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



    }
    //Update position and check collisions
    Vector3 desired_position = position + velocity * seconds_elapsed;
    testCollisions(desired_position, seconds_elapsed);
    model.rotate(camera_yaw, Vector3(0, 1, 0));


    //snapping the player Y pos to ground (if on ground)
    if (is_grounded) {
        air_time = 0.0f;
        position.y = ground_normal.y;
    }
    
    
    // Calculate player-specific forward vector
    Vector3 forward = model.frontVector();
    Vector3 new_forward = (forward - ground_normal * forward.dot(ground_normal)).normalize();
    model.setFrontAndOrthonormalize(new_forward);
    
    
    // Each player calculates their own slope direction and facing angle
    Vector3 slope_direction = calculateSlopeDirection();
    Vector3 player_forward = model.frontVector().normalize();
    float facing_slope_dot = player_forward.dot(-slope_direction); // Negative means uphill
    float slope_factor = calculateSlopeFactor();
    
    // Handle uphill movement
    if (facing_slope_dot < 0) {
        uphill_timer += seconds_elapsed;
        // Update camera pitch for uphill movement
        float slope_angle_deg = slope_factor * RAD2DEG;
        float target_angle = -slope_angle_deg * 0.045f;
        target_angle = clamp(target_angle, -15.0f, 0.0f);
        updateCameraPitch(target_angle);

        // Handle uphill physics and deceleration
        handleUphillMovement(seconds_elapsed, slope_factor);
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
                if (Input::isKeyPressed(SDL_SCANCODE_UP)) {
                    if (animation_state != eAnimationState::IMPULSE) {
                        animator.playAnimation("data/meshes/animations/impulse.skanim");
                        animation_state = eAnimationState::IMPULSE;
                    }
                } else if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) {
                    if (animation_state != eAnimationState::BRAKE) {
                        animator.playAnimation("data/meshes/animations/brake.skanim");
                        animation_state = eAnimationState::BRAKE;
                    }
                    // Add braking physics
                    float brake_force = 3.0f * base_deceleration;
                    current_speed *= (1.0f - brake_force * seconds_elapsed);
                    if (current_speed < 0.1f)
                        current_speed = 0.0f;
                    if (velocity.length() > 0) {
                        velocity = velocity.normalize() * current_speed;
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
                     // Add braking physics
                    float brake_force = 3.0f * base_deceleration; // Stronger than base deceleration
                    // Apply stronger deceleration when braking
                    current_speed *= (1.0f - brake_force * seconds_elapsed);
                    // Ensure we don't get negative speed from braking
                    if (current_speed < 0.1f)
                        current_speed = 0.0f;
                    // Update velocity to match new speed
                    if (velocity.length() > 0) {
                        velocity = velocity.normalize() * current_speed;
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
    } else if (air_time > 0.45f) {
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

    //updateSnowParticles(seconds_elapsed);

    EntityMesh::update(seconds_elapsed);
}



// Helper methods implementation
Vector3 Player::calculateSlopeDirection() const {
    return (Vector3::UP - ground_normal * (Vector3::UP.dot(ground_normal))).normalize();
}

float Player::calculateSlopeFactor() const {
    float slope_angle = acos(ground_normal.dot(Vector3::UP));
    return sin(slope_angle);
}

void Player::updateCameraPitch(float target_angle) {
    bool is_player2 = (this == World::get_instance()->player2);
    float& current_pitch = is_player2 ? World::get_instance()->camera2_pitch : World::get_instance()->camera_pitch;
    
    // Use player's own model forward to determine if they're facing up/down the slope
    Vector3 player_forward = model.frontVector().normalize();
    Vector3 slope_dir = calculateSlopeDirection();
    float facing_factor = player_forward.dot(-slope_dir);
    
    // Adjust target angle based on player's specific facing direction
    if (facing_factor < 0) { // Facing uphill
        target_angle = clamp(target_angle, -15.0f, 0.0f);
    } else { // Facing downhill
        target_angle = clamp(target_angle, 0.0f, 15.0f);
    }
    
    // Smooth camera transition using player-specific values
    float camera_angle = lerp(current_pitch, target_angle, camera_smooth_factor);
    current_pitch = camera_angle;
}

void Player::handleUphillMovement(float seconds_elapsed, float slope_factor) {
    bool is_player2 = (this == World::get_instance()->player2);
    bool is_moving_forward = is_player2 ? Input::isKeyPressed(SDL_SCANCODE_U) : Input::isKeyPressed(SDL_SCANCODE_W);
    
    // Get player-specific forward direction and slope
    Vector3 player_forward = model.frontVector().normalize();
    Vector3 slope_dir = calculateSlopeDirection();
    float facing_factor = player_forward.dot(-slope_dir);
    
    if (uphill_timer > 0.3f && !is_moving_forward) {
        // Apply deceleration when facing uphill
        if (facing_factor < 0) { // Only apply when actually facing uphill
            current_speed *= (1.0f - uphill_deceleration * abs(slope_factor) * seconds_elapsed);
            
            if (current_speed < 5.0f) {
                current_speed -= uphill_deceleration * 3.0f * seconds_elapsed;
            }
            
            if (current_speed > 10.0f) {
                /*
                float slope_angle_deg = slope_factor * RAD2DEG;
                float steep_target_angle = -slope_angle_deg * 5.9f;
                steep_target_angle = clamp(steep_target_angle, -15.0f, 0.0f); // Adjusted clamp values
                updateCameraPitch(steep_target_angle);
                */
            }
        }
    }
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
    float best_up_factor = 0.3f; // Minimum threshold for valid slopes
    for (const sCollisionData& collision : ground_collisions) {
        float up_factor = fabs(collision.colNormal.dot(Vector3::UP));
        if (up_factor > best_up_factor) {  //Allow landing on sloped surfaces
            is_grounded = true;
            ground_height = collision.colPoint.y;
            new_ground_normal = collision.colNormal; //Store normal of the ground
            best_up_factor = up_factor; // Update best factor
        }
    }

    //If grounded, update the ground_normal for this specific player
    if (is_grounded) {
        this->ground_normal = new_ground_normal;
    } else {
        this->ground_normal = Vector3::UP; // Reset to default when in air
    }
    //Collision with objects
    for (const sCollisionData& collision : collisions) {
        EntityCollider* collider = dynamic_cast<EntityCollider*>(collision.colEntity);
        
        float up_factor = collision.colNormal.dot(Vector3::UP);

        if (collision.colEntity && collider->name != "" && collider->name != " ") {
            //std::cout << "Player is colliding with: " << collider->name << std::endl;
        }

        
    
        //Skip collisions that are flat (ramps,etc
        if (up_factor > 0.6f) {
            continue;
        }


        if (up_factor < 0.3f) { // True walls
            Vector3 wall_normal = collision.colNormal;
            Vector3 velocity_direction = velocity.normalize();
            //current_speed *= 0.5f;

            
            //if collision name is not CDas_Board_A__ef_dashboard
            if (collider->name != "scene/ice_jump_A__sn_jumpSnow01/ice_jump_A__sn_jumpSnow01.obj" 
                && collider->name != "scene/CDas_Board_A__ef_dashboard/CDas_Board_A__ef_dashboard.obj"
                && collider->name != "scene/CGli_Board__ef_glideboard/CGli_Board__ef_glideboard.obj"
                && collider->name != "scene/polySurface8364587_1__sn_snowroad04s/polySurface8364587_1__sn_snowroad04s.obj"
                && collider->name != "scene/polySurface8364601__sn_GoalLine/polySurface8364601__sn_GoalLine.obj"
                && collider->name !=  "scene/polySurface8363355__sn_snowroad_Ktens/polySurface8363355__sn_snowroad_Ktens.obj") {
                //current_speed *= 0.5f;
                //current_speed = 0.0f;
                //velocity = collision.colNormal * bounce_force;
                //vertical_velocity = 0.0f;

                float impact_speed = velocity.length();
                impact_speed = std::max(impact_speed, 25.0f);
                
                // Get the incoming velocity direction
                Vector3 incoming_dir = velocity.normalize();
                Vector3 bounce_dir;
                // Normal reflection for head-on collisions
                bounce_dir = incoming_dir - collision.colNormal * (2.0f * incoming_dir.dot(collision.colNormal));
                bounce_dir.normalize();

                // Apply bounce
                velocity = bounce_dir * impact_speed;
                //current_speed = impact_speed * 0.1f;
                current_speed = 0.0f;
                vertical_velocity = 0.0f;
                
                // Move player back to collision point and push them away
                Vector3 safe_position = collision.colPoint + (collision.colNormal * (5.0f + rand() % 10));
                model.setTranslation(safe_position.x, safe_position.y, safe_position.z);
            }

            if (velocity_direction.dot(wall_normal) < -0.5f) {
                // Only reduce speed for actual wall collisions
                //current_speed *= 0.5f;  // Move it here so it only affects wall hits
                //Vector3 slide_direction = velocity_direction - wall_normal * (velocity_direction.dot(wall_normal));
                //velocity = slide_direction * current_speed;
            } else {
                //velocity *= 0.1f;
                //current_speed *= 0.9f;
            }
        }
        /*
        //collisions with non-ground objects
        if (up_factor < 0.3f) { //steep objects or walls
            //Calculate sliding along the side wall
            Vector3 wall_normal = collision.colNormal;

            //current_speed *= 0.5f;
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
            */
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