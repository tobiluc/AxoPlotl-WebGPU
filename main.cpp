#define WEBGPU_CPP_IMPLEMENTATION

#include "AxoPlotl/Application.h"

int main()
{
    AxoPlotl::Application app;

    if (!app.init()) {
        return 1;
    }

#ifdef __EMSCRIPTEN__
    auto callback = [](void *arg) {
        Application* pApp = reinterpret_cast<Application*>(arg);
        pApp->MainLoop();
    };
    emscripten_set_main_loop_arg(callback, &app, 0, true);
#else // __EMSCRIPTEN__
    while (app.isRunning()) {
        app.mainLoop();
    }
#endif // __EMSCRIPTEN__

    return 0;
}
