// implementation of wrapper to hide work 
// with rendering systems (glfw and glew), and imgui

#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <Windows.h>
#include <stdio.h>
#include <cmath>

#include "glew.h"
#include "glfw3.h"
#include "glfw3native.h"

class Rendering
{
public:
	Rendering();

	GLFWwindow* get_window() { return this->window; }
	HWND get_HWND() { return this->hWnd; }

	void begin();
	void end();

	void shutdown();

private:
	GLFWwindow* window;
	HWND hWnd;
};

