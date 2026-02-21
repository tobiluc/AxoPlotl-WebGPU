#pragma once

#include "AxoPlotl/PluginBase.hpp"
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
    bool init();

    // Draw a frame and handle events
    void run();

    void on_window_resize(float width, float height);

    void terminate();

    float clear_color_[3] = {1,1,1};

    inline Scene& scene() {return scene_;}

    inline GLFWwindow* window() {return window_;}

    wgpu::Device device_;
    wgpu::Adapter adapter_;
    wgpu::Surface surface_;

private:
    GLFWwindow* window_;
    wgpu::Queue queue_;
    wgpu::TextureFormat color_format_ = wgpu::TextureFormat::Undefined;
    wgpu::Texture depthTexture;
    wgpu::TextureView depthTextureView;
    wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Undefined;

    std::unique_ptr<wgpu::ErrorCallback> error_callback_;

    Scene scene_;

    bool init_gui();

    void update_gui(wgpu::RenderPassEncoder _render_pass);

    void configure_surface();

    void create_depth_texture();
};

}
