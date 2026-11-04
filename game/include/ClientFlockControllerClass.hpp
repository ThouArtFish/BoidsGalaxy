#pragma once

#include <ModelClass.hpp>
#include "PlanetManagerClass.hpp"
#include "FlockControllerBase.hpp"

class ClientFlockController :  public FlockControllerBase {
public:
	ClientFlockController(size_t boid_count);
	void updateFlock(float dt) override;
	void updatePlanets(const PlanetManager& planets_manager) override;
	void drawAll(const TAGShaderManager::Shader& shader) override;
private:
	struct BoidData {
		size_t leaf = 0;
		float speed;
		glm::vec3 direction;
	};
	struct OctreeNode {
		size_t parent_index, depth, first_child_index = 0;
		TAGMesh::BoundingBox bounds, loose_bounds;
		std::vector<size_t> boid_indices, planet_indices;
		
		OctreeNode(size_t parent_index, size_t depth, const TAGMesh::BoundingBox& bounds);
		bool isLeaf() const { return !first_child_index; }
	};
	static constexpr float OCTREE_LOOSE_BOUNDS_FAC = 1.7f;
	static constexpr size_t OCTREE_MAX_BOIDS = 15;
	static constexpr size_t OCTREE_MAX_DEPTH = 7;

	TAGModel boid_model = TAGModel({}, TAGResourceManager::BufferAccess::STREAM);
	std::vector<PlanetManager::PlanetData> planet_data;
	std::vector<BoidData> boids_data;
	std::vector<OctreeNode> octree;
	std::vector<size_t> free_intervals;

	void generateOctree();
	size_t getBoidLeaf(glm::vec3 pos, size_t start_node) const;
	std::vector<size_t> getPlanetLeaf(glm::vec3 pos, float radius, size_t start_node) const;
	static TAGModel::Object pointDirection(const glm::vec3& new_direction);
	static std::array<TAGMesh::BoundingBox, 8> splitNode(const TAGMesh::BoundingBox& node);
};