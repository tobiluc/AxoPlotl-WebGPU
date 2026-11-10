#define IMGUI_DEFINE_MATH_OPERATORS

#include "Application.hpp"
#include "AxoPlotl/debug/meshes.hpp"
#include "AxoPlotl/gui/fonts.hpp"
#include "AxoPlotl/gui/themes.hpp"
#include "AxoPlotl/input/Mouse.hpp"
#include "AxoPlotl/objects/SegmentObject.hpp"
#include "AxoPlotl/objects/SphericalHarmonicsObject.hpp"
#include "AxoPlotl/objects/TetrahedronObject.hpp"
#include "AxoPlotl/rendering/detail/redraw.hpp"
#include "ImGuiFileDialog.h"
#include <cassert>
#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include <mach/task_info.h>
#include <mach/mach.h>
#include <AxoPlotl/rendering/detail/wgpu_commons.hpp>
#include <AxoPlotl/tools/DataControlTool.hpp>
#include <OpenVolumeMesh/IO/ovmb_write.hh>
#include <imgui_internal.h>
#include "AxoPlotl/rendering/save_texture_to_file.hpp"
#include "AxoPlotl/rendering/draw_texture.hpp"

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
    current_scene_index_(0)
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
    // Instance
    //------------------
    wgpu::Instance instance = wgpuCreateInstance(nullptr);

    surface_ = glfwGetWGPUSurface(instance, window_);

    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.compatibleSurface = surface_;
    adapter_ = instance.requestAdapter(adapterOpts);

    instance.release();

    //------------------
    // Device
    //------------------

    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = wgpu::StringView("My Device");
    deviceDesc.requiredFeatureCount = 0;
    deviceDesc.requiredLimits = nullptr;
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = wgpu::StringView("The default queue");

    deviceDesc.deviceLostCallbackInfo.callback = [](
         WGPUDevice const* _device,
         WGPUDeviceLostReason _reason,
         WGPUStringView _message,
         void* _userdata1, void* _userdata2)
    {
        std::cerr << "Device lost: " << static_cast<uint32_t>(_reason);

        if (_message.data) {
            std::cerr << " (" << std::string_view(_message.data, _message.length) << ")";
        }
        std::cerr << std::endl;
    };
    deviceDesc.uncapturedErrorCallbackInfo.callback = [](
         WGPUDevice const* _device,
         WGPUErrorType _type,
         WGPUStringView _message,
         void* _userdata1, void* _userdata2)
    {
        std::cerr << "WebGPU Error (" << static_cast<uint32_t>(_type) << "): "
                  << _message.data << std::endl;
        std::exit(1);
    };
    deviceDesc.uncapturedErrorCallbackInfo.userdata1 = nullptr;
    deviceDesc.uncapturedErrorCallbackInfo.userdata2 = nullptr;

    device_ = adapter_.requestDevice(deviceDesc);

    //----------
    // Surface
    //----------
    configure_surface();

    //-------------------
    // Special Textures
    //-------------------
    wgpu::DepthStencilState depthStencilState = create_default_depth_state();
    depth_texture_format_ = depthStencilState.format;
    viewport_.resize(device_, framebuffer_size().x, framebuffer_size().y, surface_format_, depth_texture_format_);

    wgpu::BufferDescriptor pickBuffDesc{};
    pickBuffDesc.label = wgpu::StringView("Pixel Picking Buffer");
    pickBuffDesc.size = sizeof(PickResult);
    pickBuffDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    pickBuffDesc.mappedAtCreation = false;
    picking_buffer_ = device_.createBuffer(pickBuffDesc);

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
    debug.picking_renderer_.init(device_, 256, 256);

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
    surface_.getCurrentTexture(&surface_texture);

    if (surface_texture.status !=
        wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal) {
        return;
    }

    // Build the ImGui interface FIRST.
    render_editor_ui();

    // Create surface view.
    wgpu::TextureViewDescriptor view_desc{};
    view_desc.format = surface_format_;
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
    auto encoder = device_.createCommandEncoder();

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
                pick_result_ = request_pick_result(pos->x, pos->y);
                if (pick_result_.object_id_ < UINT32_MAX) {
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
    device_.getQueue().submit(1, &cmd_buffer);

#ifndef __EMSCRIPTEN__
    surface_.present();
#endif

    // Release resources as in your original implementation.
    target_view.release();
    cmd_buffer.release();
    encoder.release();

#ifndef WEBGPU_BACKEND_WGPU
    wgpuTextureRelease(surface_texture.texture);
#endif

    wgpuPollEvents(device_, false);

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

    configure_surface();
    viewport_.resize(device_, width, height, surface_format_, depth_texture_format_);
    debug.picking_renderer_.resize(viewport_.width(), viewport_.height());

    wgpuPollEvents(device_, true);
}

