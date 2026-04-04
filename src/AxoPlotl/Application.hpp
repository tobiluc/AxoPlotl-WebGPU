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

class Application
{
private:
    float clear_color_[3] = {1,1,1};
    PickConfig picking_config_;

    struct Time {
        float delta_time_ = 0;
        float frames_per_second_ = 0;
        float seconds_since_last_frame_ = 0;
        float time_of_last_frame_ = 0;
        void update();
    } time_;

    float font_scale_ = 1.0f;
    float inspector_rel_width_ = 0.3f;
    bool inspector_right_aligned_ = false;
    bool inspector_enabled_ = true;
    std::function<void(Application* _app)> inspector_callback_;

public:
    Application();

    ~Application();

    // init everything and return true if it went all right
    bool init();

    // Draw a frame and handle events
    void frame_tick();

    void run();

    void on_window_resize(float width, float height);

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

    inline std::function<void(Application* _app)>& inspector_callback() {
        return inspector_callback_;
    }

    inline bool& inspector_enabled() {
        return inspector_enabled_;
    }

    PickResult request_pick_result(float _x, float _y);

    inline constexpr float fps() const {
        return time_.frames_per_second_;
    }

private:
    PickResult pick_result_;

    // Processed at the end of a frame and then removed
    std::vector<std::function<void()>> deferred_calls_;

    GLFWwindow* window_;
    wgpu::Queue queue_;
    //wgpu::TextureFormat color_format_ = wgpu::TextureFormat::Undefined;
    wgpu::Texture depth_texture_;
    wgpu::TextureView depth_texture_view_;
    wgpu::TextureFormat depth_texture_format_ = wgpu::TextureFormat::Undefined;
    wgpu::Texture picking_texture_;
    wgpu::TextureView picking_view_;
    wgpu::Buffer picking_buffer_;
    wgpu::RenderPassEncoder gui_render_pass_;
    wgpu::RenderPassEncoder scene_render_pass_;

    std::unique_ptr<wgpu::ErrorCallback> error_callback_;

    Scene scene_;

    void clear();

    bool init_glfw();

    bool init_imgui();

    void render_imgui(
        wgpu::RenderPassEncoder _render_pass, bool _just_clicked_on_object);

    void configure_surface();

    void create_depth_texture();

    void create_picking_texture();
};

}
