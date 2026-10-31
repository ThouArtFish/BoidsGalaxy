#pragma once

#include <ResourceManagerClass.hpp>
#include <ShaderManagerClass.hpp>
#include <ModelClass.hpp>
#include <MeshClass.hpp>
#include <UtilClass.hpp>
#include "PingPongBufferClass.hpp"

class ComputeFlockController {
public:
	struct BoidConstants {
		GLfloat separation_factor = 0.5f, alignment_factor = 0.5f, cohesion_factor = 0.05f;
		GLfloat protected_range_sqr = 25.0f, visible_range_sqr = 900.0f;
		GLfloat border_factor = 0.2f;
		GLfloat min_speed = 20.0f, max_speed = 60.0f;
		alignas(16) glm::vec3 max_margin;
		GLuint total_boids;
		GLfloat dt;
	};
	struct BoidObject {
		glm::vec3 direction;
		GLfloat speed;
	};

	static constexpr float BOID_SCALE = 1.5f;
	inline static const glm::vec3 START_DIR = { 0.0f, 1.0f, 0.0f };
	inline static const std::string COMPUTE_BOID_UPDATE_NAME = "computer_boid_update";
	inline static const float GAME_BOUNDS_MARGIN = 0.87f;
	inline static const std::vector<TAGMesh::Vertex> MESH_VERTICES = {
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() }
	};
	inline static const std::vector<TAGMesh::Fragment> MESH_INDICES = { { 0, 1, 2 }, { 3, 4, 5 } };
	inline static const TAGMesh::Material MESH_MAT = { .colour = glm::vec3(1.0f, 0.0f, 0.0f) };
	inline static const glm::uvec3 CELLS_PER_AXIS = { 10, 10, 10 };

	BoidConstants constants;

	ComputeFlockController(size_t boid_count);
	void updateFlock(float dt);
	void drawAll(const TAGShaderManager::Shader& shader);
private:
	TAGMesh boid_mesh{ MESH_VERTICES, MESH_INDICES, MESH_MAT };
	TAGShaderManager shaders{};

	PingPongBuffer<TAGModel::ShaderObject> boids_instance;
	PingPongBuffer<BoidObject> boids_data;
	TAGResourceManager::UnmappedBuffer<BoidConstants> boids_constants;
	TAGMesh::BoundingBox game_bounds = { .max = glm::vec3(350.0f), .min = glm::vec3(-350.0f) };
};
