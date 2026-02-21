#define WEBGPU_CPP_IMPLEMENTATION

#include "AxoPlotl/Application.hpp"

int main()
{
    std::cout << std::filesystem::current_path()  << std::endl;

    // Initialize main Runner
    AxoPlotl::Application app;
    if (!app.init()) {
        std::cerr << "Failed to initialice AxoPlotl" << std::endl;
        return 1;
    }
#ifdef __EMSCRIPTEN__
    auto callback = [](void *arg) {
        Application* app_ptr = reinterpret_cast<Application*>(arg);
        app_ptr->run();
    };
    emscripten_set_main_loop_arg(callback, &app, 0, true);
#else // __EMSCRIPTEN__
    while (!glfwWindowShouldClose(app.window())) {
        @autoreleasepool {
            app.run();
        }
    }
#endif // __EMSCRIPTEN__
    app.terminate();

    return 0;
}
