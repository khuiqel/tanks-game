#include "window-initializer.h"

#include "constants.h"
#include <algorithm> //std::fill
#include <iostream>

#include <stb_image.h>
#include "renderer.h" //only to set the projection matrix
#include <gtc/matrix_transform.hpp>

#include "frame-time-graph.h"
#include "keypress-manager.h"
#include "game-scene-manager.h" //to refresh the graphics when the window size changes
#include "game-manager.h" //settings (enable alpha)

GLFWwindow* WindowInitializer::glfw_window = nullptr;
bool WindowInitializer::currently_fullscreen = false;
int WindowInitializer::old_window_width;
int WindowInitializer::old_window_height;
int WindowInitializer::old_window_xpos;
int WindowInitializer::old_window_ypos;

int WindowInitializer::window_width;
int WindowInitializer::window_height;
int WindowInitializer::gamewindow_width;
int WindowInitializer::gamewindow_height;

void WindowInitializer::windowResizeFunc(GLFWwindow*, int w, int h) {
	//note: this function also gets called when moving the window between monitors (when the scale is different) (on Windows)
	WindowInitializer::window_width = w;
	WindowInitializer::window_height = h;

	// Define x-axis and y-axis range
	const float appXmin = 0.0;
	const float appXmax = GAME_WIDTH;
	const float appYmin = 0.0;
	const float appYmax = GAME_HEIGHT;

	// Define that OpenGL should use the whole window for rendering
	glViewport(0, 0, w, h);

	if ((appXmax - appXmin) / w < (appYmax - appYmin) / h) { //too wide
		float scale = ((appYmax - appYmin) / h) / ((appXmax - appXmin) / w);
		Renderer::proj = glm::ortho(float(GAME_WIDTH/2) - float(GAME_WIDTH/2)*scale, float(GAME_WIDTH/2) + float(GAME_WIDTH/2)*scale, 0.0f, (float)GAME_HEIGHT);
		WindowInitializer::gamewindow_width = WindowInitializer::window_height * (GAME_WIDTH/GAME_HEIGHT);
		WindowInitializer::gamewindow_height = WindowInitializer::window_height;
	} else { //too tall
		float scale = ((appXmax - appXmin) / w) / ((appYmax - appYmin) / h);
		Renderer::proj = glm::ortho(0.0f, (float)GAME_WIDTH, float(GAME_HEIGHT/2) - float(GAME_HEIGHT/2)*scale, float(GAME_HEIGHT/2) + float(GAME_HEIGHT/2)*scale);
		WindowInitializer::gamewindow_width = WindowInitializer::window_width;
		WindowInitializer::gamewindow_height = WindowInitializer::window_width * (GAME_HEIGHT/GAME_WIDTH);
	}

	//Windows does not do any updates when the window is being held or resized, so force a redraw:
	#ifdef _WIN32
	FrameTimeGraph::pushGraphTime("tick", 0); //HACK: since tick will never happen when the window is being resized, add it here; will not double-add when the game is over because that still does a tick
	GameSceneManager::DrawScenes_WindowResize();
	#endif
}

void WindowInitializer::windowCoordsToGameCoords(double inputX, double inputY, int& actualX, int& actualY) {
	//basically subtract out the black bars (just left/top), convert to a percent of the window, scale to game size
	//when the window is too wide, WindowInitializer::window_height = WindowInitializer::gamewindow_height; same when too tall for width
	//therefore this can handle both cases
	actualX =      (inputX - (WindowInitializer::window_width  - WindowInitializer::gamewindow_width)/2)  / WindowInitializer::gamewindow_width   * GAME_WIDTH;
	actualY = (1 - (inputY - (WindowInitializer::window_height - WindowInitializer::gamewindow_height)/2) / WindowInitializer::gamewindow_height) * GAME_HEIGHT;
}

//from: https://stackoverflow.com/questions/21421074/how-to-create-a-full-screen-window-on-the-current-monitor-with-glfw#31526753
GLFWmonitor* WindowInitializer::get_current_monitor(GLFWwindow* window) {
	int nmonitors, i;
	int wx, wy, ww, wh;
	int mx, my, mw, mh;
	int overlap, bestoverlap;
	GLFWmonitor* bestmonitor;
	GLFWmonitor** monitors;
	const GLFWvidmode* mode;

	bestoverlap = 0;
	bestmonitor = NULL;

	glfwGetWindowPos(window, &wx, &wy);
	glfwGetWindowSize(window, &ww, &wh);
	monitors = glfwGetMonitors(&nmonitors);

	for (i = 0; i < nmonitors; i++) {
		mode = glfwGetVideoMode(monitors[i]);
		glfwGetMonitorPos(monitors[i], &mx, &my);
		mw = mode->width;
		mh = mode->height;

		overlap =
			std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) *
			std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

		if (bestoverlap < overlap) {
			bestoverlap = overlap;
			bestmonitor = monitors[i];
		}
	}

	return bestmonitor;
}

