#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <array>
#include <cassert>
#include <sstream>
#include <numeric>

//#define APP_BASE
//#define APP_TRIANGLE
#define APP_TEXTURE

#include "VkAppBase.h"
#include "VkAppTriangle.h"
#include "VkAppTexture.h"

const int WindowWidth = 640, WindowHeight = 480;
const char* AppTitle = "ClearScreen";

int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, 0);
	auto window = glfwCreateWindow(WindowWidth, WindowHeight, AppTitle, nullptr, nullptr);

	// Vulkan 初期化
#if defined(APP_BASE)
	VulkanAppBase theApp;
#elif defined(APP_TRIANGLE)
	VulkanAppTriangle theApp{};
#elif defined(APP_TEXTURE)
	VulkanAppTexture theApp{};
#else
	VulkanAppBase theApp;
#endif
	theApp.initialize(window, AppTitle);

	while (glfwWindowShouldClose(window) == GLFW_FALSE)
	{
		glfwPollEvents();
		theApp.render();
	}

	// Vulkan 終了
	theApp.terminate();
	glfwTerminate();
	return 0;
}