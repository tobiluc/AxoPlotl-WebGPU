#define IMGUI_DEFINE_MATH_OPERATORS
#include "AxoPlotl/gui/widgets/draw_texture.hpp"
#include "AxoPlotl/gui/widgets/input_slider.hpp"
#include "Application.hpp"
#include "AxoPlotl/gui/fonts.hpp"
#include "AxoPlotl/gui/themes.hpp"
#include "AxoPlotl/input/Mouse.hpp"
#include "AxoPlotl/core/objects/SegmentObject.hpp"
#include "AxoPlotl/core/objects/SphericalHarmonicsObject.hpp"
#include "AxoPlotl/core/objects/TetrahedronObject.hpp"
#include "AxoPlotl/rendering/redraw.hpp"
#include "ImGuiFileDialog.h"
#include <cassert>
#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include <mach/task_info.h>
#include <mach/mach.h>
#include <AxoPlotl/rendering/wgpu_commons.hpp>
#include <AxoPlotl/tools/DataControlTool.hpp>
#include <OpenVolumeMesh/IO/ovmb_write.hh>
#include <imgui_internal.h>
#include <AxoPlotl/IO/texture_io.hpp>
#include <AxoPlotl/utils/ScopeExit.hpp>

#ifdef WEBGPU_BACKEND_WGPU
#  include <webgpu/wgpu.h>
#endif // WEBGPU_BACKEND_WGPU

void wgpuPollEvents(
    [[maybe_unused]] wgpu::Device _device,
    [[maybe_unused]] bool _yield_to_browser) {
#if defined(WEBGPU_BACKEND_DAWN)
    _device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
    _device.poll(false, nullptr);
#endif
}

namespace AxoPlotl::internal
{

Application::Application() :
current_scene_index_(0),
viewport_(this)
{
    auto default_scene = std::make_unique<Scene>();
    scenes_.push_back(std::move(default_scene));
}

Application::~Application()
{
    cleanup();
    glfwTerminate();
}

Scene& Application::add_scene()
{
    current_scene_index_ = scenes_.size();
    auto new_scene = std::make_unique<Scene>();
    new_scene->init(this);
    scenes_.push_back(std::move(new_scene));
    return *scenes_.back();
}

bool Application::set_scene(const Scene& _scene)
{
    for (uint32_t scene_idx = 0; scene_idx < scenes_.size(); ++scene_idx) {
        if (scenes_.at(scene_idx).get() == &_scene) {
            return set_scene(scene_idx);
        }
    }
    return false;
}

bool Application::set_scene(uint32_t _scene_index)
{
    if (_scene_index >= scenes_.size()) {return false;}
    current_scene_index_ = _scene_index;
    return true;
}

constexpr bool Application::has_scene() const {
    return current_scene_index_ < scenes_.size();
}

void Application::Time::update()
{
    // Compute frames per second
    float time_of_current_frame = (float)glfwGetTime();
    delta_time_ = time_of_current_frame - time_of_last_frame_;
    time_of_last_frame_ = time_of_current_frame;
    seconds_since_last_frame_ += delta_time_;
    if (seconds_since_last_frame_ >= 1.0f) {
        seconds_since_last_frame_ -= 1.0f;
        frames_per_second_ = 1.0f / delta_time_;
    }
}

bool Application::init_glfw()
{
    if (window_) {
        glfwDestroyWindow(window_);
    }

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window_ = glfwCreateWindow(640, 480, "AxoPlotl - WebGPU", nullptr, nullptr);

    return window_;
}

bool Application::init()
{
    // Create Window
    if (initialized_ || !init_glfw()) {return false;}

    //-----------
    // Callbacks
    //-----------
    // Handle Resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, int width, int height) {
        auto app_ptr = reinterpret_cast<AxoPlotl::internal::Application*>(glfwGetWindowUserPointer(window));
        if (app_ptr) {
            app_ptr->on_window_resize(width, height);
        }
    });

    glfwSetCursorPosCallback(window_, AxoPlotl::internal::mouse_callback);
    glfwSetScrollCallback(window_, AxoPlotl::internal::scroll_callback);
    glfwSetMouseButtonCallback(window_, [](GLFWwindow*,int,int,int) {trigger_redraw();});
    glfwSetKeyCallback(window_, [](GLFWwindow*,int,int,int,int) {trigger_redraw();});

    // Drop Callback -> Drag in Files to load them
    glfwSetDropCallback(window_, [](GLFWwindow* window, int count, const char** paths) {

        auto app_ptr = static_cast<AxoPlotl::internal::Application*>(glfwGetWindowUserPointer(window));
        if (!app_ptr) {return;}

        for (int i = 0; i < count; ++i) {
            app_ptr->scene().add_mesh(std::filesystem::path(paths[i]));
        }
    });

    //------------------
    // GPU Context
    //------------------
    wgpu::Instance instance = wgpuCreateInstance(nullptr);
    render_context_.surface = glfwGetWGPUSurface(instance, window_);
    gpu_context_.init(instance, render_context_.surface);

    //----------------
    // Render Context
    //----------------
    const auto fb_size = framebuffer_size();
    render_context_.configure_surface(gpu_context_, fb_size.x, fb_size.y);

    //-------------------
    // Special Textures
    //-------------------
    wgpu::DepthStencilState depthStencilState = create_default_depth_state();
    viewport_.resize(gpu_context_.device, render_context_.surface_format, framebuffer_size().x, framebuffer_size().y);

    //----------
    // Gui
    //----------
    if (!init_imgui()) {
        return false;
    }

    //----------
    // Scenes
    //----------
    for (auto& scene : scenes_) {
        scene->init(this);
    }

    //----------
    // Picker
    //----------
    picker_.init(gpu_context_.device);
    picking_renderer_.init(gpu_context_.device, 256, 256);

    std::cout << "AxoPlotlApp initialized." << std::endl;
    initialized_ = true;
    return true;
}

