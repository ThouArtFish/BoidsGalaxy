#include "../include/ImGuiHandlerClass.hpp"

void ImGuiHandler::createContext(GLFWwindow* window) {
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

void ImGuiHandler::destroyContext() {
	if (context_created) {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		context_created = false;
	}
}

void ImGuiHandler::startFrame(FlockControllerBase* base) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Boids Simulation Constants");
	ImGui::Text("Average Update Time: %.3f ms", base->update_timer.update_avg * 1000.0f);
	ImGui::Text("Average Render Time: %.3f ms", base->render_timer.update_avg * 1000.0f);
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

void ImGuiHandler::draw() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
