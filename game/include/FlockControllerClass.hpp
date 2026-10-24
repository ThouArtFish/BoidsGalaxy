#pragma once

#include <ModelClass.hpp>

class FlockController {
public:
	float separation_factor = 0.5f, alignment_factor = 0.5f, cohesion_factor = 0.05f;
	float protected_range = 25.0f, visible_range = 625.0f;
	float border_factor = 0.2f;
	TAGMesh::BoundingBox game_bounds = { .max = glm::vec3(300.0f), .min = glm::vec3(-300.0f) };
	float min_speed = 20.0f, max_speed = 60.0f;

	FlockController(size_t boid_count);
	void updateFlock(float dt);
	void drawAll(const TAGShaderManager::Shader& shader);
private:
	struct BoidData {
		size_t leaf = 0;
		float speed;
		glm::vec3 direction;
	};
	struct OctreeNode {
		size_t parent_index, depth, first_child_index = 0;
		TAGMesh::BoundingBox bounds, loose_bounds;
		std::vector<size_t> boid_indices;
		
		OctreeNode(size_t parent_index, size_t depth, const TAGMesh::BoundingBox& bounds);
		bool isLeaf() const { return !first_child_index; }
	};
	inline static const std::string BOID_MESH_NAME = "boids";
	inline static const glm::vec3 START_DIR = glm::vec3(0.0f, 1.0f, 0.0f);
	static constexpr float BORDER_BOUNDS_FAC = 0.87f;
	static constexpr float OCTREE_LOOSE_BOUNDS_FAC = 1.7f;
	static constexpr size_t OCTREE_MAX_BOIDS = 15;
	static constexpr size_t OCTREE_MAX_DEPTH = 5;
	TAGModel boid_model = TAGModel({}, TAGResourceManager::BufferAccess::STREAM);
	std::vector<BoidData> boids_data;
	std::vector<OctreeNode> octree;
	std::vector<size_t> free_intervals;

	void generateOctree();
	size_t getBoidLeaf(glm::vec3 pos, size_t start_node) const;
	static TAGModel::Object pointDirection(const glm::vec3& new_direction);
	static std::array<TAGMesh::BoundingBox, 8> splitNode(const TAGMesh::BoundingBox& node);
};