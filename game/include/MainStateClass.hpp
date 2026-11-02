#pragma once

#include <BaseStateClass.hpp>
#include <LightManagerClass.hpp>
#include <ShaderManagerClass.hpp>
#include <ModelClass.hpp>
#include <SkyboxClass.hpp>
#include <UtilClass.hpp>
#include "TAMPlayerController.hpp"
#include "ClientFlockControllerClass.hpp"
#include "ComputeFlockControllerClass.hpp"
#include "PlanetManagerClass.hpp"
#include "ImGuiHandlerClass.hpp"

class MainState : public TAGBaseState {
public:
	MainState();
	void enter() override {};
	void exit() override {};
	void mouseCallback() override;
	void framebufferSizeCallback() override;
	void iconifyCallback() override {};
	std::string mainLoop() override;
private:
	static constexpr float AMBIENCE = 0.4f;
	static constexpr size_t BOID_COUNT = 6400;
	static constexpr float SPEED = 70.0f;
	static constexpr size_t PLANET_COUNT = 5;

	inline static const std::string main_shader = "main";
	inline static const std::string skybox_shader = "skybox";
	inline static const std::string boid_mesh = "boid";
	inline static const std::vector<std::string> shader_names = { main_shader, skybox_shader };

	TAM::PlayerController player_controller{};
	std::unique_ptr<FlockControllerBase> flock_controller = std::make_unique<ComputeFlockController>(BOID_COUNT);
	PlanetManager planet_manager{ PLANET_COUNT };
	TAGSkybox skybox{ "space_skybox.png" };
	TAGLightManager<TAGLight::Ray> sunlight{ TAGResourceManager::BufferAccess::STATIC, 1 };
	ImGuiHandler imgui_handler{};
	TAGShaderManager shaders{};
	bool mouse_locked = false;

	void setCameraMatrix() const;
	void setProjectiveMatrix() const;
};
