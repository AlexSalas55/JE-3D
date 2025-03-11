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
    //player->model.setTranslation(00.0f, 200.0f, 0.0f); //mapa pepino
    //player->model.setTranslation(372.0f, 700.0f, 230.0f); //two marios per separat
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
        // skybox->model.scale(5000.0f, 5000.0f, 5000.0f); //Para escalar el mapa? TODO

        // Create second skybox for player 2 (if multiplayer is enabled)
        if (Game::instance->multiplayer_enabled) {
            // We can reuse the same texture and material for the second skybox
            skybox2 = new EntityMesh(Mesh::Get("data/meshes/cubemap.ASE"), cubemap_material);
        }
    }

    //timer
    time = 0.0f;

    // Load scene
    SceneParser parser;
    bool ok = parser.parse("data/myscene.scene", root);
    assert(ok);

    // Initialize Phong shader and default material
    phong_shader = Shader::Get("data/shaders/phong.vs", "data/shaders/phong.fs");
    
    // Set up default light - position it high above the starting point for better visibility
    light_position = Vector3(345.0f, 500.0f, 37.0f);  // Above starting position
    light_color = Vector3(0.95f, 0.92f, 0.9f);  // Slightly dimmer, warmer light for snow
    
    // Set up second light between portal fall and finish line
    light2_position = Vector3(600.0f, 400.0f, 200.0f);  // Positioned between portal and finish
    light2_color = Vector3(0.85f, 0.9f, 1.0f);  // Slightly cooler light for contrast
}

