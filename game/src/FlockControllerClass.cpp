#include "../include/FlockControllerClass.hpp"

FlockController::FlockController(size_t boid_count, float boid_min_speed, float boid_max_speed) : min_speed(boid_min_speed), max_speed(boid_max_speed)
{
	static const std::vector<TAGMesh::Vertex> vertices = {
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() }
	};
	static const std::vector<TAGMesh::Fragment> fragments = { { 0, 1, 2 }, { 3, 4, 5 } };
	const float start_speed = (boid_max_speed - boid_min_speed) / 2.0f;

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
	
	for (size_t i = 0; i < boids.size(); i++) {
		const auto& boid = boids[i];
		const auto& boid_movement = boid_data[i];

		glm::vec3 move_away{}, vel_avg{}, pos_avg{};
		int visible_boids = 0;

		for (size_t j = 0; j < boids.size(); j++) {
			if (i == j) continue;

			const auto& other_boid = boids[j];
			const auto& other_boid_movement = boid_data[j];
			const glm::vec3 dist = boid.position - other_boid.position;
			const float length_sqr = glm::dot(dist, dist);

			if (length_sqr < protected_range) move_away += dist;
			if (length_sqr < visible_range) {
				vel_avg += other_boid_movement.direction * other_boid_movement.speed;
				pos_avg += other_boid.position;
				visible_boids++;
			};
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

TAGModel::Object FlockController::pointDirection(const glm::vec3& old_direction, const glm::vec3& new_direction) {
	const glm::vec3 cross = glm::cross(old_direction, new_direction);
	const float cross_length = glm::length(cross);
	return {
		.rotation_axis = (cross_length > 0.0f ? cross / cross_length : START_DIR),
		.angle = std::atan2(cross_length, glm::dot(old_direction, new_direction))
	};
}
