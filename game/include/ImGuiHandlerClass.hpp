#pragma once

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_glfw.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"
#include "FlockControllerBase.hpp"

class ImGuiHandler {
public:
	static void createContext(GLFWwindow* window) {
		if (!context_created) {
			// Setup Dear ImGui context
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

			// Setup Platform/Renderer backends
			ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
			ImGui_ImplOpenGL3_Init();
			context_created = true;
		}
	}
	static void destroyContext() {
		if (context_created) {
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			context_created = false;
		}
	}
	void startFrame(FlockControllerBase* base) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Boids Simulation Constants");
		ImGui::SliderInt("Boid Count", (int*)&base->boid_count, base->MIN_BOIDS, base->MAX_BOIDS);
		ImGui::SliderFloat("Separation", &base->separation_factor, 0.0f, 1.0f);
		ImGui::SliderFloat("Alignment", &base->alignment_factor, 0.0f, 1.0f);
		ImGui::SliderFloat("Cohesion", &base->cohesion_factor, 0.0f, 0.5f);
		ImGui::SliderFloat("Visible Range", &base->visible_range, 0.0f, 200.0f);
		ImGui::SliderFloat("Collision Range", &base->protected_range, 0.0f, 200.0f);
		ImGui::InputFloat3("Minimum Space Bounds", &base->game_bounds.min[0]);
		ImGui::InputFloat3("Maximum Space Bounds", &base->game_bounds.max[0]);
		ImGui::SliderFloat("Minimum Speed", &base->min_speed, 0.0f, 100.0f);
		ImGui::SliderFloat("Maximum Speed", &base->max_speed, 0.0f, 100.0f);

		ImGui::End();
	}
	void draw() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
private:
	inline static bool context_created = false;
};