void World::render() {
    // Determine which camera to use based on viewport
    Camera* current_camera = camera;
    EntityMesh* current_skybox = skybox;
    
    // If in multiplayer mode and rendering the right viewport, use camera2 and skybox2
    if (Game::instance->multiplayer_enabled) {
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        // If this is the right viewport (x > 0), use camera2 and skybox2
        if (viewport[0] > 0 && Game::instance->camera2) {
            current_camera = Game::instance->camera2;
            current_skybox = skybox2;
        }
    }
    
    // Set the camera as default
    current_camera->enable();
    
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    if(current_skybox)
       current_skybox->render(current_camera);
    
    glEnable(GL_DEPTH_TEST);
    
    // Draw the floor grid
    //drawGrid();
    
    // Set light parameters for Phong shader before rendering scene
    if (phong_shader) {
        phong_shader->enable();
        
        // Update light position to follow player for better visibility
        light_position = player->model.getTranslation() + Vector3(0, 300.0f, 0);
        
        phong_shader->setUniform3("u_light_position", light_position.x, light_position.y, light_position.z);
        phong_shader->setUniform3("u_light_color", light_color.x, light_color.y, light_color.z);
        phong_shader->setUniform3("u_light2_position", light2_position.x, light2_position.y, light2_position.z);
        phong_shader->setUniform3("u_light2_color", light2_color.x, light2_color.y, light2_color.z);
        phong_shader->setUniform3("u_camera_position", current_camera->eye.x, current_camera->eye.y, current_camera->eye.z);
        
        // Set default material parameters - adjusted for snow
        phong_shader->setUniform1("u_ambient", 0.45f);
        phong_shader->setUniform1("u_diffuse", 0.6f);
        phong_shader->setUniform1("u_specular", 0.2f);
        phong_shader->setUniform1("u_shininess", 32.0f);
        
        phong_shader->disable();
    }
    
    // Set the same lighting parameters for the player's skinning_phong shader
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
        // Update players
        player->update(seconds_elapsed);
        
        if (Game::instance->multiplayer_enabled && player2) {
            // Handle Player 2 controls (H and K for camera rotation)
            if (Input::isKeyPressed(SDL_SCANCODE_LEFT))
                camera2_yaw -= seconds_elapsed * rotation_speed;
            if (Input::isKeyPressed(SDL_SCANCODE_RIGHT))
                camera2_yaw += seconds_elapsed * rotation_speed;
            /*                
            if (Input::isKeyPressed(SDL_SCANCODE_I))
                camera2_pitch += seconds_elapsed * rotation_speed;
            if (Input::isKeyPressed(SDL_SCANCODE_J))
                camera2_pitch -= seconds_elapsed * rotation_speed;
                */
            // Update Player 2
            player2->update(seconds_elapsed);
        }

        // Player 1 camera controls with arrow keys
        if (Input::isKeyPressed(SDL_SCANCODE_A))
            camera_yaw -= seconds_elapsed * rotation_speed;
        if (Input::isKeyPressed(SDL_SCANCODE_D))
            camera_yaw += seconds_elapsed * rotation_speed;

            /*
        if (Input::isKeyPressed(SDL_SCANCODE_DOWN))
            camera_pitch -= seconds_elapsed * rotation_speed;
        if (Input::isKeyPressed(SDL_SCANCODE_UP))
            camera_pitch += seconds_elapsed * rotation_speed;
            */

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
            // Configuración de la cámara en tercera persona
            float orbit_dist = 6.0f;  // Distancia de la cámara al jugador
            
            // Calcular la posición deseada de la cámara
            center = player_pos + Vector3(0.f, 0.8f, 0.0f); // Punto de mira ajustado
            Vector3 target_eye = player_pos - front * orbit_dist + Vector3(0.0f, 1.5f, 0.0f); // Posición deseada de la cámara
            
            // Aplicar detección de colisiones para ajustar la posición de la cámara
            eye = adjustCameraPosition(target_eye, center, 0.5f);
            
            // Asegurarse de que la cámara no esté demasiado cerca del jugador
            float min_distance = 2.0f;
            float current_distance = (eye - center).length();
            if (current_distance < min_distance) {
                Vector3 dir = (eye - center).normalize();
                eye = center + dir * min_distance;
            }
        }

        // Actualizar la cámara con las nuevas posiciones
        camera->lookAt(eye, center, Vector3(0, 1, 0));
        /*
        // Add roll for player 1
        Matrix44 rollMatrix;
        rollMatrix.setRotation(camera_roll * DEG2RAD, camera->center - camera->eye);
        camera->up = rollMatrix.rotateVector(Vector3(0,1,0));
        */

        // Handle camera for player 2 in multiplayer mode
        if (Game::instance->multiplayer_enabled && player2) {
            // Use the same camera pitch as player 1 for consistent behavior
            //camera2_pitch = camera_pitch;
            
            Matrix44 mYaw2;
            mYaw2.setRotation(camera2_yaw, Vector3(0, 1, 0));
            Matrix44 mPitch2;
            mPitch2.setRotation(camera2_pitch, Vector3(-1, 0, 0));
            
            Vector3 front2 = (mPitch2 * mYaw2).frontVector().normalize();
            Vector3 player2_pos = player2->model.getTranslation();
            
            // Third-person camera for player 2
            float orbit_dist2 = 6.0f;
            center2 = player2_pos + Vector3(0.f, 0.8f, 0.0f);
            Vector3 target_eye2 = player2_pos - front2 * orbit_dist2 + Vector3(0.0f, 1.5f, 0.0f);
            // Aplicar detección de colisiones para ajustar la posición de la cámara del jugador 2
            eye2 = adjustCameraPosition(target_eye2, center2, 0.5f);
            
            // Asegurarse de que la cámara no esté demasiado cerca del jugador 2
            float min_distance2 = 2.0f;
            float current_distance2 = (eye2 - center2).length();
            if (current_distance2 < min_distance2) {
                Vector3 dir2 = (eye2 - center2).normalize();
                eye2 = center2 + dir2 * min_distance2;
            }
            // Actualizar la cámara del jugador 2
            Game::instance->camera2->lookAt(eye2, center2, Vector3(0, 1, 0));
            /*
            // Add roll for player 2
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
    collision.distance = max_ray_dist; // Inicializar con la distancia máxima

    for (auto e : root->children) {
        //Ignorar al jugador para evitar colisiones con si mismo
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

Vector3 World::adjustCameraPosition(const Vector3& target_eye, const Vector3& target_center, float min_distance) {
    // Si no estamos en el escenario de entrenamiento, devolver la posición original sin ajustes
    if (!is_training_stage) {
        return target_eye;
    }
    
    // Dirección desde el centro (jugador) hacia la cámara
    Vector3 dir = (target_eye - target_center).normalize();
    
    // Distancia deseada
    float desired_distance = (target_eye - target_center).length();
    
    // Añadir un pequeño offset al origen para evitar colisiones con el propio jugador
    Vector3 adjusted_origin = target_center + Vector3(0, 0.5f, 0);
    
    // Verificar si la posición deseada de la cámara está dentro de alguna malla
    if (isPointInsideMesh(target_eye)) {
        std::cout << "Camera target position is inside a mesh!" << std::endl;
        
        // Realizar un raycast desde el centro hacia la posición deseada de la cámara
        sCollisionData collision = raycast(adjusted_origin, dir, eCollisionFilter::ALL, true, desired_distance);
        
        if (collision.collider) {
            std::cout << "Camera collision detected! Distance: " << collision.distance << std::endl;
            
            // Calcular la nueva distancia, dejando un pequeño margen para no estar exactamente en la superficie
            float new_distance = collision.distance * 0.85f; // Reducir un 15% para alejarse de la colisión (más suave)
            
            // Asegurarse de que la distancia no sea demasiado pequeña
            new_distance = std::max(new_distance, 2.0f);
            
            // Calcular la nueva posición de la cámara con una elevación adicional
            // Añadimos un componente vertical para elevar la cámara y obtener una vista más aérea
            Vector3 up_offset = Vector3(0, 1.0f, 0) * (desired_distance - new_distance) * 0.5f;
            Vector3 adjusted_eye = adjusted_origin + dir * new_distance + up_offset;
                        
            // Verificar que la nueva posición no esté dentro de ninguna malla
            if (isPointInsideMesh(adjusted_eye)) {
                // Si aún está dentro, intentar acercar la cámara al jugador con diferentes ángulos
                std::cout << "Camera still inside mesh after adjustment!" << std::endl;
                
                // Intentar diferentes distancias y elevaciones hasta encontrar una posición válida
                for (float factor = 0.7f; factor >= 0.1f; factor -= 0.1f) {
                    new_distance = desired_distance * factor;
                    // Aumentar la elevación a medida que nos acercamos al jugador
                    float elevation_factor = (1.0f - factor) * 2.0f;
                    Vector3 higher_offset = Vector3(0, 1.5f, 0) * elevation_factor;
                    adjusted_eye = adjusted_origin + dir * new_distance + higher_offset;
                    
                    if (!isPointInsideMesh(adjusted_eye)) {
                        std::cout << "Found valid camera position at factor: " << factor << std::endl;
                        return adjusted_eye;
                    }
                }
                
                // Si no se encuentra una posición válida, usar una posición muy cercana al jugador con vista aérea
                return adjusted_origin + dir * 1.5f + Vector3(0, 2.0f, 0);
            }
            
            return adjusted_eye;
        }
    }
    
    // Si no hay colisión o el punto no está dentro de ninguna malla, devolver la posición original
    // con una ligera tendencia a elevarse para mantener consistencia
    Vector3 slight_elevation = Vector3(0, 0.2f, 0);
    return target_eye + slight_elevation;
}

bool World::isPointInsideMesh(const Vector3& point, float radius) {
    // Comprobar si el punto está dentro de alguna malla usando test_scene_collisions
    std::vector<sCollisionData> collisions;
    std::vector<sCollisionData> ground_collisions;
    
    // Crear una esfera alrededor del punto para detectar colisiones
    Vector3 sphere_center = point;
    
    // Comprobar colisiones con la esfera
    test_scene_collisions(sphere_center, collisions, ground_collisions, eCollisionFilter::ALL);
    
    // Si hay colisiones, el punto está dentro de alguna malla
    if (!collisions.empty()) {
        std::cout << "Point is inside mesh! Collisions: " << collisions.size() << std::endl;
        return true;
    }
    
    return false;
}