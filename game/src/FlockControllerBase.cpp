#include "../include/ComputeFlockControllerClass.hpp"

FlockControllerBase::FlockControllerBase(size_t boid_count) : boid_count(glm::clamp(boid_count, MIN_BOIDS, MAX_BOIDS)) {}

void FlockControllerBase::addUpdateTime(double dt) {
	update_count += 1.0;
	update_total += dt;
	if (update_count >= UPDATE_FRAMES) {
		update_avg = update_total / update_count;
		update_total = 0.0f;
		update_count = 0.0f;
	}
}
