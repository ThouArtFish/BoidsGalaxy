#pragma once

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/imgui_impl_glfw.h"
#include "../vendor/imgui/imgui_impl_opengl3.h"
#include "FlockControllerBase.hpp"

class ImGuiHandler {
public:
	static void createContext(GLFWwindow* window);
	static void destroyContext();
	void startFrame(FlockControllerBase* base);
	void draw();
private:
	inline static bool context_created = false;
};