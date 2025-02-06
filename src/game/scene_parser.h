#pragma once

#include "framework/entities/entity.h"
#include "framework/framework.h"
#include "graphics/material.h"

#include <map>
#include <vector>

class SceneParser {

	std::map<std::string, sRenderData> meshes_to_load;

public:
	bool parse(const char* filename, Entity* root);
};