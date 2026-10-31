#include "../include/ComputeFlockControllerClass.hpp"

ComputeFlockController::ComputeFlockController(size_t boid_count) :
	boids_instance(boid_count, TAGResourceManager::BufferAccess::STREAM),
	boids_data(boid_count, TAGResourceManager::BufferAccess::STREAM),
	boids_constants(1, 1, TAGResourceManager::BufferAccess::STREAM)
{
	// Compile shaders
	shaders.addShader<ProgramType::Compute>(ProgramType::Compute{ COMPUTE_BOID_UPDATE_NAME, "BoidUpdateCompute.glsl" });

	// Calculate game bounds
	/*
	const float visible_range = glm::sqrt(BoidConstants{}.visible_range_sqr);
	for (size_t i = 0; i < 3; i++) {
		game_bounds.max[i] = (float)CELLS_PER_AXIS[i] * visible_range;
		game_bounds.min[i] = -game_bounds.max[i];
	}
	*/

	// Instantiate constants
	constants = { .max_margin = game_bounds.max * GAME_BOUNDS_MARGIN, .total_boids = (GLuint)boid_count, .dt = 1.0f / 60.0f };

	// Instantiate boid data
	const float start_speed = (constants.min_speed + constants.max_speed) / 2.0f;
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
			BOID_SCALE,
			glm::vec3(axis_and_angle),
			axis_and_angle.w
		};
		boid_objs[i] = {
			dir,
			start_speed
		};
	}
	boids_instance.updateReadBuffer(boid_inst);
	boids_data.updateReadBuffer(boid_objs);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ComputeFlockController::updateFlock(float dt) {
	// Update constants and wait for completion
	constants.dt = dt;
	boids_constants.updateBuffer({ constants });
	glMemoryBarrier(GL_UNIFORM_BARRIER_BIT);

	// Bind buffers to compute shader
	boids_instance.bindReadWrite(0, 1);
	boids_data.bindReadWrite(2, 3);
	boids_constants.bindToShader(4, TAGResourceManager::ShaderBufferType::UNIFORM, 0);

	// Dispatch compute
	shaders.useShader(COMPUTE_BOID_UPDATE_NAME);
	glDispatchCompute(glm::ceil((GLfloat)constants.total_boids / 64.0f), 1, 1);
	shaders.stopShader();

	// Unbind buffer
	boids_instance.unbindReadWrite();
	boids_data.unbindReadWrite();
	boids_constants.unbindFromShader(4, TAGResourceManager::ShaderBufferType::UNIFORM);

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
	boid_mesh.draw(shader, {}, inst_buffer.getMaxObjects());
	inst_buffer.unbindFromVertexArrayObject(TAGMesh::default_vao_instance_binding_point, TAGMesh::getVertexArrayObjectID());
	glEnable(GL_CULL_FACE);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
