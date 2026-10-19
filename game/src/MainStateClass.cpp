#include "../include/MainStateClass.hpp"

MainState::MainState() {
	static const std::vector<TAGMesh::Vertex> vertices = {
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2() },
		{ glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() },
		{ glm::vec3(0.0f, -1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2() }
	};
	static const std::vector<TAGMesh::Fragment> fragments = { { 0, 1, 2 }, { 3, 4, 5 } };
	boids.addMesh(boid_mesh, vertices, fragments, { .colour = glm::vec3(1.0f, 0.0f, 0.0f) });
	boids.setInstance({ .position = glm::vec3(0.0f, 0.0f, 0.0f), .scale = 5.0f }, -1, boid_mesh);

	sunlight.setLight({ .direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)), .colour = { 1.0f, 1.0f, 1.0f } });
	sunlight.bindToShader();

	shaders.useShader(main_shader).set(TAGShaderManager::default_options.ambience, AMBIENCE);
	shaders.setAll<glm::mat4>(shader_names,
		{ TAGShaderManager::default_options.perspective_mat },
		{ createPerspectiveMatrix() }
	);
}

void MainState::mouseCallback() {
	camera_direction = player_controller.cameraDirection();
}

void MainState::framebufferSizeCallback() {
	shaders.setAll<glm::mat4>(shader_names,
		{ TAGShaderManager::default_options.perspective_mat },
		{ createPerspectiveMatrix() }
	);
}

std::string MainState::mainLoop() {
	if (getKeyState(GLFW_KEY_ESCAPE)) return "END";

	if (getKeyState(GLFW_KEY_P) == 1) {
		setWindowFullscreen(TAGEnum::TOGGLE);
		setMouseLock(TAGEnum::TOGGLE);
	}

	camera_position += player_controller.playerDirection() * SPEED * (float)delta_time;

	const glm::mat4 cam_mat = createCameraMatrix();
	shaders.setAll<glm::mat4>(shader_names,
		{ TAGShaderManager::default_options.camera_mat },
		{ cam_mat, glm::mat4(glm::mat3(cam_mat)) }
	);

	boids.drawAll(shaders.useShader(main_shader), boid_mesh, {.cull_backface = false});
	skybox.draw(shaders.useShader(skybox_shader));

	return "CURRENT";
}
