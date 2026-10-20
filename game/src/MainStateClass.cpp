#include "../include/MainStateClass.hpp"

MainState::MainState() {
	TAGBaseState::far = 300.0f;

	sunlight.setLight({ .direction = glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f)), .colour = { 1.0f, 1.0f, 1.0f } });
	sunlight.bindToShader();

	shaders.useShader(main_shader).set(TAGShaderManager::default_options.ambience, AMBIENCE);
	setProjectiveMatrix();
}

void MainState::mouseCallback() {
	camera_direction = player_controller.cameraDirection();
}

void MainState::framebufferSizeCallback() {
	setProjectiveMatrix();
}

std::string MainState::mainLoop() {
	if (getKeyState(GLFW_KEY_ESCAPE)) return "END";

	if (getKeyState(GLFW_KEY_P) == 1) {
		setWindowFullscreen(TAGEnum::TOGGLE);
		setMouseLock(TAGEnum::TOGGLE);
	}

	camera_position += player_controller.playerDirection(camera_direction, glm::vec3(0.0f)) * SPEED * (float)delta_time;

	setCameraMatrix();

	flock_controller.updateFlock((float)delta_time);

	flock_controller.drawAll(shaders.useShader(main_shader));
	skybox.draw(shaders.useShader(skybox_shader));

	return "CURRENT";
}

void MainState::setCameraMatrix() const {
	const glm::mat4 cam_mat = createCameraMatrix();
	shaders.setAll<glm::mat4>(shader_names,
		{ TAGShaderManager::default_options.camera_mat },
		{ cam_mat, glm::mat4(glm::mat3(cam_mat)) }
	);
}

void MainState::setProjectiveMatrix() const {
	shaders.setAll<glm::mat4>(shader_names,
		{ TAGShaderManager::default_options.perspective_mat },
		{ createPerspectiveMatrix() }
	);
}
