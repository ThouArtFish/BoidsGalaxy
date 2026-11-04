#pragma once

#include <ModelClass.hpp>

class PlanetManager {
public:
	struct PlanetData {
		glm::vec3 position;
		float orbit_radius;
	};
	inline static size_t planet_vertical_strips = 15, planet_horizontal_strips = 30;
	inline static float min_radius = 50.0f, max_radius = 65.0f;
	inline static float orbit_radius_factor = 1.3f;
	inline static float orbit_pull = 1.5f;
	inline static float orbit_pull_range = 10.0f;
	inline static float tangent_boost = 1.5f;
	size_t num_planets;

	PlanetManager(size_t num_planets, const glm::vec3& arm = { 0.0f, 1.0f, 0.0f }, const glm::vec3& axis = { 1.0f, 0.0f, 0.0f });
	void generatePlanets(const TAGMesh::BoundingBox& bounds);
	std::vector<PlanetData> getPlanetData() const;
	void drawAll(const TAGShaderManager::Shader& shader);
private:
	inline static const std::string PLANET_MESH_NAME = "planet";
	TAGModel planet_model = TAGModel({}, TAGResourceManager::BufferAccess::STATIC);
};