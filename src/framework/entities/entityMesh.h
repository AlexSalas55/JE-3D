#pragma once

#include "framework/includes.h"
#include "framework/framework.h"
#include "framework/entities/entity.h"
#include "graphics/mesh.h"
#include "graphics/material.h"
#include "framework/animation.h"

class Camera;

class EntityMesh : public Entity {

public:
    EntityMesh() : mesh(nullptr) {}
    EntityMesh(Mesh* mesh, const Material& material);
    //EntityMesh(Mesh* mesh, const Material& material, const std::string& name = "");

    virtual ~EntityMesh();

    Mesh* mesh = nullptr;
    Material material;
    std::string name;

    // animations
    Animator animator;
    bool isAnimated = false;

    // Miembros agregados para el renderizado instanciado
    bool isInstanced = false;
    std::vector<Matrix44> models;

    virtual void render(Camera* camera) override;
    virtual void update(float delta_time) override;
};