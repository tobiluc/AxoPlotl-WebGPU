#include "Application.h"
#include <cassert>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__
#ifdef WEBGPU_BACKEND_WGPU
#  include <webgpu/wgpu.h>
#endif // WEBGPU_BACKEND_WGPU

namespace AxoPlotl
{

Application::Application()
{}

Application::~Application()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Application::init()
{
    //------------------
    // Window
    //------------------
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(640, 480, "Learn WebGPU", nullptr, nullptr);
    if (!window) {return false;}

    return renderer.init(window);
}

void Application::mainLoop() {
    glfwPollEvents();

    renderer.render();
}

bool Application::isRunning() {
    return !glfwWindowShouldClose(window);
}

}
