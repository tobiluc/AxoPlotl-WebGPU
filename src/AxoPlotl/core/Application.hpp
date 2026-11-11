#pragma once

#include <AxoPlotl/core/Scene.hpp>
#include <AxoPlotl/core/Viewport.hpp>
#include "AxoPlotl/core/picking.hpp"
#include "AxoPlotl/rendering/debug/PickingTextureVis.hpp"
#include <webgpu/webgpu.hpp>
#include <AxoPlotl/rendering/MyGpuContext.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__


namespace AxoPlotl::internal
{

// DO NOT USE THIS CLASS DIRECTLY!
// INSTEAD USE THE FREE FUNCTIONS IN THE AXOPLOTL NAMESPACE
class Application
{
private:
    // PickConfig picking_config_;
    // PickingTextureRenderer picking_visualizer_;

    struct Time {
        float delta_time_ = 0;
        float frames_per_second_ = 0;
        float seconds_since_last_frame_ = 0;
        float time_of_last_frame_ = 0;
        void update();
    } time_;

    struct Inspector {
        bool enabled = true;
        std::function<void()> callback = [](){}; // cursed stuff...
    } inspector_;

    struct Requests {
        bool reset_layout = false;
        bool pick_result = false;
    } requests_;

    GpuContext gpu_context_;
    RenderContext render_context_;

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

    glm::vec<2,int> framebuffer_size() const {
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
        return {fbWidth, fbHeight};
    }

    const GpuContext& gpu_context() const {return gpu_context_;}
    const RenderContext& render_context() const {return render_context_;}

    const Viewport& viewport() const {return viewport_;}
    Viewport& viewport() {return viewport_;}

    bool save_viewport_screenshot(const std::filesystem::path& _path, const ViewSize _size);

    void call_deferred(std::function<void()> _f) {
        deferred_calls_.push_back(_f);
    }

    std::function<void()>& inspector_callback() {
        return inspector_.callback;
    }

    bool& inspector_enabled() {
        return inspector_.enabled;
    }

    constexpr float fps() const {
        return time_.frames_per_second_;
    }

private:
    struct Debugging
    {
        DebugVis_PickingTextureRenderer picking_renderer_;
        bool display_picking_texture = false;
        std::string screenshot_filename_ = "AxoPlotl_Screenshot.png";
        float screenshot_width_ = 512;
        float screenshot_height_ = 512;
    } debug;

    Picker picker_;

    // Processed at the end of a frame and then removed
    std::vector<std::function<void()>> deferred_calls_;

    GLFWwindow* window_;
    Viewport viewport_;

    // wgpu::RenderPassEncoder gui_render_pass_;
    // wgpu::RenderPassEncoder scene_render_pass_;

    std::vector<Scene> scenes_;
    uint32_t current_scene_index_ = 0;

    void clear();

    bool init_glfw();

    bool init_imgui();

    void render_viewport_ui();

    void render_editor_ui();

    void render_scene(
        wgpu::CommandEncoder& _encoder,
        Viewport& _viewport);

    void render_gui(wgpu::CommandEncoder& _encoder,
                    wgpu::TextureView& _target_view);

    void render_main_menu_bar_ui();

    void render_scenes_tab_bar_ui();

    void render_inspector_ui();

    void build_default_layout(ImGuiID dockspace_id);

    constexpr bool has_scene() const {
        return current_scene_index_ < scenes_.size();
    }
};

}
