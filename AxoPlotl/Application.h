#pragma once

#include "rendering/Renderer.h"
#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__


namespace AxoPlotl
{

class Application {
public:

    Application();

    ~Application();

    // init everything and return true if it went all right
    bool init();

    // Uninitialize everything that was initialized
    //void Terminate();

    // Draw a frame and handle events
    void mainLoop();

    // Return true as long as the main loop should keep on running
    bool isRunning();

private:

private:
    GLFWwindow* window = nullptr;
    Renderer renderer;
};

}
