#include "../include/ComputeFlockControllerClass.hpp"

ComputeFlockController::ComputeFlockController(size_t boid_count) : FlockControllerBase(boid_count),
	boids_instance(boid_count, TAGResourceManager::BufferAccess::STREAM),
	boids_data(boid_count, TAGResourceManager::BufferAccess::STREAM),
	boids_constants(1, 1, TAGResourceManager::BufferAccess::STREAM)
{
	// Compile shaders
	shaders.addShader<ProgramType::Compute>(ProgramType::Compute{ COMPUTE_BOID_UPDATE_NAME, "BoidUpdateCompute.glsl" });

	// Instantiate boid data
	const float start_speed = (min_speed + max_speed) / 2.0f;
	std::vector<TAGModel::ShaderObject> boid_inst(boid_count);
	std::vector<BoidObject> boid_objs(boid_count);
	for (size_t i = 0; i < boid_count; i++) {
		// Generate instance data
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
		const glm::vec4 axis_and_angle = TAGUtil::pointDirection(dir, START_DIR);

		boid_inst[i] = {
			spawn_pos,
			boid_scale,
			glm::vec3(axis_and_angle),
			axis_and_angle.w
		};
		boid_objs[i] = {
			dir,
			start_speed
		};
	}
	boids_instance.getReadBuffer().updateBuffer(boid_inst);
	boids_data.getReadBuffer().updateBuffer(boid_objs);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

ComputeFlockController::BoidConstants::BoidConstants(ComputeFlockController* self, float dt) {
	separation_factor = self->separation_factor;
	alignment_factor = self->alignment_factor;
	cohesion_factor = self->cohesion_factor;
	protected_range_sqr = self->protected_range * self->protected_range;
	visible_range_sqr = self->visible_range * self->visible_range;
	border_factor = self->border_factor;
	min_speed = self->min_speed;
	max_speed = self->max_speed;
	max_margin = self->game_bounds.max * self->border_start_factor;
	total_boids = self->boid_count;
	this->dt = dt;
}

void ComputeFlockController::updateFlock(float dt) {
	// Get compute time from last frame
	const double poll_time = query_manager.poll();
	if (poll_time >= 0.0) addUpdateTime(poll_time / 1.0e9);

	// If boid count has changed, add or remove boids
	auto& boids_data_buffer = boids_data.getReadBuffer();
	auto& boids_instance_buffer = boids_instance.getReadBuffer();
	const size_t change_boids = (size_t)glm::abs((int)boid_count - (int)boids_data_buffer.getCurrentObjects());
	if (boid_count > boids_data_buffer.getCurrentObjects()) {
		std::vector<TAGModel::ShaderObject> boid_inst(change_boids);
		std::vector<BoidObject> boid_objs(change_boids);
		const float start_speed = (max_speed - min_speed) / 2.0f;
		for (size_t i = 0; i < change_boids; i++) {
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
			const glm::vec4 axis_and_angle = TAGUtil::pointDirection(dir, START_DIR);

			boid_inst[i] = {
				spawn_pos,
				boid_scale,
				glm::vec3(axis_and_angle),
				axis_and_angle.w
			};
			boid_objs[i] = {
				dir,
				start_speed
			};
		}
		if (change_boids + boids_data_buffer.getCurrentObjects() > boids_data_buffer.getMaxObjects()) {
			const size_t new_size = glm::min(MAX_BOIDS, (change_boids + boids_data_buffer.getCurrentObjects()) * 2);
			boids_data_buffer.resizeBuffer(new_size);
			boids_data.getWriteBuffer().resizeBuffer(new_size);
			boids_instance_buffer.resizeBuffer(new_size);
			boids_instance.getWriteBuffer().resizeBuffer(new_size);
		}
		boids_data_buffer.appendBuffer(boid_objs);
		boids_instance_buffer.appendBuffer(boid_inst);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
	else if (boid_count < boids_data_buffer.getCurrentObjects()) {
		const size_t start_index = boids_data_buffer.getCurrentObjects() - change_boids;
		boids_data_buffer.eraseBuffer(start_index);
		boids_instance_buffer.eraseBuffer(start_index);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	// Start timer
	query_manager.begin();

	// Update constants and wait for completion
	boids_constants.updateBuffer({ BoidConstants(this, dt) });
	glMemoryBarrier(GL_UNIFORM_BARRIER_BIT);

	// Bind buffers to compute shader
	boids_instance.bindReadWrite(0, 1);
	boids_data.bindReadWrite(2, 3);
	boids_constants.bindToShader(4, TAGResourceManager::ShaderBufferType::UNIFORM, 0);

	// Dispatch compute
	shaders.useShader(COMPUTE_BOID_UPDATE_NAME);
	glDispatchCompute(glm::ceil((GLfloat)boid_count / 64.0f), 1, 1);
	shaders.stopShader();

	// Unbind buffer
	boids_instance.unbindReadWrite();
	boids_data.unbindReadWrite();
	boids_constants.unbindFromShader(4, TAGResourceManager::ShaderBufferType::UNIFORM);

	// End time
	query_manager.end();

	// Swap read and write buffers
	boids_instance.switchBuffers();
	boids_data.switchBuffers();

	// First since buffer is attached to VAO for instance data, second since ranges will be written to on later frames
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void ComputeFlockController::drawAll(const TAGShaderManager::Shader& shader) {
	glDisable(GL_CULL_FACE);
	PingPongBuffer<TAGModel::ShaderObject>::Buffer& inst_buffer = boids_instance.getWriteBuffer();
	inst_buffer.bindToVertexArrayObject(TAGMesh::default_vao_instance_binding_point, TAGMesh::getVertexArrayObjectID(), 0);
	boid_mesh.draw(shader, {}, boid_count);
	inst_buffer.unbindFromVertexArrayObject(TAGMesh::default_vao_instance_binding_point, TAGMesh::getVertexArrayObjectID());
	glEnable(GL_CULL_FACE);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
