#include "../include/FlockControllerClass.hpp"

FlockController::FlockController(size_t boid_count) {
	static const std::vector<TAGMesh::Vertex> vertices = {
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() }
	};
	static const std::vector<TAGMesh::Fragment> fragments = { { 0, 1, 2 }, { 3, 4, 5 } };
	const float start_speed = (max_speed - min_speed) / 2.0f;

	boid_model.addMesh(BOID_MESH_NAME, vertices, fragments, { .colour = glm::vec3(1.0f, 0.0f, 0.0f) });
	boids_data.reserve(boid_count);
	for (size_t i = 0; i < boid_count; i++) {
		const glm::vec3 spawn_pos = {
			TAGUtil::randomRangeFloat(game_bounds.min.x, game_bounds.max.x),
			TAGUtil::randomRangeFloat(game_bounds.min.y, game_bounds.max.y),
			TAGUtil::randomRangeFloat(game_bounds.min.z, game_bounds.max.z)
		};

		glm::vec3 dir = {
			TAGUtil::randomRangeFloat(-1.0f, 1.0f),
			TAGUtil::randomRangeFloat(-1.0f, 1.0f),
			TAGUtil::randomRangeFloat(-1.0f, 1.0f)
		};
		dir = glm::normalize((glm::dot(dir, dir) == 0.0f ? glm::vec3(1.0f) : dir));

		TAGModel::Object obj = pointDirection(dir);
		obj.position = spawn_pos;
		obj.scale = 1.5f;

		boid_model.setInstance(obj, -1, BOID_MESH_NAME);
		boids_data.emplace_back(0, start_speed, dir);
	}
	boid_model.updateInstanceBuffer(BOID_MESH_NAME);

	generateOctree();
}

FlockController::OctreeNode::OctreeNode(size_t parent_index, size_t depth, const TAGMesh::BoundingBox& bounds) :
	parent_index(parent_index), depth(depth), bounds(bounds)
{
	const glm::vec3 c = (bounds.min + bounds.max) / 2.0f;
	const glm::vec3 disp_to_max = bounds.max - c;

	loose_bounds.max = c + disp_to_max * OCTREE_LOOSE_BOUNDS_FAC;
	loose_bounds.min = c - disp_to_max * OCTREE_LOOSE_BOUNDS_FAC;

	boid_indices.reserve(OCTREE_MAX_BOIDS);
}