void WindowInitializer::BeginningStuff() {
	if (KeypressManager::getKeyState("F11")) [[unlikely]] {
		if (currently_fullscreen) {
			//it's possible for the number of monitors to change, so these positions could be wrong, but whatever

			glfwSetWindowAttrib(WindowInitializer::glfw_window, GLFW_DECORATED, GLFW_TRUE);
			glfwSetWindowAttrib(WindowInitializer::glfw_window, GLFW_RESIZABLE, GLFW_TRUE); //this is *very needed* on Linux
			glfwSetWindowSize(WindowInitializer::glfw_window, WindowInitializer::old_window_width, WindowInitializer::old_window_height);
			glfwSetWindowPos(WindowInitializer::glfw_window, WindowInitializer::old_window_xpos, WindowInitializer::old_window_ypos); //pos should be set after size

			currently_fullscreen = false;
		} else {
			GLFWmonitor* monitor = get_current_monitor(WindowInitializer::glfw_window);
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			//doesn't need glfwGetMonitorContentScale to work properly

			int monitor_xpos, monitor_ypos;
			glfwGetMonitorPos(monitor, &monitor_xpos, &monitor_ypos);
			glfwGetWindowPos(WindowInitializer::glfw_window, &WindowInitializer::old_window_xpos, &WindowInitializer::old_window_ypos);
			glfwGetWindowSize(WindowInitializer::glfw_window, &WindowInitializer::old_window_width, &WindowInitializer::old_window_height);

			glfwSetWindowAttrib(WindowInitializer::glfw_window, GLFW_DECORATED, GLFW_FALSE); //this is not stated to be required for proper fullscreen
			glfwSetWindowAttrib(WindowInitializer::glfw_window, GLFW_RESIZABLE, GLFW_FALSE);
			glfwSetWindowPos(WindowInitializer::glfw_window, monitor_xpos, monitor_ypos);
			glfwSetWindowSize(WindowInitializer::glfw_window, mode->width, mode->height); //note: on Ubuntu, this will force the window to the largest monitor, unless auto-hide dock is enabled

			currently_fullscreen = true;
		}

		KeypressManager::unsetKeyState("F11");
	}
}

void WindowInitializer::WindowInitialize(int* argc, char** argv, std::string windowName) {
	WindowInitializer::WindowInitialize(argc, argv, windowName, 120, 120);
}

void WindowInitializer::WindowInitialize(int* argc, char** argv, std::string windowName, int startX, int startY) {
	WindowInitializer::WindowInitialize(argc, argv, windowName, startX, startY, 2.5);
}

void WindowInitializer::WindowInitialize(int* argc, char** argv, std::string windowName, int startX, int startY, double sizeMultiplier) {
	// Initialize GLFW
	if (!glfwInit()) {
		throw std::runtime_error("glfw failed");
	}

	// Setup window position, size, and title
	GLFWwindow* window;
	WindowInitializer::window_width = GAME_WIDTH*sizeMultiplier; WindowInitializer::window_height = GAME_HEIGHT*sizeMultiplier;
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
	// printf("%i, %i, %i\n", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
	#if !defined(_WIN32) && GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 4
	glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11); //GLFW 3.4 doesn't work on Wayland for some reason
	#endif

	window = glfwCreateWindow(WindowInitializer::window_width, WindowInitializer::window_height, windowName.c_str(), NULL, NULL);
	if (window == NULL) {
		const char* msg;
		glfwGetError(&msg);
		throw std::runtime_error("GLFW could not create the window: " + std::string(msg));
	}

	int primary_monitor_xpos, primary_monitor_ypos;
	glfwGetMonitorPos(glfwGetPrimaryMonitor(), &primary_monitor_xpos, &primary_monitor_ypos); //does not seem to do anything on Windows
	glfwSetWindowPos(window, primary_monitor_xpos + startX, primary_monitor_ypos + startY); //this sets the position of the inner window part, not the OS window position
	WindowInitializer::glfw_window = window;

	glfwGetWindowPos(WindowInitializer::glfw_window, &WindowInitializer::old_window_xpos, &WindowInitializer::old_window_ypos);
	glfwGetWindowSize(WindowInitializer::glfw_window, &WindowInitializer::old_window_width, &WindowInitializer::old_window_height); //required for monitor content scaling
	WindowInitializer::window_width = WindowInitializer::old_window_width; WindowInitializer::window_height = WindowInitializer::old_window_height;
	WindowInitializer::gamewindow_width = WindowInitializer::window_width; WindowInitializer::gamewindow_height = WindowInitializer::window_height;
	//glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xscale, &yscale);
	WindowInitializer::SetWindowIcon("res/favicon-64.png"); //if the OS doesn't like 64x64, boo hoo, no window icon then

	glfwMakeContextCurrent(window);

	//thanks to https://community.khronos.org/t/wglmakecurrent-issues/62656/3 for solving why a draw call would take ~15ms for no reason (it's just the V-sync time)
	//glfwSwapInterval(0);

	//initialize glad2
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0) {
		printf("Failed to initialize OpenGL context\n");
		throw std::runtime_error("glad failed");
	}
	// Successfully loaded OpenGL
	//printf("Loaded OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

	glDisable(GL_DEPTH_TEST); //technically not required, but not all drivers implement the OpenGL spec correctly
	//transparency:
	const GameSettings& game_settings = GameManager::get_settings();
	if (game_settings.Bullet_DrawAlpha) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

//from: https://stackoverflow.com/questions/44321902/load-icon-function-for-glfwsetwindowicon/64783512#64783512
void WindowInitializer::SetWindowIcon(const char* image_path) {
	GLFWimage images[1];
	auto ret = stbi_load(image_path, &images[0].width, &images[0].height, NULL, 4);
	if (ret == NULL) {
		//Windows doesn't care, but Linux does
		std::cerr << "Could not load \"" << image_path << "\": " << stbi_failure_reason() << std::endl;
	} else {
		images[0].pixels = ret;
		glfwSetWindowIcon(WindowInitializer::glfw_window, 1, images);
		stbi_image_free(images[0].pixels);
	}
}

void WindowInitializer::UninitializeWindow() {
	glfwTerminate();
}
