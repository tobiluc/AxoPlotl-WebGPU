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
}

bool Application::init()
{
    //------------------
    // Window
    //------------------
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(640, 480, "AxoPlotl - WebGPU", nullptr, nullptr);
    if (!window) {return false;}

    // Handle Resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->renderer.onWindowResize(width, height);
        }
    });

    return renderer.init(window);
}

void Application::mainLoop() {
    glfwPollEvents();

    renderer.render();
}

void Application::terminate() {
    renderer.release();
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Application::isRunning() {
    return !glfwWindowShouldClose(window);
}

}