void Application::frame_tick(bool _headless)
{
    //------------------
    // Frame Update
    //------------------
    time_.update();
    Mouse::update(window_);
    glfwPollEvents();

    //------------------
    // Frame Maintenance
    //------------------
    auto frame_cleanup = ScopeExit([&]() noexcept {

        // We take a snapshot of deferred calls such
        // that one deferred call can schedule another one
        // without a concurrent modification exception.
        auto deferred = std::exchange(deferred_calls_, {});
        for (auto& fn : deferred) {
            try {fn();}
            catch (const std::exception& e) {
                std::cerr << "Deferred call failed: "
                          << e.what() << '\n';
            } catch (...) {
                std::cerr << "Deferred call failed: unknown exception\n";
            }
        }

        try {
            wgpuPollEvents(gpu_context_.device, false);
        } catch (...) {
            std::cerr << "GPU polling failed\n";
        }
    });

    //------------------
    // Update
    //------------------
    if (!should_redraw()) {
        return;
    }

    //------------------
    // Acquire Surface
    //------------------
    wgpu::SurfaceTexture surface_texture{};

    render_context_.surface.getCurrentTexture(
        &surface_texture
        );

    if (surface_texture.status !=
        wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal)
    {
        // No rendering this frame.
        // Frame maintenance still runs through ScopeExit.
        return;
    }

    //------------------
    // Surface Texture Cleanup
    //------------------
// #ifndef WEBGPU_BACKEND_WGPU
    auto surface_texture_cleanup = ScopeExit([&]() noexcept {
        if (surface_texture.texture) {
            wgpuTextureRelease(surface_texture.texture);
            surface_texture.texture = nullptr;
        }
    });
// #endif

    //------------------
    // Build UI
    //------------------
    if (!_headless) {
        draw_editor_ui();
    }

    //------------------
    // Surface View
    //------------------
    wgpu::TextureViewDescriptor view_desc{};

    view_desc.format = render_context_.surface_format;
    view_desc.dimension = wgpu::TextureViewDimension::_2D;
    view_desc.baseMipLevel = 0;
    view_desc.mipLevelCount = 1;
    view_desc.baseArrayLayer = 0;
    view_desc.arrayLayerCount = 1;
    view_desc.aspect = wgpu::TextureAspect::All;

    auto target_view =
        static_cast<wgpu::TextureView>(
            wgpuTextureCreateView(
                surface_texture.texture,
                &view_desc
                )
            );

    auto target_view_cleanup = ScopeExit([&]() noexcept {
        target_view.release();
    });

    //------------------
    // Command Encoder
    //------------------
    auto encoder = gpu_context_.device.createCommandEncoder();

    auto encoder_cleanup = ScopeExit([&]() noexcept {
        encoder.release();
    });

    //------------------
    // Render Scene
    //------------------
    if (viewport_.visible() && viewport_.valid())
    {
        render_scene(
            encoder,
            viewport_
            );

        //------------------
        // Click to Pick
        //------------------
        if (Mouse::RIGHT_JUST_PRESSED)
        {
            const auto pos = viewport().mouse_to_local();

            if (pos.has_value())
            {
                picker_.request_pixel(
                    viewport(),
                    scene().camera(),
                    pos->x,
                    pos->y
                    );

                if (picker_.pixel().entity.object_id < UINT32_MAX) {
                    requests_.pick_result = true;
                }
            }
        }

        //------------------
        // Picking Debug
        //------------------
        picking_renderer_.render(
            encoder,
            viewport_.picking_view()
            );
    }

    //------------------
    // Render GUI
    //------------------
    if (!_headless) {
        render_gui(
            encoder,
            target_view
            );
    }

    //------------------
    // Submit
    //------------------
    auto cmd_buffer = encoder.finish();

    auto cmd_buffer_cleanup = ScopeExit([&]() noexcept {
        cmd_buffer.release();
    });

    gpu_context_.device.getQueue().submit(
        1,
        &cmd_buffer
        );

    //------------------
    // Present
    //------------------
    render_context_.surface.present();
}

