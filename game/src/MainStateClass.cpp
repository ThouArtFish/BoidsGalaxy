#include "../include/MainStateClass.hpp"

MainState::MainState() {
	TAGBaseState::far = 1000.0f;
	frame_interval = 0.0f;

	imgui_handler.createContext(TAGBaseState::window);

	shaders.addShader<ProgramType::Default>({
		{ main_shader, "", TAGShaderManager::DefaultProgramType::INSTANCED_MODEL_DRAW },
		{ skybox_shader, "", TAGShaderManager::DefaultProgramType::SKYBOX_DRAW }
	});

	planet_manager.generatePlanets();

	sunlight.setLight({ .direction = { 1.0f, 0.0f, 0.0f }, .colour = { 1.0f, 1.0f, 1.0f } });
	sunlight.updateLightBuffer();

	shaders.useShader(main_shader).set(TAGShaderManager::default_options.ambience, AMBIENCE);
	setProjectiveMatrix();
}

void MainState::mouseCallback() {
	if (mouse_locked) camera_direction = player_controller.cameraDirection();
}

void MainState::framebufferSizeCallback() {
	setProjectiveMatrix();
}

std::string MainState::mainLoop() {
	imgui_handler.startFrame(flock_controller.get());

	if (getKeyState(GLFW_KEY_ESCAPE)) {
		imgui_handler.destroyContext();
		return "END";
	} 

	if (getKeyState(GLFW_KEY_G) == 1) {
		setMouseLock(TAGEnum::TOGGLE);
		mouse_locked = !mouse_locked;
	}

	if (getKeyState(GLFW_KEY_F) == 1) setWindowFullscreen(TAGEnum::TOGGLE);

	camera_position += player_controller.playerDirection(camera_direction, glm::vec3(0.0f)) * SPEED * (float)delta_time;

	setCameraMatrix();

	flock_controller->updateFlock((float)delta_time);

	sunlight.bindToShader();
	const auto& shader = shaders.useShader(main_shader);
	flock_controller->drawAll(shader);
	planet_manager.drawAll(shader);
	skybox.draw(shaders.useShader(skybox_shader));
	imgui_handler.draw();

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
