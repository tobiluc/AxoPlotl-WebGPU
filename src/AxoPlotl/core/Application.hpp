#pragma once

#include <AxoPlotl/core/Scene.hpp>
#include <AxoPlotl/core/Viewport.hpp>
#include "AxoPlotl/core/picking.hpp"
#include "AxoPlotl/rendering/PickingTextureVis.hpp"
#include <webgpu/webgpu.hpp>
#include <AxoPlotl/rendering/MyGpuContext.hpp>
#include <AxoPlotl/core/RunConfig.hpp>

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

    struct Time {
        float delta_time_ = 0;
        float frames_per_second_ = 0;
        float seconds_since_last_frame_ = 0;
        float time_of_last_frame_ = 0;
        void update();
    } time_;

    struct Inspector {
        bool enabled = true;
        std::function<void()> callback = [](){}; // yeah...
    } inspector_;

    struct Requests {
        bool reset_layout = false;
        bool pick_result = false;
    } requests_;

    bool scenes_tab_bar_enabled_ = true;

    GpuContext gpu_context_;
    RenderContext render_context_;

    PickingTextureVis picking_renderer_;

    Picker picker_;

    // Processed at the end of a frame and then removed
    std::vector<std::function<void()>> deferred_calls_;

    GLFWwindow* window_;
    Viewport viewport_;

    std::vector<std::unique_ptr<Scene>> scenes_;
    uint32_t current_scene_index_ = 0;

    bool initialized_ = false;

public:
    Application();

    ~Application();

    // init everything and return true if it went all right
    bool init();

    int run(RunConfig _cfg = RunConfig());

    void cleanup();

    void on_window_resize(float width, float height);

    Scene& scene() {return *scenes_.at(current_scene_index_);}

    Scene& add_scene();

    bool set_scene(const Scene& _scene);

    bool set_scene(uint32_t _scene_index);

    constexpr bool has_scene() const;

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

    bool save_viewport_screenshot(const std::filesystem::path& _path, const ViewSize _size, bool _transparent_background = false);

    void call_deferred(std::function<void()> _f) {
        deferred_calls_.push_back(_f);
    }

    std::function<void()>& inspector_callback() {
        return inspector_.callback;
    }

    bool& inspector_enabled() {
        return inspector_.enabled;
    }

    constexpr float frames_per_second() const {
        return time_.frames_per_second_;
    }

    void render_picking_texture_ui() {
        picking_renderer_.resize(viewport_.width(), viewport_.height());
        picking_renderer_.draw_ui();
    }

    const Picker::Pixel& picked_pixel() const {
        return picker_.pixel();
    }

private:
    // Draw a frame and handle events
    void frame_tick(bool _headless = false);

    bool init_glfw();

    bool init_imgui();

    void draw_viewport_ui();

    void draw_scenes_tab_bar_ui();

    void draw_inspector_ui();

    void draw_main_menu_bar_ui();

    // Calls the individual ui functions (Viewport/Scenes Tab/Inspector/...)
    void draw_editor_ui();

    void render_scene(
        wgpu::CommandEncoder& _encoder,
        Viewport& _viewport);

    void render_gui(wgpu::CommandEncoder& _encoder,
                    wgpu::TextureView& _target_view);

    void build_default_layout(ImGuiID dockspace_id);
};

}
