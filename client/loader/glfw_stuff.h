#pragma once

#include <Windows.h>
#include <stdio.h>

#include "glfw3.h"
#include "glfw3native.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// thanks for this code to
// https://github.com/glfw/glfw/issues/310#issuecomment-46750107
void glfwSetWindowCenter(GLFWwindow* window) {
	// Get window position and size
	int window_x, window_y;
	glfwGetWindowPos(window, &window_x, &window_y);

	int window_width, window_height;
	glfwGetWindowSize(window, &window_width, &window_height);

	// Halve the window size and use it to adjust the window position to the center of the window
	window_width *= 0.5;
	window_height *= 0.5;

	window_x += window_width;
	window_y += window_height;

	// Get the list of monitors
	int monitors_length;
	GLFWmonitor** monitors = glfwGetMonitors(&monitors_length);

	if (monitors == NULL) {
		// Got no monitors back
		return;
	}

	// Figure out which monitor the window is in
	GLFWmonitor* owner = NULL;
	int owner_x, owner_y, owner_width, owner_height;

	for (int i = 0; i < monitors_length; i++) {
		// Get the monitor position
		int monitor_x, monitor_y;
		glfwGetMonitorPos(monitors[i], &monitor_x, &monitor_y);

		// Get the monitor size from its video mode
		int monitor_width, monitor_height;
		GLFWvidmode* monitor_vidmode = (GLFWvidmode*)glfwGetVideoMode(monitors[i]);

		if (monitor_vidmode == NULL) {
			// Video mode is required for width and height, so skip this monitor
			continue;

		}
		else {
			monitor_width = monitor_vidmode->width;
			monitor_height = monitor_vidmode->height;
		}

		// Set the owner to this monitor if the center of the window is within its bounding box
		if ((window_x > monitor_x && window_x < (monitor_x + monitor_width)) && (window_y > monitor_y && window_y < (monitor_y + monitor_height))) {
			owner = monitors[i];

			owner_x = monitor_x;
			owner_y = monitor_y;

			owner_width = monitor_width;
			owner_height = monitor_height;
		}
	}

	if (owner != NULL) {
		// Set the window position to the center of the owner monitor
		glfwSetWindowPos(window, owner_x + (owner_width * 0.5) - window_width, owner_y + (owner_height * 0.5) - window_height);
	}
}

// glfw Dragging NODECORATION window pice of code 
// BEGIN
// thanks for this code to
// https://stackoverflow.com/a/34920067/15301691
int cp_x;
int cp_y;
int offset_cpx;
int offset_cpy;
int w_posx;
int w_posy;
int buttonEvent;

void cursor_position_callback(GLFWwindow * window, double x, double y) {
	if (buttonEvent == 1) {
		offset_cpx = x - cp_x;
		offset_cpy = y - cp_y;
	}
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		buttonEvent = 1;
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		cp_x = floor(x);
		cp_y = floor(y);
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		buttonEvent = 0;
		cp_x = 0;
		cp_y = 0;
	}
}

void glfwDragWindow(GLFWwindow * window) {
	if (buttonEvent == 1) {
		glfwGetWindowPos(window, &w_posx, &w_posy);
		glfwSetWindowPos(window, w_posx + offset_cpx, w_posy + offset_cpy);
		offset_cpx = 0;
		offset_cpy = 0;
		cp_x += offset_cpx;
		cp_y += offset_cpy;
	}
}
// END