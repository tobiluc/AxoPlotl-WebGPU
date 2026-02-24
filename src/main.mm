#include "AxoPlotl/gui/themes.hpp"
#define WEBGPU_CPP_IMPLEMENTATION

#include "AxoPlotl/Application.hpp"
#include "AxoPlotl/tools/DataControlTool.hpp"
#include "AxoPlotl/tools/DebugTool.hpp"
#include "AxoPlotl/tools/PlottingTool.hpp"

AxoPlotl::DebugTool tool_debug;
AxoPlotl::DataControlTool tool_data_control;
AxoPlotl::PlottingTool tool_plotting;

void user_ui_callback(AxoPlotl::Application* _app)
{
  tool_data_control.render_ui(*_app);
  tool_plotting.render_ui(*_app);
  tool_debug.render_ui(*_app);
}

int main()
{
    std::cout << std::filesystem::current_path()  << std::endl;

    // Initialize main Runner
    AxoPlotl::Application app;
    app.user_ui_callback() = user_ui_callback;
    if (!app.init()) {
        std::cerr << "Failed to initialice AxoPlotl" << std::endl;
        return 1;
    }
    AxoPlotl::GUI::apply_theme(AxoPlotl::GUI::Theme::ModernLight);

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
