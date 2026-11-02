#pragma once

#include <MeshClass.hpp>
#include "QueryManagerClass.hpp"

class Timer {
public:
	double update_avg = 0.0;
	inline void addUpdateTime(double dt) {
		update_count += 1.0;
		update_total += dt;
		if (update_count >= UPDATE_FRAMES) {
			update_avg = update_total / update_count;
			update_total = 0.0f;
			update_count = 0.0f;
		}
	}
private:
	static constexpr double UPDATE_FRAMES = 10.0;
	double update_total = 0.0;
	double update_count = 0.0;
};

struct FlockControllerBase {
	inline static const std::vector<TAGMesh::Vertex> MESH_VERTICES = {
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() }
	};
	inline static const std::vector<TAGMesh::Fragment> MESH_INDICES = { { 0, 2, 1 }, { 3, 4, 5 } };
	inline static const TAGMesh::Material MESH_MAT = { .colour = glm::vec3(1.0f, 0.0f, 0.0f) };
	inline static const std::string BOID_MESH_NAME = "boids";
	inline static const glm::vec3 START_DIR = glm::vec3(0.0f, 1.0f, 0.0f);
	static constexpr size_t MIN_BOIDS = 10;
	static constexpr size_t MAX_BOIDS = 10000;
	static constexpr float UPDATE_FRAMES = 60.0f;

	float separation_factor = 0.5f, alignment_factor = 0.5f, cohesion_factor = 0.05f;
	float protected_range = 5.0f, visible_range = 30.0f;
	float border_factor = 0.2f;
	float border_start_factor = 0.87f;
	TAGMesh::BoundingBox game_bounds = { .max = glm::vec3(350.0f), .min = glm::vec3(-350.0f) };
	float min_speed = 20.0f, max_speed = 60.0f;
	float boid_scale = 1.5f;
	Timer update_timer{}, render_timer{};
	QueryManager query_manager{};
	size_t boid_count;

	virtual ~FlockControllerBase() = default;
	FlockControllerBase(size_t boid_count) : boid_count(glm::clamp(boid_count, MIN_BOIDS, MAX_BOIDS)) {};
	virtual void updateFlock(float dt) = 0;
	virtual void drawAll(const TAGShaderManager::Shader& shader) = 0;
};
