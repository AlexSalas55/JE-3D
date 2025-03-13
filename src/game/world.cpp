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
    player_material.shader = Shader::Get("data/shaders/skinning_phong.vs", "data/shaders/skinning_phong.fs");
    player_material.diffuse = Texture::Get("data/meshes/playerColor.png");
    player_material.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);  // Set base color to white
    player = new Player(Mesh::Get("data/meshes/player.mesh"), player_material, "player");
    player->model.setTranslation(345.0f, 184.0f, 37.0f); //merged marios
    

    root->addChild(player);
    
    // Create second player for multiplayer
    if (Game::instance->multiplayer_enabled) {
        player2 = new Player(Mesh::Get("data/meshes/player.mesh"), player_material, "player2");
        player2->model.setTranslation(5.0f, 200.0f, 0.0f); // Position player2 next to player1
        root->addChild(player2);
        
        // Setup second camera
        Game::instance->camera2 = new Camera();
        Game::instance->camera2->lookAt(Vector3(0.f, 2.f, -5.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f));
        Game::instance->camera2->setPerspective(65.f, Game::instance->window_width/(float)Game::instance->window_height, 0.1f, 3000.f);
    }
    
    {
        /////////////////////// Create heightmap DISABLED ///////////////////////    

        // Create heightmap - reducir el tamaño
        // float size = 2500.0f;  // Cambiado de 500.0f a 50.0f

        // Mesh* heightmap_mesh = new Mesh();
        // heightmap_mesh->createSubdividedPlane(size, 256);

        // Material heightmap_material = {
        //     Shader::Get("data/shaders/height_map.vs", "data/shaders/height_map.fs"),
        //     Vector4(1.0f, 1.0f, 1.0f, 1.0f),
        //     Texture::Get("data/textures/heightmap.png")
        // };

        // EntityMesh* heightmap = new EntityMesh(heightmap_mesh, heightmap_material);
        // heightmap->model.translate(-size * 0.5f, 0.0f, -size * 0.5f);
        // root->addChild(heightmap);
    }
    
    {
        // Create skybox environment for player 1
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

        // Create second skybox for player 2 (if multiplayer is enabled)
        if (Game::instance->multiplayer_enabled) {
            // reuse same material
            skybox2 = new EntityMesh(Mesh::Get("data/meshes/cubemap.ASE"), cubemap_material);
        }
    }

    //timer
    time = 0.0f;

    // Load scene
    SceneParser parser;
    bool ok = parser.parse("data/myscene.scene", root);
    assert(ok);

    // Initialize phong shader
    phong_shader = Shader::Get("data/shaders/phong.vs", "data/shaders/phong.fs");
    
    // Set up default light
    light_position = Vector3(345.0f, 500.0f, 37.0f);  // Above starting position
    light_color = Vector3(0.95f, 0.92f, 0.9f);
    
    // second light
    light2_position = Vector3(600.0f, 400.0f, 200.0f);  // Positioned between portal and finish
    light2_color = Vector3(0.85f, 0.9f, 1.0f);
}

