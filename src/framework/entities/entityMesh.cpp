#include "entityMesh.h"
#include "framework/camera.h"

#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/material.h"


EntityMesh::EntityMesh(Mesh* mesh, const Material& material)
	: mesh(mesh), material(material)
{
}

EntityMesh::~EntityMesh() {
	// Libera recursos si fuese necesario
}

void EntityMesh::render(Camera* camera)
{
	/*
	if (!mesh) {
		assert(0);
		return;
	}
	if (!material.shader) {
		material.shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
	}*/

	camera->enable();

	Shader* shader = material.shader;
	shader->enable();

	shader->setUniform("u_model", getGlobalMatrix());
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_color", material.color);

	if (!material.shader || !mesh) return;

	// Enable shader and pass uniforms
	material.shader->enable();

	// Set standard uniforms
	material.shader->setUniform("u_model", getGlobalMatrix());
	material.shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	
	// Set material color
	material.shader->setUniform("u_color", material.color);

	// Set texture if available
	if (material.diffuse) {
		material.shader->setUniform("u_texture", material.diffuse, 0);
	}
	
	if (isAnimated) {
		mesh->renderAnimated(GL_TRIANGLES, &animator.getCurrentSkeleton());
	} else {
		mesh->render(GL_TRIANGLES);
	}


	// Render the mesh
	mesh->render(GL_TRIANGLES);

	// Disable shader
	material.shader->disable();

	// Propaga la llamada a render a los hijos invocando el método de la clase base
	Entity::render(camera);
}

void EntityMesh::update(float delta_time)
{
	// Aquí se puede actualizar la lógica específica de EntityMesh

	// Propaga la llamada a update a los hijos mediante el método de la clase base
	Entity::update(delta_time);
}
