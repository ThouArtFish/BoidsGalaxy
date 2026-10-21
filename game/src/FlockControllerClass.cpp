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
	boid_data.reserve(boid_count);
	for (size_t i = 0; i < boid_count; i++) {
		const glm::vec3 spawn_pos = {
			TAGUtil::randomRangeFloat(game_bounds.min.x, game_bounds.max.x),
			TAGUtil::randomRangeFloat(game_bounds.min.y, game_bounds.max.y),
			TAGUtil::randomRangeFloat(game_bounds.min.z, game_bounds.max.z)
		};

		glm::vec3 dir = {
			TAGUtil::randomFloat<float>() * 2.0f - 1.0f,
			TAGUtil::randomFloat<float>() * 2.0f - 1.0f,
			TAGUtil::randomFloat<float>() * 2.0f - 1.0f
		};
		dir = glm::normalize((glm::dot(dir, dir) == 0.0f ? glm::vec3(1.0f) : dir));

		TAGModel::Object obj = pointDirection(START_DIR, dir);
		obj.position = spawn_pos;
		obj.scale = 1.5f;

		boid_model.setInstance(obj, -1, BOID_MESH_NAME);
		boid_data.emplace_back(start_speed, dir);
	}
	boid_model.updateInstanceBuffer(BOID_MESH_NAME);
}

void FlockController::updateFlock(float dt) {
	TAGMesh::BoundingBox margins = { .max = game_bounds.max * 0.87f, .min = game_bounds.min * 0.87f };
	const auto& boids = boid_model.getAllInstances(BOID_MESH_NAME);
	std::vector<TAGModel::Object> new_boids(boids.size());
	std::vector<BoidMovement> new_boids_movements(boids.size());

	generateOctree();

	for (size_t i = 0; i < boids.size(); i++) {
		const auto& boid = boids[i];
		const auto& boid_movement = boid_data[i];

		std::vector<size_t> leaves, index_stack;
		index_stack.push_back(0);
		while (!index_stack.empty()) {
			const size_t index = index_stack.back();
			const OctreeNode& node = octree[index];
			index_stack.pop_back();

			const glm::vec3 bounded_disp = boid.position - glm::clamp(boid.position, node.bounds.min, node.bounds.max);
			const float bounded_dist_sqr = glm::dot(bounded_disp, bounded_disp);

			if (bounded_dist_sqr < visible_range) {
				if (node.is_leaf) {
					leaves.push_back(index);
				}
				else {
					index_stack.insert(index_stack.end(), node.indices.begin(), node.indices.end());
				}
			}
		}

		glm::vec3 move_away{}, vel_avg{}, pos_avg{};
		int visible_boids = 0;

		for (size_t leaf_index : leaves) {
			for (size_t j : octree[leaf_index].indices) {
				if (i == j) continue;

				const auto& other_boid = boids[j];
				const auto& other_boid_movement = boid_data[j];
				const glm::vec3 dist = boid.position - other_boid.position;
				const float length_sqr = glm::dot(dist, dist);

				if (length_sqr < visible_range) {
					vel_avg += other_boid_movement.direction * other_boid_movement.speed;
					pos_avg += other_boid.position;
					visible_boids++;

					if (length_sqr < protected_range) move_away += dist;
				};
			}
		}

		BoidMovement new_boid_movement = boid_movement;
		glm::vec3 boid_vel = boid_movement.direction * boid_movement.speed;
		if (visible_boids > 0) {
			boid_vel += (
				((vel_avg / (float)visible_boids) - boid_vel) * alignment_factor + 
				((pos_avg / (float)visible_boids) - boid.position) * cohesion_factor
			) * dt;
		}
		if (move_away != glm::vec3(0.0f)) {
			boid_vel += move_away * separation_factor * dt;
		}
		boid_vel += (glm::clamp(boid.position, margins.min, margins.max) - boid.position) * turn_factor * dt;

		const float length = glm::length(boid_vel);
		new_boid_movement.speed = glm::clamp(length, min_speed, max_speed);
		new_boid_movement.direction = (length == 0.0f ? boid_movement.direction : boid_vel / length);

		TAGModel::Object new_boid = pointDirection(START_DIR, new_boid_movement.direction);
		new_boid.scale = boid.scale;
		new_boid.position = boid.position + new_boid_movement.direction * new_boid_movement.speed * dt;

		new_boids[i] = new_boid;
		new_boids_movements[i] = new_boid_movement;
	}

	boid_model.setAllInstances(new_boids, BOID_MESH_NAME);
	boid_model.updateInstanceBuffer(BOID_MESH_NAME);
	boid_data = new_boids_movements;
}

void FlockController::drawAll(const TAGShaderManager::Shader& shader) {
	boid_model.drawAll(shader, BOID_MESH_NAME, { .cull_backface = false });
}

void FlockController::generateOctree() {
	octree.clear();
	
	const auto& boids = boid_model.getAllInstances(BOID_MESH_NAME);

	std::vector<size_t> indices(boids.size());
	for (size_t i = 0; i < boids.size(); i++) indices[i] = i;

	size_t id = 0;

	if (indices.size() > OCTREE_MAX_BOIDS) {
		auto child_nodes = splitNode(game_bounds);

		std::vector<size_t> child_indices(8);
		for (size_t i = 0; i < 8; i++) {
			child_indices[i] = ++id;
			recursiveOctreeStep(boids, child_nodes[i], indices, id);
		}

		octree.emplace_back(0, game_bounds, child_indices, false);
	}
	else {
		octree.emplace_back(0, game_bounds, indices, true);
	}

	std::sort(octree.begin(), octree.end(), [](const OctreeNode& a, const OctreeNode& b) { return a.id < b.id; });
}

void FlockController::recursiveOctreeStep(const std::vector<TAGModel::Object>& boids, TAGMesh::BoundingBox& bounds, std::vector<size_t>& indices, size_t& id) {
	const size_t next_id = id;
	std::vector<size_t> inside_indices, outside_indices;

	glm::vec3 min = (bounds.max + bounds.min) / 2.0f, max = (bounds.max + bounds.min) / 2.0f;
	for (size_t index : indices) {
		const glm::vec3& pos = boids[index].position;
		if (bounds.collisionPoint(pos)) {
			inside_indices.push_back(index);
			min = glm::min(pos, min);
			max = glm::max(pos, max);
		}
		else {
			outside_indices.push_back(index);
		}
	}
	bounds.min = min;
	bounds.max = max;
	indices = std::move(outside_indices);
	outside_indices.clear();
	outside_indices.shrink_to_fit();

	if (inside_indices.size() > OCTREE_MAX_BOIDS) {
		auto child_nodes = splitNode(bounds);

		std::vector<size_t> child_indices(8);
		for (size_t i = 0; i < 8; i++) {
			child_indices[i] = ++id;
			recursiveOctreeStep(boids, child_nodes[i], inside_indices, id);
		}

		octree.emplace_back(next_id, bounds, child_indices, false);
	}
	else {
		octree.emplace_back(next_id, bounds, inside_indices, true);
	}
}

TAGModel::Object FlockController::pointDirection(const glm::vec3& old_direction, const glm::vec3& new_direction) {
	const glm::vec3 cross = glm::cross(old_direction, new_direction);
	const float cross_length = glm::length(cross);
	return {
		.rotation_axis = (cross_length > 0.0f ? cross / cross_length : START_DIR),
		.angle = std::atan2(cross_length, glm::dot(old_direction, new_direction))
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
