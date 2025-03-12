#include "player.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/input.h"
#include "game/game.h"
#include "game/world.h"
#include "framework/entities/entity_collider.h"
#include "framework/audio.h"


Player::Player(Mesh* mesh, const Material& material, const std::string& name)
    : EntityMesh(mesh, material)
{
    this->name = name;
    walk_speed = 5.0f;
    //Animations
    isAnimated = true;
    
    //initialize animator
    Animation* idle = Animation::Get("data/meshes/animations/idle.skanim");
    if (idle) {
        animator.playAnimation("data/meshes/animations/idle.skanim", true);
    }

    //init falling snow particles
    for (int i = 0; i < MAX_FALLING_SNOW; i++) {
        spawnFallingSnowParticle(i);
    }
}

void Player::spawnFallingSnowParticle(int index) {
    Vector3 player_pos = model.getTranslation();
    Vector3 front = model.frontVector().normalize();
    float radius = 30.0f;  //area around player for snow
    
    //spawn snow
    falling_snow[index].position = Vector3(
        player_pos.x + (rand() % 1000 - 500) * 0.06f, 
        player_pos.y + 15.0f + (rand() % 100) * 0.1f, 
        player_pos.z + front.z * 10.0f + (rand() % 1000 - 500) * 0.06f  //bias towards front
    );
    falling_snow[index].offset = (rand() % 1000) * 0.001f * 2 * M_PI;
    falling_snow[index].speed = 6.0f + (rand() % 100) * 0.04f;  //slower for better visibility
    falling_snow[index].alpha = 0.8f + (rand() % 20) * 0.01f;
    falling_snow[index].active = true;
}

void Player::updateFallingSnow(float dt, const Vector3& camera_pos) {
    Vector3 player_pos = model.getTranslation();
    Vector3 front = model.frontVector().normalize();
    float camera_speed = velocity.length();
    //spawn snow particles
    for (int i = 0; i < MAX_FALLING_SNOW; i++) {
        if (!falling_snow[i].active) {
            spawnFallingSnowParticle(i);
            continue;
        }
        //update position
        falling_snow[i].position.y -= falling_snow[i].speed * dt;
        falling_snow[i].position.x += cos(falling_snow[i].offset + Game::instance->time * 2.0f) * dt * 0.5f; //gentle movement
        falling_snow[i].position.z += sin(falling_snow[i].offset + Game::instance->time * 1.5f) * dt * 0.3f; 

        //respawn if too far from player or too low
        if (falling_snow[i].position.y < player_pos.y - 5.0f || 
            (falling_snow[i].position - player_pos).length() > 40.0f) { 
            spawnFallingSnowParticle(i);
        }
    }
}

