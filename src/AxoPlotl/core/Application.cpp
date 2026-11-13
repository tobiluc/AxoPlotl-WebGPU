#include "AxoPlotl/gui/widgets/draw_texture.hpp"
#include "AxoPlotl/gui/widgets/input_slider.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS

#include "Application.hpp"
#include "AxoPlotl/debug/meshes.hpp"
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

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__
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
#elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
    if (_yield_to_browser) {
        emscripten_sleep(100);
    }
#endif
}

namespace AxoPlotl::internal
{

Scene& Application::add_scene()
{
    current_scene_index_ = scenes_.size();
    scenes_.emplace_back();
    scenes_.back().init(this);
    return scenes_.back();
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

Application::Application() :
    current_scene_index_(0),
    viewport_(this)
{
    scenes_.emplace_back();
}

Application::~Application()
{
    clear();
    glfwTerminate();
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
    if (!init_glfw()) {return false;}

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
        scene.init(this);
    }

    //----------
    // Picker
    //----------
    picker_.init(gpu_context_.device);
    debug.picking_renderer_.init(gpu_context_.device, 256, 256);

    return true;
}

void Application::frame_tick()
{
    time_.update();
    Mouse::update(window_);
    glfwPollEvents();

    if (!on_draw()) {
        return;
    }

    // Acquire surface texture.
    wgpu::SurfaceTexture surface_texture;
    render_context_.surface.getCurrentTexture(&surface_texture);

    if (surface_texture.status !=
        wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal) {
        return;
    }

    // Build the ImGui interface FIRST.
    render_editor_ui();

    // Create surface view.
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
            wgpuTextureCreateView(surface_texture.texture, &view_desc));

    // Create command encoder.
    auto encoder = gpu_context_.device.createCommandEncoder();

    // Render scene only if viewport is visible.
    if (viewport_.visible() && viewport_.valid())
    {
        render_scene(
            encoder,
            viewport_);

        //------------------
        // Click to Pick
        //------------------
        if (Mouse::RIGHT_JUST_PRESSED) {
            const auto pos = viewport().mouse_to_local();
            if (pos.has_value()) {
                picker_.request_pixel(viewport(), scene().camera(), pos->x, pos->y);
                if (picker_.pixel().object_id_ < UINT32_MAX) {
                    requests_.pick_result = true;
                }
            }
        }

        // Render to Debug Picking Visualization
        debug.picking_renderer_.render(
            encoder, viewport_.picking_view());
    }

    // Render editor UI onto surface.
    render_gui(encoder, target_view);

    // Submit.
    auto cmd_buffer = encoder.finish();
    gpu_context_.device.getQueue().submit(1, &cmd_buffer);

#ifndef __EMSCRIPTEN__
    render_context_.surface.present();
#endif

    // Release resources as in your original implementation.
    target_view.release();
    cmd_buffer.release();
    encoder.release();

#ifndef WEBGPU_BACKEND_WGPU
    wgpuTextureRelease(surface_texture.texture);
#endif

    wgpuPollEvents(gpu_context_.device, false);

    for (auto& fn : deferred_calls_) {
        fn();
    }
    deferred_calls_.clear();
}

void Application::run()
{
#ifdef __EMSCRIPTEN__
    auto callback = [](void *arg) {
        Application* app_ptr = reinterpret_cast<Application*>(arg);
        app_ptr->run();
    };
    emscripten_set_main_loop_arg(callback, this, 0, true);
#else // __EMSCRIPTEN__
    while (!glfwWindowShouldClose(window())) {
        frame_tick();
    }
#endif // __EMSCRIPTEN__
    clear();
    init(); // prepare for next run
}

void Application::on_window_resize(float width, float height)
{
    if (width == 0|| height == 0) {return;} // window minimized

    //const auto fb_size = framebuffer_size();
    render_context_.configure_surface(gpu_context_, width, height);
    viewport_.resize(gpu_context_.device, render_context_.surface_format, width, height);
    debug.picking_renderer_.resize(width, height);

    wgpuPollEvents(gpu_context_.device, true);
}

void Application::clear()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();

    for (auto& scene : scenes_) {
        scene.destroy();
    }

    gpu_context_.release();
    viewport_.release();

    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
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
    GUI::load_fonts();

    return true;
}

void Application::render_viewport_ui()
{
    viewport_.render_ui(gpu_context_.device, render_context_.surface_format);

    // Clicking on an Object
    if (requests_.pick_result) [[unlikely]] {
        ImGui::OpenPopup("PickingPopup");
        requests_.pick_result = false;
    }
    if (picker_.pixel().object_id_ < UINT32_MAX && ImGui::BeginPopup("PickingPopup")) {
        auto obj = scene().get_object(picker_.pixel().object_id_);
        if (obj) [[likely]] {obj->render_ui_picking(picker_);}
        ImGui::EndPopup();
    }

    if (debug.display_picking_texture) {
        debug.picking_renderer_.render_ui();
    }
}