int Application::run(RunConfig _cfg)
{
    if (!initialized_) { // safety measure
        if (!init()) {
            return 1;
        }
    }
    glfwSetWindowShouldClose(window(), GLFW_FALSE); // reopen window
    if (_cfg.num_ticks.has_value()) {
        *_cfg.num_ticks = std::max(*_cfg.num_ticks, 1llu);
    }
    while (!glfwWindowShouldClose(window()) && (!_cfg.num_ticks.has_value() || (--(*_cfg.num_ticks) > 0llu))) {
        frame_tick(_cfg.headless);
    }
    if (!_cfg.manual_cleanup) {
        cleanup();
    }
    return 0;
}

void Application::on_window_resize(float width, float height)
{
    if (width <= 0|| height <= 0) {return;} // window minimized

    //const auto fb_size = framebuffer_size();
    render_context_.configure_surface(gpu_context_, width, height);

    wgpuPollEvents(gpu_context_.device, true);

    trigger_redraw();
}

void Application::cleanup()
{
    if (!initialized_) {return;}

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();
    ImGui::DestroyContext();

    for (auto& scene : scenes_) {
        scene->destroy();
    }

    viewport_.release();
    render_context_.release();
    gpu_context_.release();

    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    std::cout << "AxoPlotlApp cleared." << std::endl;

    initialized_ = false;
}

bool Application::init_imgui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window_, true);
    wgpu::SurfaceCapabilities surf_caps = render_context_.get_surface_capabilities(gpu_context_);

    // new imgui init
    ImGui_ImplWGPU_InitInfo imgui_init_info{};
    imgui_init_info.Device = gpu_context_.device;
    imgui_init_info.NumFramesInFlight = 3;
    imgui_init_info.RenderTargetFormat = surf_caps.formats[0];
    //imgui_init_info.RenderTargetFormat = wgpu::TextureFormat::BGRA8Unorm;
    // imgui_init_info.DepthStencilFormat = depth_texture_format_;
    imgui_init_info.DepthStencilFormat = WGPUTextureFormat_Undefined;
    imgui_init_info.PipelineMultisampleState.count = 1;
    if (!ImGui_ImplWGPU_Init(&imgui_init_info)) {
        std::cerr << "Failed to initialize ImGui WebGPU backend\n";
        return false;
    }

    // This is with the older imgui version from Learn WebGPU
    //ImGui_ImplWGPU_Init(device_, 3, surf_caps.formats[0], depthTextureFormat);

    // Font
    gui::load_fonts();

    return true;
}