void Player::renderFallingSnow(Camera* camera) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    
    glPointSize(2.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_FALLING_SNOW; i++) {
        if (falling_snow[i].active) {
            glColor4f(1.0f, 1.0f, 1.0f, falling_snow[i].alpha);
            glVertex3f(falling_snow[i].position.x, 
                      falling_snow[i].position.y, 
                      falling_snow[i].position.z);
        }
    }
    glEnd();
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Player::render(Camera* camera)
{
    //render the animated player mesh
    EntityMesh::render(camera);

    //render falling snow
    renderFallingSnow(camera);

    //debug visualization
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

    //manage wind sound based on speed
    float speed = velocity.length();
    if (speed > 0.1f) {
        //volume based on speed
        float volume = clamp(speed / top_speed * 0.4f, 0.0f, 0.8f);
        
        //start sound if not playing
        if (!is_wind_sound_playing) {
            wind_sound_channel = Audio::Play("data/assets/audio/sound_wind.wav", volume, BASS_SAMPLE_LOOP);
            is_wind_sound_playing = true;
        } else {
            //adjust volume if is playing
            BASS_ChannelSetAttribute(wind_sound_channel, BASS_ATTRIB_VOL, volume);
        }
    } else if (is_wind_sound_playing) {
        //stop sound if speed is too low
        Audio::Stop(wind_sound_channel);
        is_wind_sound_playing = false;
    }

    //Portal fall check (first checkpoint)
    float x_tolerance = 25.0f; 
    float y_tolerance = 32.0f;  
    float z_tolerance = 35.0f; 

    if (!in_portal_fall && 
        abs(position.x - 366.614f) < x_tolerance && 
        abs(position.y - (-637.898f)) < y_tolerance &&
        abs(position.z - portal_target.z) < z_tolerance) {
        
        //enter portal fall mode
        in_portal_fall = true;
        current_speed = 0.0f;
        velocity = Vector3(0.0f);
        vertical_velocity = 0.0f;
        is_grounded = false;
        
        //lock X position during fall
        position.x = 360.614f;
        model.setTranslation(position.x, position.y, position.z);
    }

    //handle portal falling
    if (in_portal_fall) {
        //only apply gravity
        velocity.y -= gravity * air_gravity_multiplier * seconds_elapsed;
        velocity.y = std::max(velocity.y, terminal_velocity);
        //move target Z position
        float z_diff = portal_target.z - position.z;
        velocity.z = (z_diff >= 0.0f ? 1.0f : -1.0f) * std::min(abs(z_diff), 30.0f); // Smooth Z transition
        //check if target reached
        if (position.y <= portal_target.y) {
            in_portal_fall = false;
            position.y = portal_target.y;
            position.z = portal_target.z;
            model.setTranslation(position.x, position.y, position.z);
            is_grounded = true;
        }
        
        //update pos
        position = position + velocity * seconds_elapsed;
        model.setTranslation(position.x, position.y, position.z);
        
        return;
    }
    //if 1 pressed player spawn in first pos
    if (Input::isKeyPressed(SDL_SCANCODE_1)) {
        position = Vector3(345.0f, 184.0f, 37.0f);
        model.setTranslation(position.x, position.y, position.z);
        current_speed = 0.0f;
        velocity = Vector3(0.0f);
        vertical_velocity = 0.0f;
    }
    //if 2 pressed player spawn in second pos
    if (Input::isKeyPressed(SDL_SCANCODE_2)) {
        position = Vector3(367.0f, -762.0f, 228.0f);
        model.setTranslation(position.x, position.y, position.z);
        current_speed = 0.0f;
        velocity = Vector3(0.0f);
        vertical_velocity = 0.0f;
    }
    //debug position
    //std::cout << "Skier Position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
    //ground detection (handling collisions and ground check)
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;
    World::get_instance()->test_scene_collisions(position, collisions, ground_collisions, eCollisionFilter::ALL);

    is_grounded = false;
    Vector3 best_ground_normal = Vector3::UP;
    float best_up_factor = 0.0f;

    //find ground normal and update grounded state
    for (const sCollisionData& collision : ground_collisions) {
        float up_factor = collision.colNormal.dot(Vector3::UP);
        if (up_factor > slope_tolerance) {
            is_grounded = true;
            //update if is a better slope
            if (up_factor > best_up_factor) {
                best_up_factor = up_factor;
                best_ground_normal = collision.colNormal;
                position.y = collision.colPoint.y;
            }
        }
    }

    //update ground normal if valid ground collision
    if (is_grounded) {
        ground_normal = best_ground_normal;
    }

    if (is_grounded) {
        //reset air time when landing
        air_time = 0.0f;
        float slope_angle = acos(ground_normal.dot(Vector3::UP));
        Vector3 slope_direction = (Vector3::UP - ground_normal * (Vector3::UP.dot(ground_normal))).normalize();

        Vector3 move_direction(0.0f);
        //movement controls for each player
        if (this == World::get_instance()->player2) {
            if (Input::isKeyPressed(SDL_SCANCODE_UP)) move_direction.z = 1.0f;
        } else {
            if (Input::isKeyPressed(SDL_SCANCODE_W)) move_direction.z = 1.0f;
        }

        if (move_direction.length() > 0.01f) {
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
                //slope angle in degrees
                float slope_angle_deg = slope_factor * RAD2DEG;
                //desired camera angle based on slope
                float target_camera_angle = slope_angle_deg * 0.01f;
                target_camera_angle = clamp(target_camera_angle, 0.45f, 5.0f);
                //smooth interpolation
                float camera_smooth_factor = 0.01f;
                //lateral tilt calculation
                Vector3 right = model.rightVector();
                Vector3 projected_right = (right - ground_normal * right.dot(ground_normal)).normalize();
                float lateral_slope = right.dot(ground_normal); // How much the ground slopes sideways
                
                //camera roll based on lateral slope
                float target_roll = lateral_slope * 25.0f; //convert to degrees and amplify
                target_roll = clamp(target_roll, -15.0f, 15.0f); //limit maximum roll

                //smooth interpolation for roll
                if (this == World::get_instance()->player2) {
                    World::get_instance()->camera2_roll = lerp(World::get_instance()->camera2_roll, target_roll, camera_smooth_factor);
                } else {
                    World::get_instance()->camera_roll = lerp(World::get_instance()->camera_roll, target_roll, camera_smooth_factor);
                }

                //pitch interpolation
                if (this == World::get_instance()->player2) {
                    float camera_angle = lerp(World::get_instance()->camera2_pitch, target_camera_angle, camera_smooth_factor);
                    World::get_instance()->camera2_pitch = camera_angle;
                } else {
                    float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
                    World::get_instance()->camera_pitch = camera_angle;
                }
                
            } else if (slope_factor < 0) { //uphill deceleration
                current_acceleration -= uphill_deceleration * abs(slope_factor);
            }

            if (current_speed < max_speed) {
                current_speed += current_acceleration * seconds_elapsed;
            }
            //update velocity
            velocity = slope_move * current_speed;
        }
        else {
            //when no input gravity and friction affect speed
            Vector3 camera_direction = front;
            camera_direction.y = 0.0f; //ignore vertical
            //project camera forward dir onto ground
            Vector3 slope_move = (camera_direction - ground_normal * camera_direction.dot(ground_normal)).normalize();
            //slope factor
            float slope_factor = sin(slope_angle);
            //apply gravity(only downhill)
            if (slope_factor > 0.01f) { 
                velocity += -slope_direction * (gravity_force * slope_factor * seconds_elapsed);
                //incr elocity over time
                current_speed += acceleration * seconds_elapsed;
            }
            else if (slope_factor < 0) {
            } else { //flat ground (decelerate gradually)
                if (current_speed > 0.1f) {
                    current_speed *= (1.0f - base_deceleration * seconds_elapsed);
                    
                    //current movement direction
                    Vector3 current_dir = velocity.normalize();
                    
                    //only update velocity if valid direction
                    if (current_dir.length() > 0.01f) {
                        velocity = current_dir * current_speed;
                    } else {
                        //if no clear direction, just stop
                        current_speed = 0.0f;
                        velocity = Vector3(0.0f);
                    }
                } else {
                    //if <threshold,stop
                    current_speed = 0.0f;
                    velocity = Vector3(0.0f);
                }
                //camera angle based on player-specific slope and facing direction
                Vector3 player_forward = model.frontVector().normalize();
                Vector3 slope_dir = calculateSlopeDirection();
                float facing_factor = player_forward.dot(-slope_dir);
                float slope_angle_deg = slope_factor * RAD2DEG;
                
                float target_camera_angle;
                if (facing_factor < 0) { //facing uphill
                    target_camera_angle = -slope_angle_deg * 0.03f;
                    target_camera_angle = clamp(target_camera_angle, -5.0f, -0.01f);
                } else { //facing downhill
                    target_camera_angle = slope_angle_deg * 0.03f;
                    target_camera_angle = clamp(target_camera_angle, 0.01f, 5.0f);
                }
                updateCameraPitch(target_camera_angle);
            }

            //slope angle in degrees
            float slope_angle_deg = slope_factor * RAD2DEG;
            //desired camera angle based on slope
            float target_camera_angle = slope_angle_deg * 0.02f;
            target_camera_angle = clamp(target_camera_angle, 0.45f, 5.0f);
            //smooth interpolation
            float camera_smooth_factor = 0.01f;
            //interpolation between camera pitch and target camera angle
            if (this == World::get_instance()->player2) {
            float camera_angle = lerp(World::get_instance()->camera2_pitch, target_camera_angle, camera_smooth_factor);
            World::get_instance()->camera2_pitch = camera_angle;
            } else {
                float camera_angle = lerp(World::get_instance()->camera_pitch, target_camera_angle, camera_smooth_factor);
                World::get_instance()->camera_pitch = camera_angle;
            }
            //normalize velocity to current speed
            if (velocity.length() > 0) {
                velocity = velocity.normalize() * current_speed;
            }
            //update velocity based on slope-movement direction and speed
            velocity = slope_move * current_speed;
        }
        
        //dynamic friction on ground to reduce speed over time
        float effective_friction = ground_friction;
        float slope_factor = sin(slope_angle);

        if (slope_factor > 0) {
            effective_friction *= (1.0f - slope_factor * 0.5f); //downhill less friction

        } else if (slope_factor < 0) {
            //a lot more when uphill
            effective_friction *= (1.0f + abs(slope_factor) * 0.5f);
        }
        //effective friction based on the slope
        current_speed *= (1.0f - effective_friction * seconds_elapsed);
    }

    //air physics
    if (!is_grounded) {
        //update air time
        air_time += seconds_elapsed;
        
        //if in air for more than
        if (air_time > 0.9f) {
            //air camera for player
            Vector3 player_forward = model.frontVector().normalize();
            Vector3 slope_dir = calculateSlopeDirection();
            float facing_factor = player_forward.dot(-slope_dir);
            float slope_factor = calculateSlopeFactor();
            float slope_angle_deg = slope_factor * RAD2DEG;
            
            //camera angle based on player facing direction in air
            float target_camera_angle;
            if (facing_factor < 0) { //facing uphill in air
                target_camera_angle = -slope_angle_deg * 4.0f;
                target_camera_angle = clamp(target_camera_angle, 1.0, 5.0f);
            } else { //facing downhill in air
                target_camera_angle = slope_angle_deg * 4.0f;
                target_camera_angle = clamp(target_camera_angle, 1.0f, 5.0f);
            }
            updateCameraPitch(target_camera_angle);
        } 

        //gravity in correct dir
        Vector3 gravity_direction = Vector3::UP - ground_normal * (ground_normal.dot(Vector3::UP));
        gravity_direction.normalize();
        
        //gravity increased to make fall faster
        velocity.y -= gravity * air_gravity_multiplier * seconds_elapsed;
        velocity += gravity_direction * gravity_force * seconds_elapsed;

        //push the skier forward
        velocity += front * current_speed * seconds_elapsed;

        //terminal velocity to prevent floating
        velocity.y = std::max(velocity.y, terminal_velocity);
    }
    //update pos and check collisions
    Vector3 desired_position = position + velocity * seconds_elapsed;
    testCollisions(desired_position, seconds_elapsed);
    model.rotate(camera_yaw, Vector3(0, 1, 0));

    //snapping player pos to ground (if on ground)
    if (is_grounded) {
        air_time = 0.0f;
        position.y = ground_normal.y;
    }

    //player forward vector
    Vector3 forward = model.frontVector();
    Vector3 new_forward = (forward - ground_normal * forward.dot(ground_normal)).normalize();
    model.setFrontAndOrthonormalize(new_forward);
    
    //each player calculates their own slope direction and facing angle
    Vector3 slope_direction = calculateSlopeDirection();
    Vector3 player_forward = model.frontVector().normalize();
    float facing_slope_dot = player_forward.dot(-slope_direction); //negative slope means uphill
    float slope_factor = calculateSlopeFactor();
    
    //handle uphill movement
    if (facing_slope_dot < 0) {
        uphill_timer += seconds_elapsed;
        //camera pitch for uphill movement
        float slope_angle_deg = slope_factor * RAD2DEG;
        float target_angle = -slope_angle_deg * 0.045f;
        target_angle = clamp(target_angle, -15.0f, 0.0f);
        updateCameraPitch(target_angle);

        //uphill physics and deceleration
        handleUphillMovement(seconds_elapsed, slope_factor);
    } else {
        uphill_timer = 0.0f; //reset timer if not facing uphill
    }

    /////////////////////////////////// ANIMATION STATE SYSTEM ///////////////////////////////////
    if (is_grounded) {
        if (velocity.length() > 0.0f) {
            //play move sound if not already playing
            if (!is_move_sound_playing) {
                move_sound_channel = Audio::Play("data/assets/audio/sound_move.wav", 0.2f);
                is_move_sound_playing = true;
            }
            
            //check if this is player2 and handle its controls
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
                    
                    //stop move sound if playing
                    if (is_move_sound_playing) {
                        Audio::Stop(move_sound_channel);
                        is_move_sound_playing = false;
                    }
                    
                    //play brake sound if not already playing
                    if (!is_brake_sound_playing) {
                        brake_sound_channel = Audio::Play("data/assets/audio/sound_brake.wav", 0.7f);
                        is_brake_sound_playing = true;
                    }
                    
                    //braking physics
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
                    
                    //stop brake sound if playing
                    if (is_brake_sound_playing) {
                        Audio::Stop(brake_sound_channel);
                        is_brake_sound_playing = false;
                    }
                }
            } else { //player 1 controls
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
                    
                    //stop move sound if playing
                    if (is_move_sound_playing) {
                        Audio::Stop(move_sound_channel);
                        is_move_sound_playing = false;
                    }
                    
                    //play brake sound if not already playing
                    if (!is_brake_sound_playing) {
                        brake_sound_channel = Audio::Play("data/assets/audio/sound_brake.wav", 0.7f);
                        is_brake_sound_playing = true;
                    }
                    
                    //braking physics
                    float brake_force = 3.0f * base_deceleration; //stronger than base deceleration
                    current_speed *= (1.0f - brake_force * seconds_elapsed);
                    //no negative speed from braking
                    if (current_speed < 0.1f)
                        current_speed = 0.0f;
                    //update velocity to match new speed
                    if (velocity.length() > 0) {
                        velocity = velocity.normalize() * current_speed;
                    }
                } else {
                    if (animation_state != eAnimationState::MOVE) {
                        animator.playAnimation("data/meshes/animations/move.skanim");
                        animation_state = eAnimationState::MOVE;
                    }
                    
                    //stop brake sound if playing
                    if (is_brake_sound_playing) {
                        Audio::Stop(brake_sound_channel);
                        is_brake_sound_playing = false;
                    }
                }
            }
        } else if (velocity.length() == 0.0f) {
            //stop move sound if playing
            if (is_move_sound_playing) {
                Audio::Stop(move_sound_channel);
                is_move_sound_playing = false;
            }
            
            //stop brake sound if playing
            if (is_brake_sound_playing) {
                Audio::Stop(brake_sound_channel);
                is_brake_sound_playing = false;
            }
            
            //celebrate animation with different keys for each player
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
    }else if (air_time > 0.25f) {
        //stop all sounds when player not on ground
        //(except wind sound)
        if (is_move_sound_playing) {
            Audio::Stop(move_sound_channel);
            is_move_sound_playing = false;
        }
        if (is_brake_sound_playing) {
            Audio::Stop(brake_sound_channel);
            is_brake_sound_playing = false;
        }
        if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) {
            model.rotate(velocity.length() * 0.2f, Vector3(0, 1, 0));
        }
    }

    //animations
    animator.update(seconds_elapsed);

    updateFallingSnow(seconds_elapsed, World::get_instance()->camera->eye);

    EntityMesh::update(seconds_elapsed);
}