void Application::render_main_menu_bar_ui()
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
            if (ImGui::MenuItem("Load", "Ctrl+O")) {
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
            if (ImGui::BeginMenu("Add"))
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
                ImGui::EndMenu(); //!Add
            }
            ImGui::EndMenu(); //!Edit
        }

        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::BeginMenu("Layout")) {
                ImGui::Checkbox("Enable Inspector", &inspector_.enabled);
                requests_.reset_layout |= ImGui::Button("Reset Layout");
                ImGui::EndMenu(); //!Inspector Settings
            }

            //ImGui::SliderFloat("Font Scale", &font_scale_, 0.2f, 2.0f);

            if (ImGui::BeginMenu("Scene")) {
                scene().render_settings_ui();
                ImGui::EndMenu(); //!Scene Settings
            }

            if (ImGui::BeginMenu("Theme")) {
                if (ImGui::MenuItem("Dark")) {GUI::apply_theme(GUI::Theme::Dark);}
                if (ImGui::MenuItem("Light")) {GUI::apply_theme(GUI::Theme::Light);}
                if (ImGui::MenuItem("Classic")) {GUI::apply_theme(GUI::Theme::Classic);}
                if (ImGui::MenuItem("Modern Dark")) {GUI::apply_theme(GUI::Theme::ModernDark);}
                if (ImGui::MenuItem("Modern Light")) {GUI::apply_theme(GUI::Theme::ModernLight);}

                ImGui::EndMenu(); //!Theme Settings
            }

            // ImGui::SeparatorText("Picking");
            // ImGui::Checkbox("Vertices", &picking_config_.enable_vertex_picking_);
            // ImGui::SameLine();
            // ImGui::Checkbox("Edges", &picking_config_.enable_edge_picking_);
            // ImGui::SameLine();
            // ImGui::Checkbox("Faces", &picking_config_.enable_face_picking_);
            // ImGui::SameLine();
            // ImGui::Checkbox("Cells", &picking_config_.enable_cell_picking_);
            // ImGui::SameLine();

            ImGui::EndMenu(); // !Settings
        }

#if 1
        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::BeginMenu("Visualization")) {
                ImGui::Checkbox("Show Picking Texture", &debug.display_picking_texture);
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Add Mesh with Inf and NaN Properties")) {
                scene().add_object<OpenVolumeMeshObject>(mesh_with_nan_and_inf_props());
            }
            if (ImGui::MenuItem("Add empty Mesh")) {
                scene().add_object<OpenVolumeMeshObject>(OVMVolumeMesh());
            }

            if (ImGui::MenuItem("Screenshot")) {
                debug.show_screenshot_window_ = true;
            }

            ImGui::EndMenu(); // !Debug
        }
#endif
        ImGui::EndMainMenuBar();
    }

    // Load Mesh File Dialog
    if (ImGuiFileDialog::Instance()->Display("LoadMeshDialogKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::filesystem::path filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            scene().add_mesh(filepath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // Screenshot Popup
    if (debug.show_screenshot_window_) {
        ImGui::SetNextWindowSize(
            ImVec2(480, 320),
            ImGuiCond_FirstUseEver
            );
        if (ImGui::Begin(
                "Screenshot",
                &debug.show_screenshot_window_))
        {
            InputSlider2("Size",
                         &debug.screenshot_size_.width,
                         InputSliderOptions{
                            .step=1,.min=1,.max=8000,
                            .display_name=true});
            if (ImGui::Button("Keep Aspect Ratio")) {
                if (debug.screenshot_size_.height > debug.screenshot_size_.width) {
                    debug.screenshot_size_.width = debug.screenshot_size_.height * viewport().size().aspect_ratio();
                } else {
                    debug.screenshot_size_.height = debug.screenshot_size_.width / viewport().size().aspect_ratio();
                }
            }

            const float preview_width = ImGui::GetItemRectSize().x;
            const float preview_height = preview_width / debug.screenshot_size_.aspect_ratio();

            draw_texture(viewport().color_view(), preview_width, preview_height);

            ImGui::InputText("File", debug.screenshot_filename_.data(), debug.screenshot_filename_.size());

            if (debug.screenshot_size_.width <= 8000
                && debug.screenshot_size_.height <= 8000)
            {
                if (ImGui::Button("Save")) {
                    save_viewport_screenshot(
                        debug.screenshot_filename_,
                        debug.screenshot_size_);
                }
            } else {
                ImGui::Text("Size too big! (Max. 8000 x 8000)");
            }
        }
        ImGui::End(); //!Screenshot
    }

    ImGui::End(); //!MenuBar
}

void Application::render_scenes_tab_bar_ui()
{
    ImGui::Begin("Scenes");

    if (ImGui::BeginTabBar("Scenes")) {
        for (uint32_t i = 0; i < scenes_.size(); ++i) {
            ImGui::PushID(i);

            std::string name = "Scene " + std::to_string(i);

            if (ImGui::BeginTabItem(name.c_str())) {
                current_scene_index_ = i;
                ImGui::EndTabItem();
            }

            ImGui::PopID();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Application::render_inspector_ui()
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

void Application::render_editor_ui()
{
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    render_main_menu_bar_ui();

    ImGuiID dockspace_id = ImHashStr("AxoPlotlDockSpace");

    ImGui::DockSpaceOverViewport(
        dockspace_id,
        ImGui::GetMainViewport()
        );

    // Rebuild the layout when requested.
    if (requests_.reset_layout) {
        build_default_layout(dockspace_id);
        requests_.reset_layout = false;
    }

    render_scenes_tab_bar_ui();

    render_viewport_ui();

    render_inspector_ui();

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
    const ViewSize _size)
{
    if (_size.width > 8000 || _size.height > 8000) {
        std::cerr << "Screenshot Size too big! (Max. 8000 x 8000)";
        return false;
    }
    wgpu::CommandEncoder encoder = gpu_context_.device.createCommandEncoder();
    Viewport newViewport(this);
    newViewport.resize(gpu_context_.device, render_context_.surface_format, _size.width, _size.height);
    Scene::Settings sceneSettings = scene().settings;
    scene().settings.show_orbit_target_cross = false;
    render_scene(encoder, newViewport);
    scene().settings = sceneSettings;
    wgpu::CommandBuffer cb = encoder.finish();
    gpu_context_.device.getQueue().submit(1, &cb);
    cb.release();
    encoder.release();
    return save_texture_to_file(_path, gpu_context_.device,
        newViewport.color_texture(), _size.width, _size.height);
}

}