void FlockController::updateFlock(float dt) {
	const TAGMesh::BoundingBox margins = { .max = game_bounds.max * BORDER_BOUNDS_FAC, .min = game_bounds.min * BORDER_BOUNDS_FAC };
	const auto& boids = boid_model.getAllInstances(BOID_MESH_NAME);
	std::vector<TAGModel::Object> new_boids(boids.size());
	std::vector<BoidData> new_boids_data(boids.size());
	std::vector<size_t> movers;

	for (size_t i = 0; i < boids.size(); i++) {
		const auto& boid = boids[i];
		const auto& boid_data = boids_data[i];

		glm::vec3 move_away{}, vel_avg{}, pos_avg{};
		int visible_boids = 0;

		for (size_t j : octree[boid_data.leaf].boid_indices) {
			if (i == j) continue;

			const auto& other_boid = boids[j];
			const auto& other_boid_movement = boids_data[j];
			const glm::vec3 dist = boid.position - other_boid.position;
			const float length_sqr = glm::dot(dist, dist);

			if (length_sqr < visible_range) {
				vel_avg += other_boid_movement.direction * other_boid_movement.speed;
				pos_avg += other_boid.position;
				visible_boids++;

				if (length_sqr < protected_range) move_away += dist;
			};
		}

		BoidData new_boid_data = boid_data;
		glm::vec3 boid_vel = boid_data.direction * boid_data.speed;
		if (visible_boids > 0) {
			boid_vel += (
				((vel_avg / (float)visible_boids) - boid_vel) * alignment_factor + 
				((pos_avg / (float)visible_boids) - boid.position) * cohesion_factor
			) * dt;
		}
		if (move_away != glm::vec3(0.0f)) {
			boid_vel += move_away * separation_factor * dt;
		}
		boid_vel += (glm::clamp(boid.position, margins.min, margins.max) - boid.position) * border_factor * dt;

		const float length = glm::length(boid_vel);
		new_boid_data.speed = glm::clamp(length, min_speed, max_speed);
		new_boid_data.direction = (length == 0.0f ? boid_data.direction : boid_vel / length);

		TAGModel::Object new_boid = pointDirection(new_boid_data.direction);
		new_boid.scale = boid.scale;
		new_boid.position = boid.position + new_boid_data.direction * new_boid_data.speed * dt;

		if (!octree[boid_data.leaf].loose_bounds.collisionPoint(new_boid.position)) movers.push_back(i);

		new_boids[i] = new_boid;
		new_boids_data[i] = new_boid_data;
	}

	boid_model.setAllInstances(new_boids, BOID_MESH_NAME);
	boid_model.updateInstanceBuffer(BOID_MESH_NAME);
	boids_data = new_boids_data;
	const auto& current_boids = boid_model.getAllInstances(BOID_MESH_NAME);

	// Deal with boids moving to other octree cells
	for (size_t boid_index : movers) {
		BoidData& boid_data = boids_data[boid_index];

		// Remove boid from old leaf
		const size_t old_leaf_index = boid_data.leaf;
		OctreeNode& old_leaf = octree[boid_data.leaf];
		old_leaf.boid_indices.erase(std::find(old_leaf.boid_indices.begin(), old_leaf.boid_indices.end(), boid_index));

		// Find new home for boid
		const size_t new_leaf_index = getBoidLeaf(current_boids[boid_index].position, 0);
		octree[new_leaf_index].boid_indices.push_back(boid_index);
		boid_data.leaf = new_leaf_index;

		// Merging
		if (old_leaf_index > 0) {
			OctreeNode& parent_node = octree[old_leaf.parent_index];
			bool children_all_leaf = true;
			for (size_t i = 0; i < 8; i++) {
				OctreeNode& child_node = octree[parent_node.first_child_index + i];

				if (!child_node.isLeaf()) {
					children_all_leaf = false;
					break;
				}

				parent_node.boid_indices.insert(parent_node.boid_indices.end(), child_node.boid_indices.begin(), child_node.boid_indices.end());
			}

			if (children_all_leaf && parent_node.boid_indices.size() <= OCTREE_MAX_BOIDS / 2) {
				free_intervals.push_back(parent_node.first_child_index);
				parent_node.first_child_index = 0;

				for (size_t boid_index : parent_node.boid_indices) boids_data[boid_index].leaf = old_leaf.parent_index;
			}
			else {
				parent_node.boid_indices.clear();
			}
		}

		// Splitting
		std::vector<size_t> split_cascade_stack{ boid_data.leaf };
		while (!split_cascade_stack.empty()) {
			const size_t node_index = split_cascade_stack.back();
			split_cascade_stack.pop_back();

			if (octree[node_index].boid_indices.size() > OCTREE_MAX_BOIDS && octree[node_index].depth <= OCTREE_MAX_DEPTH) {
				auto child_bounds = splitNode(octree[node_index].bounds);
				if (!free_intervals.empty()) {
					octree[node_index].first_child_index = free_intervals.back();
					free_intervals.pop_back();
					for (size_t i = 0; i < 8; i++) octree[octree[node_index].first_child_index + i] = OctreeNode(node_index, octree[node_index].depth + 1, child_bounds[i]);
				}
				else {
					octree[node_index].first_child_index = octree.size();
					for (size_t i = 0; i < 8; i++) octree.emplace_back(node_index, octree[node_index].depth + 1, child_bounds[i]);
				}

				for (size_t boid_index : octree[node_index].boid_indices) {
					const size_t leaf_index = getBoidLeaf(current_boids[boid_index].position, node_index);
					octree[leaf_index].boid_indices.push_back(boid_index);

					if (octree[leaf_index].boid_indices.size() == OCTREE_MAX_BOIDS) split_cascade_stack.push_back(leaf_index);

					boids_data[boid_index].leaf = leaf_index;
				}
				octree[node_index].boid_indices.clear();
			}
		}
	}
}

