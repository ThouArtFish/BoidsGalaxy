#pragma once

#include <ModelClass.hpp>

class FlockController {
public:
	float separation_factor = 0.3f, alignment_factor = 0.3f, cohesion_factor = 0.03f;
	float protected_range = 25.0f, visible_range = 2500.0f;
	float turn_factor = 0.2f;
	TAGMesh::BoundingBox game_bounds = { .max = glm::vec3(150.0f), .min = glm::vec3(-150.0f) };
	float min_speed = 20.0f, max_speed = 60.0f;

	FlockController(size_t boid_count);
	void updateFlock(float dt);
	void drawAll(const TAGShaderManager::Shader& shader);
private:
	struct BoidMovement {
		float speed;
		glm::vec3 direction;
	};
	struct OctreeNode {
		size_t id;
		TAGMesh::BoundingBox bounds;
		std::vector<size_t> indices;
		bool is_leaf;
	};
	inline static const std::string BOID_MESH_NAME = "boids";
	inline static const glm::vec3 START_DIR = glm::vec3(0.0f, 1.0f, 0.0f);
	static constexpr size_t OCTREE_MAX_BOIDS = 40;
	TAGModel boid_model = TAGModel({}, TAGResourceManager::BufferAccess::STREAM);
	std::vector<BoidMovement> boid_data;
	std::vector<OctreeNode> octree;

	void generateOctree();
	void recursiveOctreeStep(const std::vector<TAGModel::Object>& boids, TAGMesh::BoundingBox& bounds, std::vector<size_t>& indices, size_t& id);
	static TAGModel::Object pointDirection(const glm::vec3& old_direction, const glm::vec3& new_direction);
	static std::array<TAGMesh::BoundingBox, 8> splitNode(const TAGMesh::BoundingBox& node);
};