#pragma once

#include "framework/includes.h"
#include "framework/framework.h"
#include "framework/entities/entity.h"
#include "graphics/mesh.h"
#include "graphics/material.h"

class Camera;

class EntityMesh : public Entity {

public:
    EntityMesh();
    EntityMesh(Mesh* mesh, const Material& material);
    virtual ~EntityMesh();

    Mesh* mesh = nullptr;
    Material material;
    std::string name;

    // Miembros agregados para el renderizado instanciado
    bool isInstanced = false;
    std::vector<Matrix44> models;

    virtual void render(Camera* camera) override;
    virtual void update(float delta_time) override;
};
