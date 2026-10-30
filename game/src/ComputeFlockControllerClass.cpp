#include "../include/ComputeFlockControllerClass.hpp"

ComputeFlockController::ComputeFlockController(size_t boid_count) :
	boids_instance(boid_count, 3, TAGResourceManager::BufferAccess::STREAM),
	boids_data(boid_count, 3, TAGResourceManager::BufferAccess::STREAM),
	boids_map(boid_count, 5, TAGResourceManager::BufferAccess::STREAM),
	boids_constants(1, 1, TAGResourceManager::BufferAccess::STREAM)
{
	// Compile shaders
	shaders.addShader<ProgramType::Compute>(ProgramType::Compute{ COMPUTE_BOID_UPDATE_NAME, "BoidUpdateCompute.glsl" });

	// Calculate game bounds
	const float visible_range = glm::sqrt(BoidConstants{}.visible_range_sqr);
	for (size_t i = 0; i < 3; i++) {
		game_bounds.max[i] = (float)CELLS_PER_AXIS[i] * visible_range;
		game_bounds.min[i] = -game_bounds.max[i];
	}

	// Instantiate constants
	constants = { .total_boids = (GLuint)boid_count, .max_margin = game_bounds.max * GAME_BOUNDS_MARGIN, .dt = 1.0f / 60.0f };

	// Instantiate boid data
	const float start_speed = (constants.min_speed + constants.max_speed) / 2.0f;
	std::vector<TAGModel::ShaderObject> boid_inst(boid_count);
	std::vector<BoidObject> boid_objs(boid_count);
	std::vector<BoidKeyValue> map_objs(boid_count);
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
		map_objs[i] = { mortonCodeFromCell(glm::floor((spawn_pos - game_bounds.min) / glm::vec3(CELLS_PER_AXIS))), (GLuint)i };
	}
	boids_instance.updateBuffer(boid_inst);
	boids_data.updateBuffer(boid_objs);
	boids_map.updateBuffer(map_objs);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ComputeFlockController::updateFlock(float dt) {
	// Update constants and wait for completion
	constants.dt = dt;
	boids_constants.updateBuffer({ constants });
	glMemoryBarrier(GL_UNIFORM_BARRIER_BIT);

	// Bind buffers to compute shader and increment regions
	const GLintptr instance_write_fence = (boids_instance.getCurrentFence() + 1 == boids_instance.max_fences ? 0 : boids_instance.getCurrentFence() + 1);
	const GLintptr data_write_fence = (boids_data.getCurrentFence() + 1 == boids_data.max_fences ? 0 : boids_data.getCurrentFence() + 1);

	boids_instance.bindToShader(0, TAGResourceManager::ShaderBufferType::SHADER_STORAGE, boids_instance.getInternalOffset());
	boids_instance.bindToShader(1, TAGResourceManager::ShaderBufferType::SHADER_STORAGE, instance_write_fence * sizeof(TAGModel::ShaderObject) * boids_instance.getMaxObjects());

	boids_data.bindToShader(2, TAGResourceManager::ShaderBufferType::SHADER_STORAGE, boids_data.getInternalOffset());
	boids_data.bindToShader(3, TAGResourceManager::ShaderBufferType::SHADER_STORAGE, data_write_fence * sizeof(BoidObject) * boids_data.getMaxObjects());

	boids_constants.bindToShader(4, TAGResourceManager::ShaderBufferType::UNIFORM, 0);

	boids_instance.incrementFence();
	boids_data.incrementFence();

	// Dispatch compute
	shaders.useShader(COMPUTE_BOID_UPDATE_NAME);
	glDispatchCompute(glm::ceil((GLfloat)constants.total_boids / 64.0f), 1, 1);

	// First since buffer is attached to VAO for instance data, second since ranges will be written to on later frames
	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void ComputeFlockController::drawAll(const TAGShaderManager::Shader& shader) {
	/*
	boids_instance.bindToVertexArrayObject(TAGMesh::default_vao_instance_binding_point, TAGMesh::getVertexArrayObjectID(), boids_instance.getInternalOffset());
	boid_mesh.draw(shader, {}, boids_instance.getMaxObjects());
	*/
}

GLuint ComputeFlockController::mortonCodeFromCell(glm::uvec3 cell) {
	GLuint code = 0;
	for (size_t i = 0; i < 3; i++) {
		GLuint& v = cell[i];
		v &= 0x3FF;
		v = (v | (v << 16)) & 0x030000FF;
		v = (v | (v << 8)) & 0x0300F00F;
		v = (v | (v << 4)) & 0x030C30C3;
		v = (v | (v << 2)) & 0x09249249;
		code |= (i == 0 ? v : v << i);
	}
	return code;
}