void World::render() {
    // determine which camera to use
    Camera* current_camera = camera;
    EntityMesh* current_skybox = skybox;
    
    // if in multiplayer mode use camera2 and skybox2
    if (Game::instance->multiplayer_enabled) {
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        // If this is the right viewport (x > 0), use camera2 and skybox2
        if (viewport[0] > 0 && Game::instance->camera2) {
            current_camera = Game::instance->camera2;
            current_skybox = skybox2;
        }
    }
    
    // set the camera as default
    current_camera->enable();
    
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    if(current_skybox)
       current_skybox->render(current_camera);
    
    glEnable(GL_DEPTH_TEST);
    
    // set light parameters for phong
    if (phong_shader) {
        phong_shader->enable();
        
        // update light position to follow player
        light_position = player->model.getTranslation() + Vector3(0, 300.0f, 0);
        
        phong_shader->setUniform3("u_light_position", light_position.x, light_position.y, light_position.z);
        phong_shader->setUniform3("u_light_color", light_color.x, light_color.y, light_color.z);
        phong_shader->setUniform3("u_light2_position", light2_position.x, light2_position.y, light2_position.z);
        phong_shader->setUniform3("u_light2_color", light2_color.x, light2_color.y, light2_color.z);
        phong_shader->setUniform3("u_camera_position", current_camera->eye.x, current_camera->eye.y, current_camera->eye.z);
        
        // set default material parameters
        phong_shader->setUniform1("u_ambient", 0.45f);
        phong_shader->setUniform1("u_diffuse", 0.6f);
        phong_shader->setUniform1("u_specular", 0.2f);
        phong_shader->setUniform1("u_shininess", 32.0f);
        
        phong_shader->disable();
    }
    
    // set the same lighting for the player phong shader
    Shader* player_shader = player->material.shader;
    if (player_shader) {
        player_shader->enable();
        player_shader->setUniform3("u_light_position", light_position.x, light_position.y, light_position.z);
        player_shader->setUniform3("u_light_color", light_color.x, light_color.y, light_color.z);
        player_shader->setUniform3("u_light2_position", light2_position.x, light2_position.y, light2_position.z);
        player_shader->setUniform3("u_light2_color", light2_color.x, light2_color.y, light2_color.z);
        player_shader->setUniform3("u_camera_position", current_camera->eye.x, current_camera->eye.y, current_camera->eye.z);
        player_shader->setUniform1("u_ambient", 0.45f);
        player_shader->setUniform1("u_diffuse", 0.6f);
        player_shader->setUniform1("u_specular", 0.2f);
        player_shader->setUniform1("u_shininess", 32.0f);
        player_shader->disable();
    }
    
    // Render scene
    root->render(current_camera);
}