void Application::clear()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();

    for (auto& scene : scenes_) {
        scene.destroy();
    }

    auto queue = device_.getQueue();
    if (queue) {
        queue.release();
        queue = nullptr;
    }
    if (surface_) {
        surface_.unconfigure();
        surface_.release();
        surface_ = nullptr;
    }

    viewport_.release();

    if (adapter_) {
        adapter_.release();
        adapter_ = nullptr;
    }
    if (device_) {
        device_.release();
        device_ = nullptr;
    }

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
    wgpu::SurfaceCapabilities surf_caps{};
    surface_.getCapabilities(adapter_, &surf_caps);

    // new imgui init
    ImGui_ImplWGPU_InitInfo imgui_init_info{};
    imgui_init_info.Device = device_;
    imgui_init_info.NumFramesInFlight = 3;
    imgui_init_info.RenderTargetFormat = surface_format_;
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

void Application::render_viewport_ui() {
    viewport_.render_ui(
        device_,
        surface_format_,
        depth_texture_format_
        );

    // Clicking on an Object
    if (requests_.pick_result) [[unlikely]] {
        ImGui::OpenPopup("PickingPopup");
        requests_.pick_result = false;
    }
    if (pick_result_.object_id_ < UINT32_MAX && ImGui::BeginPopup("PickingPopup")) {
        auto obj = scene().get_object(pick_result_.object_id_);
        if (obj) [[likely]] {obj->render_ui_picking(pick_result_, picking_config_);}
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

            if (ImGui::BeginMenu("Screenshot")) {
                InputSlider2("SaveScreenshot",
                             &debug.screenshot_width_,
                             InputSliderOptions{
                                .step=1,.min=1,.max=4000,
                                .display_name=false});
                ImGui::InputText("File", debug.screenshot_filename_.data(), debug.screenshot_filename_.size());
                if (ImGui::Button("Save")) {
                    save_viewport_screenshot(
                        debug.screenshot_filename_,
                        ViewSize{.width=debug.screenshot_width_,
                                 .height=debug.screenshot_height_});
                }
                ImGui::EndMenu();
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

PickResult Application::request_pick_result(float _x, float _y)
{
    wgpu::CommandEncoder encoder = device_.createCommandEncoder();

    // We copy FROM the picking texture
    wgpu::TexelCopyTextureInfo src{};
    src.texture = viewport_.picking_texture();// picking_texture_;
    src.origin = {static_cast<uint32_t>(_x), static_cast<uint32_t>(_y), 0};
    src.mipLevel = 0;
    src.aspect = wgpu::TextureAspect::All;

    // We copy TO the picking buffer
    wgpu::TexelCopyBufferInfo dst{};
    dst.buffer = picking_buffer_;
    dst.layout.offset = 0;
    // this needs to be a multiple of 256
    // even if we care only about a 1x1x1x1 pixel
    dst.layout.bytesPerRow = 256;
    dst.layout.rowsPerImage = 1;

    // Copy the single pixel we clicked on (1 pixel is 1x1x1)
    wgpu::Extent3D copySize = {1,1,1};
    encoder.copyTextureToBuffer(src, dst, copySize);

    wgpu::CommandBuffer cb = encoder.finish();
    device_.getQueue().submit(1, &cb);
    cb.release();
    encoder.release();

    struct Context {
        bool ready;
        wgpu::Buffer buffer;
        PickResult pick;
        Scene* scene;
        ViewSize scene_size;
    };

    auto on_buffer_mapped = [](
        WGPUMapAsyncStatus status,
        WGPUStringView message,
        void* _user_data, void*)
    {
        Context* context = reinterpret_cast<Context*>(_user_data);
        context->ready = true;
        //std::cout << "Buffer mapped with status " << status << std::endl;
        if (status != WGPUMapAsyncStatus::WGPUMapAsyncStatus_Success) {return;}

        // Extract Info from Picking Result
        // and Map clicked position back to world space

        uint32_t* data = (uint32_t*)context->buffer.getConstMappedRange(0, sizeof(PickResult));

        // Index 0 stores the Object ID, so we know which
        // Objects we clicked. The corresponding object's
        // render_picking_ui function is called.
        context->pick.object_id_ = data[0];

        // For Meshes, we store the entity type and handle index
        // (Type 0:Vertex, 1:Edge, 2:Face, 3:Cell)
        context->pick.entity_type_ = data[1];
        context->pick.entity_index_ = data[2];

        // Entity Index (u32, for Meshes) and Function Value (f32, for Functions)
        // are both stored at index 2 of the pixel
        // TODO: Maybe we want different PickResult Objects instead?
        context->pick.function_value_ = std::bit_cast<float32_t>(data[2]);

        // At Index 3 we store the fragment's depth value
        // Based on this, we can reconstruct the 3d position
        // in world space
        // (normalized devie coords (ndc) x and y value, we already
        // have, it's the normalized viewport position)
        float depth = std::bit_cast<float>(data[3]);
        context->pick.ndc_.z = depth;
        const auto& v = context->scene->camera().get_view_matrix();
        const auto& p = context->scene->camera().get_projection_matrix(context->scene_size);
        Vec4f pos = glm::inverse(p * v) * Vec4f(context->pick.ndc_, 1);
        pos /= pos.w;
        context->pick.position_[0] = pos[0];
        context->pick.position_[1] = pos[1];
        context->pick.position_[2] = pos[2];

        // unmap the memory
        context->buffer.unmap();
    };

    // The context contains everyhting that is required
    // for our picking computation in on_buffer_mapped.
    // Is it passed as userdata1
    Context context;
    context.ready = false;
    context.buffer = picking_buffer_;
    context.scene = &scene();
    context.scene_size = viewport().size();

    // Get x and y components of Normalized Device Coordinates
    _x -= 0; // topleft of vieport is origin since docking rework
    _y -= 0;
    context.pick.ndc_ = {
        std::clamp((2.0*_x)/context.scene_size.width - 1.0, -1.0, 1.0),
        std::clamp(1.0-(2.0*_y)/context.scene_size.height, -1.0, 1.0),
        0
    };

    WGPUBufferMapCallbackInfo callback_info;
    callback_info.callback = on_buffer_mapped;
    callback_info.mode = WGPUCallbackMode_AllowSpontaneous;
    callback_info.userdata1 = (void*)&context;
    callback_info.userdata2 = nullptr;
    callback_info.nextInChain = nullptr;

    wgpuBufferMapAsync(
        picking_buffer_,
        wgpu::MapMode::Read,
        0, sizeof(PickResult),
        callback_info
    );
    while (!context.ready) {
        wgpuPollEvents(device_, true);
    }

    return context.pick;

}

void Application::configure_surface()
{
    auto fb_size = framebuffer_size();
    //std::cerr << viewport[2] << "/" << viewport[3] << std::endl;

    // Configure the surface
    wgpu::SurfaceConfiguration config = {};

    // Configuration of the textures created for the underlying swap chain
    config.width = static_cast<uint32_t>(fb_size.x);
    config.height = static_cast<uint32_t>(fb_size.y);
    config.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::SurfaceCapabilities capabilities = {};
    surface_.getCapabilities(adapter_, &capabilities);
    config.format = capabilities.formats[0];
    surface_format_ = config.format;

    // And we do not need any particular view format:
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.device = device_;
    config.presentMode = wgpu::PresentMode::Fifo;
    config.alphaMode = capabilities.alphaModes[0];

    surface_.unconfigure();
    surface_.configure(config);
}

bool Application::save_viewport_screenshot(
    const std::filesystem::path& _path, const ViewSize _size)
{
    wgpu::CommandEncoder encoder = device_.createCommandEncoder();
    Viewport newViewport;
    newViewport.resize(device_, _size.width, _size.height,
        surface_format_, depth_texture_format_);
    render_scene(encoder, newViewport);
    wgpu::CommandBuffer cb = encoder.finish();
    device_.getQueue().submit(1, &cb);
    cb.release();
    encoder.release();
    return save_texture_to_file(_path, device_,
        newViewport.color_texture(), _size.width, _size.height);
}

}
