#include "ui.h"
#include "input.h"
#include "graphics/shader.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "game/world.h"
#include "framework/camera.h"

bool UI::addButton(Vector2 position, Vector2 size, const char* texture_path) {
    Vector2 mouse_pos = Input::mouse_position;
    bool is_hovered = false;
    bool was_pressed = false;

    // Mouse interaction
    if (mouse_pos.x > (position.x - size.x * 0.5f) &&
        mouse_pos.x < (position.x + size.x * 0.5f) &&
        mouse_pos.y > (position.y - size.y * 0.5f) &&
        mouse_pos.y < (position.y + size.y * 0.5f)) {
        is_hovered = true;
        was_pressed = Input::wasMousePressed(SDL_BUTTON_LEFT);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    Texture* texture = Texture::Get(texture_path);

    shader->enable();
    shader->setUniform("u_color", is_hovered ? Vector4::WHITE * 2.0f : Vector4::WHITE);
    shader->setUniform("u_model", Matrix44());
    shader->setUniform("u_viewprojection", World::get_instance()->camera->viewprojection_matrix);
    shader->setUniform("u_texture", texture, 0);

    Mesh quad;
    quad.createQuad(position.x, position.y, size.x, size.y, true);
    quad.render(GL_TRIANGLES);

    shader->disable();
    glEnable(GL_DEPTH_TEST);

    return was_pressed;
} 