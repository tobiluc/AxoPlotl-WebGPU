#pragma once

#include "AxoPlotl/Scene.hpp"
#include "AxoPlotl/picking.hpp"
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

    ~Application() = default;

    // init everything and return true if it went all right
    bool init();

    // Draw a frame and handle events
    void run();

    void on_window_resize(float width, float height);

    void terminate();

    float clear_color_[3] = {1,1,1};
    float font_scale_ = 1.0f;

    inline Scene& scene() {return scene_;}

    inline GLFWwindow* window() {return window_;}

    wgpu::Device device_;
    wgpu::Adapter adapter_;
    wgpu::Surface surface_;

    glm::vec<4,float> total_viewport();

    glm::vec<4,float> scene_viewport();

    inline void add_deferred_call(std::function<void()> _f) {
        deferred_calls_.push_back(_f);
    }

    inline std::function<void(Application* _app)>& user_ui_callback() {
        return user_ui_callback_;
    }

    PickResult request_pick_result(float _x, float _y);

private:
    //float sidebar_width_ = 300.0f;
    float sidebar_rel_width_ = 0.3f;
    bool sidebar_right_aligned_ = false;
    PickResult pick_result_;

    std::function<void(Application* _app)> user_ui_callback_;

    // Processed at the end of a frame and then removed
    std::vector<std::function<void()>> deferred_calls_;

    GLFWwindow* window_;
    wgpu::Queue queue_;
    wgpu::TextureFormat color_format_ = wgpu::TextureFormat::Undefined;
    wgpu::Texture depthTexture;
    wgpu::TextureView depthTextureView;
    wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Undefined;
    wgpu::Texture picking_texture_;
    wgpu::TextureView picking_view_;
    wgpu::Buffer picking_buffer_;

    std::unique_ptr<wgpu::ErrorCallback> error_callback_;

    Scene scene_;

    bool init_gui();

    void update_gui(
        wgpu::RenderPassEncoder _render_pass, bool _just_clicked_on_object);

    void configure_surface();

    void create_depth_texture();

    void create_picking_texture();
};

}