void World::update(double seconds_elapsed) {
    time += seconds_elapsed;

    // toggle free camera
    if (Input::wasKeyPressed(SDL_SCANCODE_C)) {
        free_camera = !free_camera;
        if (free_camera) {
            // store current camera position and orientation
            camera_stored_eye = camera->eye;
            camera_stored_center = camera->center;
            camera_stored_up = camera->up;
        } else {
            // restore camera position
            camera->lookAt(camera_stored_eye, camera_stored_center, camera_stored_up);
        }
    }

    if (free_camera) {
        float speed = seconds_elapsed * camera_speed;
        
        // async input to move the camera around
        if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) 
            speed *= 10;

        Vector3 forward = (camera->center - camera->eye).normalize();
        Vector3 right = forward.cross(camera->up).normalize();
        Vector3 up = Vector3(0, 1, 0);

        // update camera movement based on its orientation
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

        // mouse look
        camera_yaw -= Input::mouse_delta.x * 0.005f;
        camera_pitch += Input::mouse_delta.y * 0.005f;
        
        // limit pitch to avoid camera flipping
        camera_pitch = clamp(camera_pitch, -M_PI * 0.4f, M_PI * 0.4f);
        
        // calculate new front vector
        Vector3 front;
        front.x = cos(camera_yaw) * cos(camera_pitch);
        front.y = sin(camera_pitch);
        front.z = sin(camera_yaw) * cos(camera_pitch);
        front = front.normalize();
        
        camera->center = camera->eye + front;
        camera->updateViewMatrix();
    }
    else {
        // update players
        player->update(seconds_elapsed);
        
        if (Game::instance->multiplayer_enabled && player2) {
            // Handle Player 2 controls - both keyboard and gamepad
            // Keyboard controls (left/right for camera)
            if (Input::isKeyPressed(SDL_SCANCODE_LEFT) || 
                (Input::gamepads[1].connected && Input::gamepads[1].axis[RIGHT_ANALOG_X] < -0.3f))
                camera2_yaw -= seconds_elapsed * rotation_speed;
            if (Input::isKeyPressed(SDL_SCANCODE_RIGHT) || 
                (Input::gamepads[1].connected && Input::gamepads[1].axis[RIGHT_ANALOG_X] > 0.3f))
                camera2_yaw += seconds_elapsed * rotation_speed;
            
            // Update Player 2
            player2->update(seconds_elapsed);
        }

        // player 1 camera controls with arrow keys
        if (Input::isKeyPressed(SDL_SCANCODE_A) || 
            (Input::gamepads[0].connected && Input::gamepads[0].axis[RIGHT_ANALOG_X] < -0.3f))
            camera_yaw -= seconds_elapsed * rotation_speed;
        if (Input::isKeyPressed(SDL_SCANCODE_D) || 
            (Input::gamepads[0].connected && Input::gamepads[0].axis[RIGHT_ANALOG_X] > 0.3f))
            camera_yaw += seconds_elapsed * rotation_speed;

        // keep pitch within limits
        camera_pitch = clamp(camera_pitch, -M_PI * 0.4f, M_PI * 0.4f);
        Matrix44 mYaw;
        mYaw.setRotation(camera_yaw, Vector3(0, 1, 0));
        Matrix44 mPitch;
        mPitch.setRotation(camera_pitch, Vector3(-1, 0, 0));

        Vector3 front = (mPitch * mYaw).frontVector().normalize();
        Vector3 player_pos = player->model.getTranslation();

        // first person camera
        if (use_first_person) {
            Vector3 camera_height = Vector3(0.0f, 1.5f, 0.0f);
            eye = player_pos + camera_height;
            center = eye + front;
        }
        else {
            // third person camera
            float orbit_dist = 6.0f;
            
            // calculate desired camera position
            center = player_pos + Vector3(0.f, 0.8f, 0.0f);
            Vector3 target_eye = player_pos - front * orbit_dist + Vector3(0.0f, 1.5f, 0.0f);
            
            // apply collision detection to adjust camera position
            eye = adjustCameraPosition(target_eye, center, 0.5f);
            
            // ensure camera is not too close to player
            float min_distance = 2.0f;
            float current_distance = (eye - center).length();
            if (current_distance < min_distance) {
                Vector3 dir = (eye - center).normalize();
                eye = center + dir * min_distance;
            }
        }

        // update camera with new positions
        camera->lookAt(eye, center, Vector3(0, 1, 0));

        // handle camera for player 2 in multiplayer mode
        if (Game::instance->multiplayer_enabled && player2) {
            // use same camera pitch as player 1 for consistent behavior
            //camera2_pitch = camera_pitch;
            
            Matrix44 mYaw2;
            mYaw2.setRotation(camera2_yaw, Vector3(0, 1, 0));
            Matrix44 mPitch2;
            mPitch2.setRotation(camera2_pitch, Vector3(-1, 0, 0));
            
            Vector3 front2 = (mPitch2 * mYaw2).frontVector().normalize();
            Vector3 player2_pos = player2->model.getTranslation();
            
            // third-person camera for player 2
            float orbit_dist2 = 6.0f;
            center2 = player2_pos + Vector3(0.f, 0.8f, 0.0f);
            Vector3 target_eye2 = player2_pos - front2 * orbit_dist2 + Vector3(0.0f, 1.5f, 0.0f);
            // apply collision detection to adjust camera position
            eye2 = adjustCameraPosition(target_eye2, center2, 0.5f);
            
            // ensure camera is not too close to player 2
            float min_distance2 = 2.0f;
            float current_distance2 = (eye2 - center2).length();
            if (current_distance2 < min_distance2) {
                Vector3 dir2 = (eye2 - center2).normalize();
                eye2 = center2 + dir2 * min_distance2;
            }
            // update player 2 camera
            Game::instance->camera2->lookAt(eye2, center2, Vector3(0, 1, 0));
            /*
            // add roll for player 2
            Matrix44 rollMatrix2;
            rollMatrix2.setRotation(camera2_roll * DEG2RAD, Game::instance->camera2->center - Game::instance->camera2->eye);
            Game::instance->camera2->up = rollMatrix2.rotateVector(Vector3(0,1,0));
            */
        }
    }

    // move skybox to follow player 1's camera
    skybox->model.setTranslation(camera->eye);
    
    // move skybox2 to follow player 2's camera if multiplayer is enabled
    if (Game::instance->multiplayer_enabled && skybox2 && Game::instance->camera2) {
        skybox2->model.setTranslation(Game::instance->camera2->eye);
    }

    // delete pending entities
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
    collision.distance = max_ray_dist; // initialize with max distance

    for (auto e : root->children) {
        // ignore player to avoid collisions with itself
        if (e == player || e == player2) {
            continue;
        }
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

        // there was a collision! update if nearest..
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

Vector3 World::adjustCameraPosition(const Vector3& target_eye, const Vector3& target_center, float min_distance) {
    // if not in training stage, return original position without adjustments
    if (!is_training_stage) {
        return target_eye;
    }
    
    // direction from center to camera
    Vector3 dir = (target_eye - target_center).normalize();
    
    // desired distance
    float desired_distance = (target_eye - target_center).length();
    
    // add a small offset to the origin to avoid collisions with the player itself
    Vector3 adjusted_origin = target_center + Vector3(0, 0.5f, 0);
    
    // check if desired camera position is inside any mesh
    if (isPointInsideMesh(target_eye)) {
        //std::cout << "Camera target position is inside a mesh!" << std::endl;
        
        // perform a raycast from center camera position
        sCollisionData collision = raycast(adjusted_origin, dir, eCollisionFilter::ALL, true, desired_distance);
        
        if (collision.collider) {
            //std::cout << "Camera collision detected! Distance: " << collision.distance << std::endl;
            
            // Calculate new distance, with small gap
            float new_distance = collision.distance * 0.85f;
            
            // ensure distance is not too small
            new_distance = std::max(new_distance, 2.0f);
            
            // calculate new camera position with additional elevation
            Vector3 up_offset = Vector3(0, 1.0f, 0) * (desired_distance - new_distance) * 0.5f;
            Vector3 adjusted_eye = adjusted_origin + dir * new_distance + up_offset;
                        
            // check if new position is inside any mesh
            if (isPointInsideMesh(adjusted_eye)) {
                //std::cout << "Camera still inside mesh after adjustment!" << std::endl;
                
                // try different distances and elevations until a valid position is found
                for (float factor = 0.7f; factor >= 0.1f; factor -= 0.1f) {
                    new_distance = desired_distance * factor;
                    // increase elevation as we get closer to the player
                    float elevation_factor = (1.0f - factor) * 2.0f;
                    Vector3 higher_offset = Vector3(0, 1.5f, 0) * elevation_factor;
                    adjusted_eye = adjusted_origin + dir * new_distance + higher_offset;
                    
                    if (!isPointInsideMesh(adjusted_eye)) {
                        //std::cout << "Found valid camera position at factor: " << factor << std::endl;
                        return adjusted_eye;
                    }
                }
                
                // if no valid position is found, use a position very close to the player
                return adjusted_origin + dir * 1.5f + Vector3(0, 2.0f, 0);
            }
            
            return adjusted_eye;
        }
    }
    
    // if there is no collision or the point is not inside any mesh, return the original position
    Vector3 slight_elevation = Vector3(0, 0.2f, 0);
    return target_eye + slight_elevation;
}

bool World::isPointInsideMesh(const Vector3& point, float radius) {
    // check if the point is inside any mesh using test_scene_collisions
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;
    
    // create a sphere around the point to detect collisions
    Vector3 sphere_center = point;
    
    // check collisions with the sphere
    test_scene_collisions(sphere_center, collisions, ground_collisions, eCollisionFilter::ALL);
    
    // if there are collisions, the point is inside a mesh
    if (!collisions.empty()) {
        //std::cout << "Point is inside mesh! Collisions: " << collisions.size() << std::endl;
        return true;
    }
    
    return false;
}