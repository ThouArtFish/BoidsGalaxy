#pragma once

#include <ModelClass.hpp>

class FlockController {
public:
	float separation_factor = 0.5f, alignment_factor = 0.5f, cohesion_factor = 0.5f;
	float protected_range = 0.25f, visible_range = 10000.0f;
	float turn_factor = 0.2f;
	TAGMesh::BoundingBox game_bounds = { .max = glm::vec3(50.0f), .min = glm::vec3(-50.0f)};
	float min_speed, max_speed;

	FlockController(size_t boid_count, float boid_min_speed, float boid_max_speed);
	void updateFlock(float dt);
	void drawAll(const TAGShaderManager::Shader& shader);
private:
	struct BoidMovement {
		float speed;
		glm::vec3 direction;
	};
	inline static const std::string BOID_MESH_NAME = "boids";
	inline static const glm::vec3 START_DIR = glm::vec3(0.0f, 1.0f, 0.0f);
	TAGModel boid_model = TAGModel({}, TAGResourceManager::BufferAccess::STREAM);
	std::vector<BoidMovement> boid_data;

	static TAGModel::Object pointDirection(const glm::vec3& old_direction, const glm::vec3& new_direction);
};