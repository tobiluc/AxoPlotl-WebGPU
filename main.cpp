#define WEBGPU_CPP_IMPLEMENTATION

#include "AxoPlotl/Application.hpp"

int main()
{
    // Create Window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(640, 480, "AxoPlotl - WebGPU", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window" << std::endl;
        return 1;
    }

    // Initialize main Runner
    AxoPlotl::Application app;
    if (!app.init(window)) {
        std::cerr << "Failed to initialice AxoPlotl" << std::endl;
        return 1;
    }

    // Handle Resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwSetWindowUserPointer(window, &app);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        auto app_ptr = reinterpret_cast<AxoPlotl::Application*>(glfwGetWindowUserPointer(window));
        if (app_ptr) {
            app_ptr->on_window_resize(width, height);
        }
    });

#ifdef __EMSCRIPTEN__
    auto callback = [](void *arg) {
        Application* app_ptr = reinterpret_cast<Application*>(arg);
        app_ptr->run();
    };
    emscripten_set_main_loop_arg(callback, &app, 0, true);
#else // __EMSCRIPTEN__
    while (!glfwWindowShouldClose(window)) {
        app.run();
    }
#endif // __EMSCRIPTEN__

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
