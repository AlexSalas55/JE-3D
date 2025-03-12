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
}

void EntityMesh::render(Camera* camera)
{

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

	// propagate the call to render to the children using the base class method
	Entity::render(camera);
}

void EntityMesh::update(float delta_time)
{

	// propagate the call to update to the children using the base class method
	Entity::update(delta_time);
}
