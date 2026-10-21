#pragma once

#include <ModelClass.hpp>

class PlanetManager {
public:
	size_t planet_vertical_strips = 15, planet_horizontal_strips = 30, num_planets;
	float min_radius = 30.0f, max_radius = 55.0f;
	TAGMesh::BoundingBox game_bounds = { .max = glm::vec3(160.0f), .min = glm::vec3(-160.0f) };

	PlanetManager(size_t num_planets, const glm::vec3& arm = { 0.0f, 1.0f, 0.0f }, const glm::vec3& axis = { 1.0f, 0.0f, 0.0f });
	void generatePlanets();
	void drawAll(const TAGShaderManager::Shader& shader);
private:
	inline static const std::string PLANET_MESH_NAME = "planet";
	TAGModel planet_model = TAGModel({}, TAGResourceManager::BufferAccess::STATIC);
};