#pragma once

#include "AxoPlotl/Scene.hpp"
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
    bool init(GLFWwindow* _window);

    // Draw a frame and handle events
    void run();

    void on_window_resize(float width, float height);

    void release();

private:
    wgpu::Device device_;
    wgpu::Queue queue_;
    wgpu::Adapter adapter_;
    wgpu::Surface surface_;
    wgpu::TextureFormat color_format_ = wgpu::TextureFormat::Undefined;

    std::unique_ptr<wgpu::ErrorCallback> error_callback_;

    Scene scene_;

    void reconfigure_surface(float width, float height);
};

}
