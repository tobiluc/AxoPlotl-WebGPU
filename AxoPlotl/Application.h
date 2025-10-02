#pragma once

#include <webgpu/webgpu.h>
#include <GLFW/glfw3.h>

namespace AxoPlotl
{

class Application {
public:
    // Initialize everything and return true if it went all right
    bool Initialize();

    // Uninitialize everything that was initialized
    void Terminate();

    // Draw a frame and handle events
    void MainLoop();

    // Return true as long as the main loop should keep on running
    bool IsRunning();

private:
    GLFWwindow* window = nullptr;
    WGPUQueue queue;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUSurface surface;
};

}
