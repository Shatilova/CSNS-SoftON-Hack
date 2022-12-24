#define GLFW_EXPOSE_NATIVE_WG
#define GLFW_EXPOSE_NATIVE_WIN32
#define STB_IMAGE_IMPLEMENTATION

#include "render.h"
#include "utils.h"
#include "glfw_stuff.h"

#include "file_system.h"

// initializing glfw, and glew, and imgui
Rendering::Rendering() {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		utils::Terminate("GLFW initialization is failed...");

	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	this->window = glfwCreateWindow(512, 320, "   ", NULL, NULL);
	if (window == NULL)
		utils::Terminate("Creating a window is failed...");

	glfwSetWindowCenter(window);

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	HWND hWnd = glfwGetWin32Window(window);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
		utils::Terminate("Failed to initialize OpenGL loader...");

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();
	ImGui::GetStyle().AntiAliasedLines = false;
	ImGui::GetStyle().AntiAliasedFill = false;
	ImGui::GetStyle().WindowRounding = 0.f;
	ImGui::GetStyle().WindowPadding = ImVec2(0.f, 0.f);

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	const auto & font = internal_fs::GetFileByName("RobotoBlack.ttf");
	void* font_raw = reinterpret_cast<void*>(const_cast<unsigned char*>(font.raw.data()));
	ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_raw, font.header.size, 16.f);
}

void Rendering::begin() {
	glfwDragWindow(window);

	glfwPollEvents();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Rendering::end() {
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
}

void Rendering::shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();
}