void Application::draw_viewport_ui()
{
    viewport_.draw_ui(gpu_context_.device, render_context_.surface_format);

    // Clicking on an Object
    if (requests_.pick_result) [[unlikely]] {
        ImGui::OpenPopup("PickingPopup");
        requests_.pick_result = false;
    }
    if (picker_.pixel().entity.object_id < UINT32_MAX && ImGui::BeginPopup("PickingPopup")) {
        // std::cerr << picker_.pixel().object_id_ << std::endl;
        auto obj = scene().get_object(picker_.pixel().entity.object_id);
        if (obj) [[likely]] {obj->draw_picking_ui(picker_);}
        ImGui::EndPopup();
    }
}

void Application::draw_main_menu_bar_ui()
{
    //--------------------
    // Top Menu Bar
    //--------------------
    ImGui::SetNextWindowPos(ImVec2(-100,-100));
    ImGui::SetNextWindowSize(ImVec2(0,0));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoInputs |
                             ImGuiWindowFlags_MenuBar;
    ImGui::Begin("MenuBar", nullptr, flags);
    //ImGui::SetWindowFontScale(font_scale_);

    //ImGui::Text("Render Settings");
    //ImGui::ColorEdit3("Background", clear_color_);
    if (ImGui::BeginMainMenuBar())
    {
        // File menu
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load Scene", "Ctrl+O")) {
                IGFD::FileDialogConfig config;
                config.path = "..";
                ImGuiFileDialog::Instance()->OpenDialog(
                    "LoadSceneDialogKey", "Choose File",
                    "(*.json){.json}",
                    config);
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                IGFD::FileDialogConfig config;
                config.path = "..";
                ImGuiFileDialog::Instance()->OpenDialog(
                    "SaveSceneDialogKey", "Choose File",
                    "(*.json){.json}",
                    config);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Load Mesh")) {
                IGFD::FileDialogConfig config;
                config.path = "..";
                ImGuiFileDialog::Instance()->OpenDialog(
                    "LoadMeshDialogKey", "Choose File",
                    "Mesh files (*.obj *.ovm *.ovmb *ply){.obj,.ovm,.ovmb,.ply}",
                    config);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                glfwSetWindowShouldClose(window_, GLFW_TRUE);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::BeginMenu("Add Object"))
            {
                if (ImGui::MenuItem("Spherical Harmonics")) {
                    scene().add_object<SHObject>();
                }
                if (ImGui::MenuItem("Tetrahedron")) {
                    scene().add_object<TetObject>();
                }
                if (ImGui::MenuItem("Segment")) {
                    scene().add_object<SegmentObject>();
                }
                ImGui::EndMenu(); //!Add Object
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Scene")) {
                if (ImGui::MenuItem("Add Scene")) {
                    add_scene();
                }
                if (ImGui::MenuItem("Delete Scene")) {
                    if (scenes_.size() > 1lu) {
                        scene().delete_objects();
                        call_deferred([&]() {
                            scenes_.erase(scenes_.begin()+current_scene_index_);
                            current_scene_index_ = 0lu;
                        });
                    }
                }
                ImGui::EndMenu(); //!Scene
            }
            ImGui::EndMenu(); //!Edit
        }

        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::BeginMenu("Layout")) {
                ImGui::Checkbox("Enable Inspector", &inspector_.enabled);
                ImGui::Checkbox("Enable Scenes Bar", &scenes_tab_bar_enabled_);
                requests_.reset_layout |= ImGui::Button("Reset Layout");
                ImGui::EndMenu(); //!Inspector Settings
            }

            if (ImGui::BeginMenu("Scene")) {
                scene().draw_settings_ui();
                ImGui::EndMenu(); //!Scene Settings
            }

            if (ImGui::BeginMenu("Theme")) {
                if (ImGui::MenuItem("Dark")) {gui::apply_theme(gui::Theme::Dark);}
                if (ImGui::MenuItem("Light")) {gui::apply_theme(gui::Theme::Light);}
                if (ImGui::MenuItem("Classic")) {gui::apply_theme(gui::Theme::Classic);}
                if (ImGui::MenuItem("Modern Dark")) {gui::apply_theme(gui::Theme::ModernDark);}
                if (ImGui::MenuItem("Modern Light")) {gui::apply_theme(gui::Theme::ModernLight);}

                ImGui::EndMenu(); //!Theme Settings
            }

            ImGui::EndMenu(); // !Settings
        }
        ImGui::EndMainMenuBar();
    }

    // Load Scene File Dialog
    if (ImGuiFileDialog::Instance()->Display("LoadSceneDialogKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::filesystem::path filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            scene().load_from_file(filepath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // Save Scene File Dialog
    if (ImGuiFileDialog::Instance()->Display("SaveSceneDialogKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::filesystem::path filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            scene().save_to_file(filepath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // Load Mesh File Dialog
    if (ImGuiFileDialog::Instance()->Display("LoadMeshDialogKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::filesystem::path filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            scene().add_mesh(filepath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::End(); //!MenuBar
}

void Application::draw_scenes_tab_bar_ui()
{
    if (!scenes_tab_bar_enabled_) {return;}

    ImGui::Begin("Scenes");

    if (ImGui::BeginTabBar("Scenes")) {
        for (uint32_t i = 0; i < scenes_.size(); ++i) {
            ImGui::PushID(i);

            std::string name = "Scene " + std::to_string(i);

            if (ImGui::BeginTabItem(name.c_str())) {
                set_scene(i);
                ImGui::EndTabItem();
            }

            ImGui::PopID();
        }

        // Add a Scene Button
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_PLUS)) {
            add_scene();
        }
        ImGui::SetItemTooltip("Add Scene");

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Application::draw_inspector_ui()
{
    if (!inspector_.enabled) {return;}

    if (!ImGui::Begin("Inspector")) {
        ImGui::End();
        return;
    }

    inspector_.callback();

    ImGui::End();
}

void Application::build_default_layout(ImGuiID dockspace_id)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    // Remove the existing layout.
    ImGui::DockBuilderRemoveNode(dockspace_id);

    // Create a new dockspace.
    ImGui::DockBuilderAddNode(
        dockspace_id,
        ImGuiDockNodeFlags_DockSpace
        );

    ImGui::DockBuilderSetNodeSize(
        dockspace_id,
        viewport->WorkSize
        );

    // Inspector on the right (25%).
    ImGuiID inspector_id;
    ImGuiID viewport_id;

    ImGui::DockBuilderSplitNode(
        dockspace_id,
        ImGuiDir_Right,
        0.25f,
        &inspector_id,
        &viewport_id
        );

    // Scenes tab to the bottom of the central area (20%).
    ImGuiID scenes_id;

    ImGui::DockBuilderSplitNode(
        viewport_id,
        ImGuiDir_Down,
        0.20f,
        &scenes_id,
        &viewport_id
        );

    // Dock the windows.
    ImGui::DockBuilderDockWindow("Inspector", inspector_id);
    ImGui::DockBuilderDockWindow("Scenes", scenes_id);
    ImGui::DockBuilderDockWindow("Viewport", viewport_id);

    // Hide Tab Bar (with node title) as Default
    ImGui::DockBuilderGetNode(scenes_id)->LocalFlags |=
        ImGuiDockNodeFlags_HiddenTabBar;
    ImGui::DockBuilderGetNode(inspector_id)->LocalFlags |=
        ImGuiDockNodeFlags_HiddenTabBar;
    ImGui::DockBuilderGetNode(viewport_id)->LocalFlags |=
        ImGuiDockNodeFlags_HiddenTabBar;

    ImGui::DockBuilderFinish(dockspace_id);
}

void Application::draw_editor_ui()
{
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    draw_main_menu_bar_ui();

    ImGuiID dockspace_id = ImHashStr("AxoPlotlDockSpace");

    ImGui::DockSpaceOverViewport(
        dockspace_id,
        ImGui::GetMainViewport()
        );

    // Rebuild the layout when requested.
    if (requests_.reset_layout) {
        inspector_.enabled = true;
        scenes_tab_bar_enabled_ = true;
        build_default_layout(dockspace_id);
        requests_.reset_layout = false;
    }

    draw_scenes_tab_bar_ui();

    draw_viewport_ui();

    draw_inspector_ui();

    ImGui::Render();
}

void Application::render_gui(
    wgpu::CommandEncoder& encoder,
    wgpu::TextureView& target_view)
{
    wgpu::RenderPassColorAttachment attachment{};

    attachment.view = target_view;
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;
    attachment.clearValue = {
        scene().settings.clear_color[0],
        scene().settings.clear_color[1],
        scene().settings.clear_color[2],
        1.0f
    };

    wgpu::RenderPassDescriptor desc{};
    desc.colorAttachmentCount = 1;
    desc.colorAttachments = &attachment;

    auto pass = encoder.beginRenderPass(desc);

    ImGui_ImplWGPU_RenderDrawData(
        ImGui::GetDrawData(),
        pass
        );

    pass.end();
}

void Application::render_scene(wgpu::CommandEncoder& encoder, Viewport& _viewport)
{
    wgpu::RenderPassColorAttachment attachments[2]{};

    attachments[0].view = _viewport.color_view();
    attachments[0].loadOp = wgpu::LoadOp::Clear;
    attachments[0].storeOp = wgpu::StoreOp::Store;
    attachments[0].clearValue = {
        scene().settings.clear_color[0],
        scene().settings.clear_color[1],
        scene().settings.clear_color[2],
        1.0f
    };

    attachments[1].view = _viewport.picking_view();
    attachments[1].loadOp = wgpu::LoadOp::Clear;
    attachments[1].storeOp = wgpu::StoreOp::Store;
    attachments[1].clearValue = {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF
    };

    wgpu::RenderPassDepthStencilAttachment depth{};
    depth.view = _viewport.depth_view();
    depth.depthClearValue = 1.0f;
    depth.depthLoadOp = wgpu::LoadOp::Clear;
    depth.depthStoreOp = wgpu::StoreOp::Store;
    depth.depthReadOnly = false;
    depth.stencilClearValue = 0;
    depth.stencilReadOnly = true;

// Match the stencil operations to your existing depth format.
#ifdef WEBGPU_BACKEND_DAWN
    depth.stencilLoadOp = wgpu::LoadOp::Undefined;
    depth.stencilStoreOp = wgpu::StoreOp::Undefined;
#endif

    wgpu::RenderPassDescriptor desc{};
    desc.colorAttachmentCount = 2;
    desc.colorAttachments = attachments;
    desc.depthStencilAttachment = &depth;

    auto pass = encoder.beginRenderPass(desc);

    pass.setViewport(
        0, 0,
        _viewport.width(), _viewport.height(),
        0, 1
        );

    pass.setScissorRect(0, 0, _viewport.width(), _viewport.height());

    scene().render(pass, ViewSize{
        .width=static_cast<float>(_viewport.width()),
        .height=static_cast<float>(_viewport.height())},
        viewport().hovered());

    pass.end();
}

bool Application::save_viewport_screenshot(
    const std::filesystem::path& _path,
    const ViewSize _size,
    bool _transparent_background)
{
    assert(render_context_.surface_format == viewport_.color_texture().getFormat());
    wgpu::CommandEncoder encoder = gpu_context_.device.createCommandEncoder();
    const auto prev_scene_size = viewport().size();
    const auto prev_pick_size = ViewSize{
        .width=static_cast<float>(picking_renderer_.width()),
        .height=static_cast<float>(picking_renderer_.height())
    };
    const auto prev_scene_settings = scene().settings;

    // Resize to desired size
    if (_transparent_background) {
        picking_renderer_.resize(_size.width, _size.height);
        picking_renderer_.mode() = PickingTextureVis::Mode::ObjectID;
    }
    viewport_.resize(gpu_context_.device, render_context_.surface_format, _size.width, _size.height);

    // Render
    scene().settings.show_orbit_target_cross = false;
    scene().settings.show_axis_cross = false;
    render_scene(encoder, viewport_);
    if (_transparent_background) {
        picking_renderer_.render(encoder, viewport_.picking_view());
    }
    scene().settings = prev_scene_settings;

    wgpu::CommandBuffer cb = encoder.finish();
    gpu_context_.device.getQueue().submit(1, &cb);
    cb.release();
    encoder.release();

    // Save Texture
    std::optional<wgpu::Texture> mask_texture = std::nullopt;
    if (_transparent_background) {mask_texture = picking_renderer_.texture();}
    bool res = save_texture_to_file(_path, gpu_context_.device,
        viewport_.color_texture(), _size.width, _size.height,
        mask_texture
    );

    // Reset Size
    viewport_.resize(gpu_context_.device, render_context_.surface_format, prev_scene_size.width, prev_scene_size.height);
    if (_transparent_background) {
        picking_renderer_.resize(prev_pick_size.width, prev_pick_size.height);
    }
    return res;
}

}
