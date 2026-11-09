#pragma once

#include "AxoPlotl/Scene.hpp"
#include "AxoPlotl/Viewport.hpp"
#include "AxoPlotl/picking.hpp"
#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__


namespace AxoPlotl
{

// DO NOT USE THIS CLASS DIRECTLY!
// INSTEAD USE THE FREE FUNCTIONS IN THE AXOPLOTL NAMESPACE
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

    //float font_scale_ = 1.0f;
    bool inspector_enabled_ = true;
    std::function<void()> inspector_callback_;

    bool reset_layout_requested_ = false;
    bool pick_result_requested_ = false;

    // Draw a frame and handle events
    void frame_tick();

public:
    Application();

    ~Application();

    // init everything and return true if it went all right
    bool init();

    void run();

    void on_window_resize(float width, float height);

    Scene& scene() {return scenes_.at(current_scene_index_);}

    Scene& add_scene();

    GLFWwindow* window() {return window_;}

    wgpu::Device device_;
    wgpu::Adapter adapter_;
    wgpu::Surface surface_;

    glm::vec<2,int> framebuffer_size() const {
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
        return {fbWidth, fbHeight};
    }

    const Viewport& viewport() const {return viewport_;}

    void call_deferred(std::function<void()> _f) {
        deferred_calls_.push_back(_f);
    }

    std::function<void()>& inspector_callback() {
        return inspector_callback_;
    }

    bool& inspector_enabled() {
        return inspector_enabled_;
    }

    constexpr float fps() const {
        return time_.frames_per_second_;
    }

private:
    PickResult pick_result_;

    PickResult request_pick_result(float _x, float _y);

    // Processed at the end of a frame and then removed
    std::vector<std::function<void()>> deferred_calls_;

    GLFWwindow* window_;
    wgpu::Queue queue_;

    Viewport viewport_;
    wgpu::TextureFormat surface_format_ = wgpu::TextureFormat::Undefined;

    wgpu::TextureFormat depth_texture_format_ = wgpu::TextureFormat::Undefined;

    wgpu::Buffer picking_buffer_;
    wgpu::RenderPassEncoder gui_render_pass_;
    wgpu::RenderPassEncoder scene_render_pass_;

    std::vector<Scene> scenes_;
    uint32_t current_scene_index_ = 0;

    void clear();

    bool init_glfw();

    bool init_imgui();

    void render_viewport_ui();

    void render_editor_ui();

    void render_scene(wgpu::CommandEncoder& _encoder);

    void render_gui(wgpu::CommandEncoder& _encoder,
                    wgpu::TextureView& _target_view);

    // void render_imgui(
    //     wgpu::RenderPassEncoder _render_pass, bool _just_clicked_on_object);

    void render_main_menu_bar_ui();

    void render_scenes_tab_bar_ui();

    void render_inspector_ui();

    void configure_surface();

    void build_default_layout(ImGuiID dockspace_id);

    constexpr bool has_scene() const {
        return current_scene_index_ < scenes_.size();
    }
};

}