//helper methods
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
    
    //use player model forward to determine if facing up or down slope
    Vector3 player_forward = model.frontVector().normalize();
    Vector3 slope_dir = calculateSlopeDirection();
    float facing_factor = player_forward.dot(-slope_dir);
    
    //adjust target angle based on player specific facing direction
    if (facing_factor < 0) { //facing uphill
        target_angle = clamp(target_angle, -15.0f, 0.0f);
    } else { //facing downhill
        target_angle = clamp(target_angle, 0.0f, 15.0f);
    }
    
    //smooth camera transition using player specific values
    float camera_angle = lerp(current_pitch, target_angle, camera_smooth_factor);
    current_pitch = camera_angle;
}

void Player::handleUphillMovement(float seconds_elapsed, float slope_factor) {
    bool is_player2 = (this == World::get_instance()->player2);
    bool is_moving_forward = is_player2 ? Input::isKeyPressed(SDL_SCANCODE_UP) : Input::isKeyPressed(SDL_SCANCODE_W);
    
    //get player forward dir and slope
    Vector3 player_forward = model.frontVector().normalize();
    Vector3 slope_dir = calculateSlopeDirection();
    float facing_factor = player_forward.dot(-slope_dir);
    
    if (uphill_timer > 0.3f && !is_moving_forward) {
        //apply deceleration when facing uphill
        if (facing_factor < 0) { //only apply when actually facing uphill
            current_speed *= (1.0f - uphill_deceleration * abs(slope_factor) * seconds_elapsed);
            
            if (current_speed < 5.0f) {
                current_speed -= uphill_deceleration * 3.0f * seconds_elapsed;
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

    //ground collisions
    float best_up_factor = 0.3f; //minimum threshold for valid slopes
    for (const sCollisionData& collision : ground_collisions) {
        float up_factor = collision.colNormal.dot(Vector3::UP);
        if (up_factor > best_up_factor) {  //allow landing on sloped surfaces
            is_grounded = true;
            ground_height = collision.colPoint.y;
            new_ground_normal = collision.colNormal; //store normal of the ground
            best_up_factor = up_factor; //update best factor
        }
    }

    //if grounded, update the ground_normal for this specific player
    if (is_grounded) {
        this->ground_normal = new_ground_normal;
    } else {
        this->ground_normal = Vector3::UP; //reset to default when in air
    }
    //collision with objects
    for (const sCollisionData& collision : collisions) {
        EntityCollider* collider = dynamic_cast<EntityCollider*>(collision.colEntity);
        
        float up_factor = collision.colNormal.dot(Vector3::UP);

        if (collision.colEntity && collider->name != "" && collider->name != " ") {
            //std::cout << "Player is colliding with: " << collider->name << std::endl;
        }

        //Skip collisions that are flat (ramps, and so
        if (up_factor > 0.6f) {
            continue;
        }

        if (up_factor < 0.3f) { //True walls
            Vector3 wall_normal = collision.colNormal;
            Vector3 velocity_direction = velocity.normalize();
            
            if (collider->name != "scene/ice_jump_A__sn_jumpSnow01/ice_jump_A__sn_jumpSnow01.obj" 
                && collider->name != "scene/CDas_Board_A__ef_dashboard/CDas_Board_A__ef_dashboard.obj"
                && collider->name != "scene/CGli_Board__ef_glideboard/CGli_Board__ef_glideboard.obj"
                && collider->name != "scene/polySurface8364601__sn_GoalLine/polySurface8364601__sn_GoalLine.obj"
                && collider->name !=  "scene/polySurface8363355__sn_snowroad_Ktens/polySurface8363355__sn_snowroad_Ktens.obj"
                && collider->name != "scene/TreeJump001__sn_woodRoad01/TreeJump001__sn_woodRoad01.obj"
                && collider->name != "scene/CGra_Flame__PanelFlame1/CGra_Flame__PanelFlame1.obj") {
                
                //update collision tracking
                double current_time = Game::instance->time;
                if (current_time - last_collision_time > 2.0) {
                    //reset counter if more than 2 sec
                    collision_count = 0;
                }
                collision_count++;
                last_collision_time = current_time;

                //check if we need to recover position
                if (collision_count >= 500 && current_time - last_collision_time <= 3.0) {
                    //reset collision tracking
                    collision_count = 0;
                    last_collision_time = 0.0;
                    
                    //reset physics state
                    velocity = Vector3(0.0f);
                    current_speed = 0.0f;
                    vertical_velocity = 0.0f;
                    
                    //recover position
                    model.setTranslation(recovery_position.x, recovery_position.y, recovery_position.z);
                    return;
                }

                float impact_speed = velocity.length();
                impact_speed = std::max(impact_speed, 25.0f);
                
                Vector3 incoming_dir = velocity.normalize();
                Vector3 bounce_dir;
                bounce_dir = incoming_dir - collision.colNormal * (2.0f * incoming_dir.dot(collision.colNormal));
                bounce_dir.normalize();

                velocity = bounce_dir * impact_speed;
                current_speed = 0.0f;
                vertical_velocity = 0.0f;
                
                Vector3 safe_position = collision.colPoint + (collision.colNormal * (5.0f + rand() % 10));
                model.setTranslation(safe_position.x, safe_position.y, safe_position.z);
            }

        }
    }
    //apply gravity & movement logic
    if (!is_grounded) {
        Vector3 horizontal_velocity = Vector3(velocity.x, 0, velocity.z);
        vertical_velocity -= gravity_force * 2.8f * seconds_elapsed;
        vertical_velocity = std::max(vertical_velocity, terminal_velocity);
        velocity = horizontal_velocity + Vector3(0, vertical_velocity, 0);
    } else {
        if (!was_grounded) {
            vertical_velocity *= 0.1f; //small bounce when landing
        } else {
            vertical_velocity = 0.0f; //reset vertical velocity when grounded
        }

        //apply slope physics (for ramps)
        Vector3 gravity(0, -gravity_force, 0);
        Vector3 acceleration = gravity - (ground_normal * gravity.dot(ground_normal));

        float slope_dot = ground_normal.dot(Vector3::UP);

        
        //allow climbing easy slopes or ramps
        if (slope_dot < 0.9f && current_speed > 5.0f) {
            if (slope_dot < 0.0f) {  //handle uphill slopes
                Vector3 launch_dir = velocity.normalize();
                launch_dir.y = 1.0f - slope_dot;  //adjust for steepness
                velocity = launch_dir * current_speed;
                is_grounded = false;  //set as not grounded when moving up ramps
            }
        } else {
            //reset vertical velocity to 0 when grounded
            vertical_velocity = 0.0f;
        }
            
    }

    //update position
    Vector3 updated_position = target_position + velocity * seconds_elapsed;
    model.setTranslation(updated_position.x, updated_position.y, updated_position.z);
}