void FlockController::drawAll(const TAGShaderManager::Shader& shader) {
	boid_model.drawAll(shader, BOID_MESH_NAME, { .cull_backface = false });
}

void FlockController::generateOctree() {
	octree.clear();
	octree.emplace_back(0, 0, game_bounds);

	std::vector<size_t> octree_build_stack;

	const auto& boids = boid_model.getAllInstances(BOID_MESH_NAME);

	std::unordered_map<size_t, std::vector<size_t>> parent_boids;
	parent_boids[0] = std::vector<size_t>(boids.size());
	for (size_t i = 0; i < boids.size(); i++) parent_boids[0][i] = i;

	if (boids.size() > OCTREE_MAX_BOIDS) {
		octree[0].first_child_index = 1;
		auto child_bounds = splitNode(game_bounds);
		for (size_t i = 0; i < 8; i++) {
			octree_build_stack.push_back(i + 1);
			octree.emplace_back(0, 1, child_bounds[i]);
		}
	} 
	else {
		octree[0].boid_indices = parent_boids[0];
	}

	while (!octree_build_stack.empty()) {
		const size_t node_index = octree_build_stack.back();
		OctreeNode& node = octree[node_index];
		octree_build_stack.pop_back();

		std::vector<size_t> leftover_boids;
		for (size_t boid_index : parent_boids[node.parent_index]) {
			const glm::vec3& pos = boids[boid_index].position;

			if (node.bounds.collisionPoint(pos)) {
				node.boid_indices.push_back(boid_index);
			}
			else {
				leftover_boids.push_back(boid_index);
			}
		}
		parent_boids[node.parent_index] = leftover_boids;

		if (node.boid_indices.size() > OCTREE_MAX_BOIDS && node.depth <= OCTREE_MAX_DEPTH) {
			parent_boids[node_index] = node.boid_indices;
			node.boid_indices.clear();
			node.first_child_index = octree.size();
			auto child_bounds = splitNode(node.bounds);
			for (size_t i = 0; i < 8; i++) {
				octree_build_stack.push_back(octree.size());
				octree.emplace_back(node_index, node.depth + 1, child_bounds[i]);
			}
		} 
		else {
			for (size_t boid_index : node.boid_indices) boids_data[boid_index].leaf = node_index;
		}
	}
}

size_t FlockController::getBoidLeaf(glm::vec3 pos, size_t start_node) const {
	pos = glm::clamp(pos, game_bounds.min, game_bounds.max);

	size_t idx = start_node;
	while (!octree[idx].isLeaf()) {
		const TAGMesh::BoundingBox& b = octree[idx].bounds;
		const glm::vec3 c = (b.min + b.max) * 0.5f;
		idx = octree[idx].first_child_index
			+ (pos.x >= c.x ? 1 : 0)
			+ (pos.y >= c.y ? 2 : 0)
			+ (pos.z >= c.z ? 4 : 0);
	}
	return idx;
}

TAGModel::Object FlockController::pointDirection(const glm::vec3& new_direction) {
	const glm::vec3 cross = glm::cross(START_DIR, new_direction);
	const float cross_length = glm::length(cross);
	return {
		.rotation_axis = (cross_length > 0.0f ? cross / cross_length : START_DIR),
		.angle = std::atan2(cross_length, glm::dot(START_DIR, new_direction))
	};
}

std::array<TAGMesh::BoundingBox, 8> FlockController::splitNode(const TAGMesh::BoundingBox& node) {
	const glm::vec3 c = (node.max + node.min) / 2.0f;
	std::array<TAGMesh::BoundingBox, 8> child_nodes{};
	for (size_t i = 0; i < 8; i++) {
		glm::vec3 min{}, max{};

		min.x = (i & 1) ? c.x : node.min.x;
		max.x = (i & 1) ? node.max.x : c.x;

		min.y = (i & 2) ? c.y : node.min.y;
		max.y = (i & 2) ? node.max.y : c.y;

		min.z = (i & 4) ? c.z : node.min.z;
		max.z = (i & 4) ? node.max.z : c.z;

		child_nodes[i] = { .max = max, .min = min };
	}
	return child_nodes;
}
