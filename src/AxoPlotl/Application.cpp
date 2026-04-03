#include "Application.hpp"
#include "AxoPlotl/debug/meshes.hpp"
#include "AxoPlotl/gui/fonts.hpp"
#include "AxoPlotl/gui/themes.hpp"
#include "AxoPlotl/input/Mouse.hpp"
#include "AxoPlotl/rendering/detail/redraw.hpp"
#include "AxoPlotl/utils/fps.hpp"
#include "ImGuiFileDialog.h"
#include "glm/ext/matrix_projection.hpp"
#include <cassert>
#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include <mach/task_info.h>
#include <mach/mach.h>
#include <AxoPlotl/rendering/detail/wgpu_commons.hpp>
#include <AxoPlotl/tools/DataControlTool.hpp>

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
    _device.poll(false);
#elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
    if (_yield_to_browser) {
        emscripten_sleep(100);
    }
#endif
}

namespace AxoPlotl
{

Application::Application() :
    user_ui_callback_([](Application* _app) {}),
    error_callback_(nullptr)
{
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
        auto app_ptr = reinterpret_cast<AxoPlotl::Application*>(glfwGetWindowUserPointer(window));
        if (app_ptr) {
            app_ptr->on_window_resize(width, height);
        }
    });

    glfwSetCursorPosCallback(window_, AxoPlotl::Input::mouse_callback);
    glfwSetScrollCallback(window_, AxoPlotl::Input::scroll_callback);
    glfwSetMouseButtonCallback(window_, [](GLFWwindow*,int,int,int) {trigger_redraw();});
    glfwSetKeyCallback(window_, [](GLFWwindow*,int,int,int,int) {trigger_redraw();});

    // Drop Callback -> Drag in Files to load them
    glfwSetDropCallback(window_, [](GLFWwindow* window, int count, const char** paths) {

        auto app_ptr = static_cast<AxoPlotl::Application*>(glfwGetWindowUserPointer(window));
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

    std::cout << "Requesting adapter..." << std::endl;
    //surface_ = glfwGetWGPUSurface(instance, _window);
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.compatibleSurface = surface_;
    adapter_ = instance.requestAdapter(adapterOpts);
    std::cout << "Got adapter: " << adapter_ << std::endl;

    instance.release();

    //------------------
    // Device
    //------------------

    std::cout << "Requesting device..." << std::endl;
    wgpu::DeviceDescriptor deviceDesc = {};
    deviceDesc.label = "My Device";
    deviceDesc.requiredFeatureCount = 0;
    deviceDesc.requiredLimits = nullptr;
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = "The default queue";
    deviceDesc.deviceLostCallbackInfo.callback = [](
        const WGPUDevice* _device,
        WGPUDeviceLostReason _reason,
        const char* _message, void* _userdata)
    {
        std::cerr << "Device lost: " << _reason;
        if (_message) {std::cerr << " (" << _message << ")";}
        std::cerr << std::endl;
    };

    device_ = adapter_.requestDevice(deviceDesc);
    std::cout << "Got device: " << device_ << std::endl;
    //device.getLimits(&supportedLimits);
    //std::cout << "device.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;

    error_callback_ = device_.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
        std::cerr << "Uncaptured device error: type " << type;
        if (message) std::cout << " (" << message << ")";
        std::cerr << std::endl;
        std::exit(1);
    });

    queue_ = device_.getQueue();


    //----------
    // Surface
    //----------
    configure_surface();

    //-------------------
    // Special Textures
    //-------------------
    create_depth_texture();
    create_picking_texture();

    wgpu::BufferDescriptor pickBuffDesc{};
    pickBuffDesc.label = "Pixel Picking Buffer";
    pickBuffDesc.size = sizeof(PickResult);
    pickBuffDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    pickBuffDesc.mappedAtCreation = false;
    picking_buffer_ = device_.createBuffer(pickBuffDesc);

    //----------
    // Scene
    //----------
    scene_.init(this);

    //----------
    // Gui
    //----------
    if (!init_imgui()) {
        return false;
    }

    return true;
}

void Application::frame_tick()
{
    // Input and Time Update
    Time::update();
    Input::Mouse::update(window_);
    glfwPollEvents();

    // Only render a certain number of frames
    // before redrawing
    if (!on_draw()) [[likely]] {return;}

    wgpu::CommandEncoder cmd_encoder = device_.createCommandEncoder();

    // Acquire next frame texture
    wgpu::SurfaceTexture surfaceTexture;
    surface_.getCurrentTexture(&surfaceTexture);

    // Safety Check
    if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) [[unlikely]] {
#ifndef WEBGPU_BACKEND_WGPU
        if (surfaceTexture.texture) {
            wgpuTextureRelease(surfaceTexture.texture);
        }
#endif //! WEBGPU_BACKEND_WGPU
        return;
    }

    wgpu::TextureViewDescriptor viewDescriptor;
    viewDescriptor.nextInChain = nullptr;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = wgpuTextureGetFormat(surfaceTexture.texture);
    viewDescriptor.dimension = WGPUTextureViewDimension_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = WGPUTextureAspect_All;
    wgpu::TextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

    wgpu::RenderPassColorAttachment color_attachments[2];

    // Color Attachment
    color_attachments[0].view = targetView;
    color_attachments[0].loadOp = wgpu::LoadOp::Clear;
    color_attachments[0].storeOp = wgpu::StoreOp::Store;
    color_attachments[0].clearValue = {clear_color_[0],clear_color_[1],clear_color_[2],1.0f};
    color_attachments[0].resolveTarget = nullptr;
    color_attachments[0].nextInChain = nullptr;
#ifndef WEBGPU_BACKEND_WGPU
    color_attachments[0].depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif //!WEBGPU_BACKEND_WGPU

    // Picking Attachment
    color_attachments[1].view = picking_view_;
    color_attachments[1].loadOp = wgpu::LoadOp::Clear;
    color_attachments[1].storeOp = wgpu::StoreOp::Store;
    color_attachments[1].clearValue = {0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
    color_attachments[1].resolveTarget = nullptr;
    color_attachments[1].nextInChain = nullptr;
#ifndef WEBGPU_BACKEND_WGPU
    color_attachments[1].depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif //!WEBGPU_BACKEND_WGPU

    wgpu::RenderPassDescriptor renderPassDesc{};
    renderPassDesc.colorAttachmentCount = 2;
    renderPassDesc.colorAttachments = color_attachments;
    renderPassDesc.label = "Main Render Pass";

    // Depth Attachment
    wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.view = depth_texture_view_;
    depthStencilAttachment.depthClearValue = 1.0f; // "far"
    depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
    depthStencilAttachment.depthReadOnly = false; // could turn off writing to depth buffer
    depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
#endif
#ifdef WEBGPU_BACKEND_DAWN
    depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
    depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
    //depthStencilAttachment.depthClearValue = std::numeric_limits<float>::quiet_NaN();
#endif
    depthStencilAttachment.stencilReadOnly = true;
    renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

    // Begin render pass
    scene_render_pass_ =
        cmd_encoder.beginRenderPass(renderPassDesc);

    // Set the Scene Viewport
    auto viewport = scene_viewport();
    scene_render_pass_.setViewport(viewport[0], viewport[1], viewport[2], viewport[3], 0.0f, 1.0f);
    scene_render_pass_.setScissorRect(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Render the Scene with its objects
    scene_.render(scene_render_pass_);

    // For the picking to work correctly, we resize the
    // scene viewprt to the full rectangle after rendering
    // viewport = total_viewport();
    // scene_render_pass_.setViewport(viewport[0], viewport[1], viewport[2], viewport[3], 0.0f, 1.0f);
    // scene_render_pass_.setScissorRect(viewport[0], viewport[1], viewport[2], viewport[3]);

    scene_render_pass_.end();

    //------------------
    // Click to Pick
    //------------------
    bool just_clicked_on_object(false);
    if (Input::Mouse::RIGHT_JUST_PRESSED) {
        int width, height;
        glfwGetWindowSize(window(), &width, &height);
        float x_scale = (float)total_viewport()[2] / (float)width;
        float y_scale = (float)total_viewport()[3] / (float)height;
        float x = x_scale*Input::Mouse::POSITION[0] - total_viewport()[0];
        float y = y_scale*Input::Mouse::POSITION[1] - total_viewport()[1];
        pick_result_ = request_pick_result(x, y);
        //std::cerr << pick_result_ << std::endl;
        if (pick_result_.object_id_ < UINT32_MAX) {
            just_clicked_on_object = true;
        }
    }

    //------------------
    // GUI Render Pass
    //------------------
    // only expects pixel color attachment (no picking texture)
    // wgpu::RenderPassColorAttachment guiColorAttachment{};
    // guiColorAttachment.view = targetView;
    color_attachments[0].loadOp = wgpu::LoadOp::Load; // load scene result
    wgpu::RenderPassDescriptor guiPassDesc{};
    guiPassDesc.colorAttachmentCount = 1;
    guiPassDesc.colorAttachments = &color_attachments[0];
    guiPassDesc.depthStencilAttachment = &depthStencilAttachment;
    gui_render_pass_ = cmd_encoder.beginRenderPass(guiPassDesc);
    render_imgui(gui_render_pass_, just_clicked_on_object);

    gui_render_pass_.end();

    // Submit
    wgpu::CommandBuffer cmdBuffer = cmd_encoder.finish();
    queue_.submit(1, &cmdBuffer);

#ifndef __EMSCRIPTEN__
    surface_.present();
#endif

// Cleanup
#ifndef WEBGPU_BACKEND_WGPU
    wgpuTextureRelease(surfaceTexture.texture);
#endif //! WEBGPU_BACKEND_WGPU
    targetView.release();
    scene_render_pass_.release();
    gui_render_pass_.release();
    cmdBuffer.release();
    cmd_encoder.release();

    wgpuPollEvents(device_, false);

    // Functions to execute after the command buffer submit
    // For example deletion of scene objects.
    for (auto& fn : deferred_calls_) {fn();}
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
    create_depth_texture();
    create_picking_texture();

    wgpuPollEvents(device_, true);
}

void Application::clear()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();

    scene().clear();

    if (queue_) {
        queue_.release();
        queue_ = nullptr;
    }
    if (surface_) {
        surface_.unconfigure();
        surface_.release();
        surface_ = nullptr;
    }
    if (depth_texture_view_) {
        depth_texture_view_.release();
        depth_texture_view_ = nullptr;
    }
    if (depth_texture_) {
        depth_texture_.destroy();
        depth_texture_.release();
        depth_texture_ = nullptr;
    }

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


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window_, true);
    wgpu::SurfaceCapabilities surf_caps{};
    surface_.getCapabilities(adapter_, &surf_caps);

    // new imgui init
    ImGui_ImplWGPU_InitInfo imgui_init_info;
    imgui_init_info.Device = device_;
    imgui_init_info.NumFramesInFlight = 3;
    imgui_init_info.RenderTargetFormat = surf_caps.formats[0];
    imgui_init_info.DepthStencilFormat = depth_texture_format_;
    imgui_init_info.PipelineMultisampleState.count = 1;
    ImGui_ImplWGPU_Init(&imgui_init_info);

    // This is with the older imgui version from Learn WebGPU
    //ImGui_ImplWGPU_Init(device_, 3, surf_caps.formats[0], depthTextureFormat);

    // Font
    GUI::load_fonts();

    return true;
}

glm::vec<4,float> Application::scene_viewport()
{
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
    float sidebar_width = sidebar_rel_width_ * fbWidth;
    if (sidebar_right_aligned_) {
        return {0.0f, 0.0f, fbWidth - sidebar_width, fbHeight};
    } else {
        return {sidebar_width, 0.0f, fbWidth - sidebar_width, fbHeight};
    }
}

glm::vec<4,float> Application::total_viewport()
{
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
    return {0.0f, 0.0f, fbWidth, fbHeight};
}

void Application::render_imgui(wgpu::RenderPassEncoder _render_pass, bool _just_clicked_on_object)
{
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Set Viewport
    ImGuiViewport* vp = ImGui::GetMainViewport();
    float sidebar_width = sidebar_rel_width_ * vp->WorkSize.x;
    vp->WorkPos.x = sidebar_right_aligned_?
        vp->WorkSize.x - sidebar_width : 0.0f;
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(sidebar_width, vp->WorkSize.y));

    // Lock the window so it can't be moved or closed
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Inspector", nullptr, flags);
    ImGui::SetWindowFontScale(font_scale_);

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
                    "LoadDialogKey", "Choose File",
                    "Mesh files (*.obj *.ovm *.ovmb *ply){.obj,.ovm,.ovmb,.ply}",
                    config);
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                //TODO
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings"))
        {
            ImGui::SeparatorText("UI");
            ImGui::Checkbox("Right", &sidebar_right_aligned_);
            ImGui::SliderFloat("Width", &sidebar_rel_width_, 0.1f, 0.9f);
            if (ImGui::BeginMenu("Theme")) {
                if (ImGui::MenuItem("Dark")) {GUI::apply_theme(GUI::Theme::Dark);}
                if (ImGui::MenuItem("Light")) {GUI::apply_theme(GUI::Theme::Light);}
                if (ImGui::MenuItem("Classic")) {GUI::apply_theme(GUI::Theme::Classic);}
                if (ImGui::MenuItem("Modern Dark")) {GUI::apply_theme(GUI::Theme::ModernDark);}
                if (ImGui::MenuItem("Modern Light")) {GUI::apply_theme(GUI::Theme::ModernLight);}
                ImGui::EndMenu(); //!Theme
            }
            ImGui::SliderFloat("Font Scale", &font_scale_, 0.2f, 2.0f);

            ImGui::SeparatorText("Scene");
            ImGui::ColorEdit3("Background", clear_color_);
            ImGui::Checkbox("Axis Cross", &scene_.axis_cross_enabled());

            ImGui::SeparatorText("Picking");
            ImGui::Checkbox("Vertices", &picking_config_.enable_vertex_picking_);
            ImGui::SameLine();
            ImGui::Checkbox("Edges", &picking_config_.enable_edge_picking_);
            ImGui::SameLine();
            ImGui::Checkbox("Faces", &picking_config_.enable_face_picking_);
            ImGui::SameLine();
            ImGui::Checkbox("Cells", &picking_config_.enable_cell_picking_);
            ImGui::SameLine();

            ImGui::EndMenu(); // !Settings
        }

#if 1
        if (ImGui::BeginMenu("Debug"))
        {
            if (ImGui::MenuItem("Add Mesh with Inf and NaN Properties")) {
                scene_.add_object<OpenVolumeMeshObject>(mesh_with_nan_and_inf_props());
            }
            if (ImGui::MenuItem("Add empty Mesh")) {
                scene_.add_object<OpenVolumeMeshObject>(OVMVolumeMesh());
            }

            ImGui::EndMenu(); // !Debug
        }
#endif

        ImGui::EndMainMenuBar();
    }

    // Load Mesh File Dialog
    if (ImGuiFileDialog::Instance()->Display("LoadDialogKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
            std::filesystem::path filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            scene().add_mesh(filepath);
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // Clicking on an Object
    if (_just_clicked_on_object) [[unlikely]] {ImGui::OpenPopup("PickingPopup");}
    if (pick_result_.object_id_ < UINT32_MAX && ImGui::BeginPopup("PickingPopup")) {
        auto obj = scene().get_object(pick_result_.object_id_);
        if (obj) [[likely]] {obj->render_ui_picking(pick_result_, picking_config_);}
        ImGui::EndPopup();
    }

    user_ui_callback_(this);

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), _render_pass);
}

PickResult Application::request_pick_result(float _x, float _y)
{
    wgpu::CommandEncoder encoder = device_.createCommandEncoder();

    // We copy FROM the picking texture
    wgpu::ImageCopyTexture src{};
    src.texture = picking_texture_;
    src.origin = {static_cast<uint32_t>(_x), static_cast<uint32_t>(_y), 0};
    src.mipLevel = 0;
    src.aspect = wgpu::TextureAspect::All;

    // We copy TO the picking buffer
    wgpu::ImageCopyBuffer dst{};
    dst.buffer = picking_buffer_;
    dst.layout.offset = 0;
    dst.layout.bytesPerRow = 256; // needs to be multiple of 256
    dst.layout.rowsPerImage = 1;

    // Copy the single pixel we clicked on (1 pixel is 1x1x1)
    wgpu::Extent3D copySize = {1,1,1};
    encoder.copyTextureToBuffer(src, dst, copySize);

    wgpu::CommandBuffer cb = encoder.finish();
    queue_.submit(1, &cb);
    cb.release();
    encoder.release();

    struct Context {
        bool ready;
        wgpu::Buffer buffer;
        PickResult pick;
        Vec4f ndc;
        Scene* scene;
        Vec4f scene_viewport;
        float aspect_ratio;
    };

    auto on_buffer_mapped = [](
        WGPUBufferMapAsyncStatus status,
        void* _user_data)
    {
        Context* context = reinterpret_cast<Context*>(_user_data);
        context->ready = true;
        //std::cout << "Buffer mapped with status " << status << std::endl;
        if (status != wgpu::BufferMapAsyncStatus::Success) {return;}

        // Extract Info from Picking Result
        // and Map clicked position back to world space
        uint32_t* data = (uint32_t*)context->buffer.getConstMappedRange(0, sizeof(PickResult));
        context->pick.object_id_ = data[0];
        context->pick.type_ = data[1];
        context->pick.index_ = data[2];

        float depth = std::bit_cast<float>(data[3]);
        context->ndc.z = depth;
        context->ndc.w = 1;
        const auto& v = context->scene->perspective().getViewMatrix();
        const auto& p = context->scene->perspective().getProjectionMatrix(context->aspect_ratio);
        Vec4f pos = glm::inverse(p * v) * context->ndc;
        pos /= pos.w;
        context->pick.position[0] = pos[0];
        context->pick.position[1] = pos[1];
        context->pick.position[2] = pos[2];

        // unmap the memory
        context->buffer.unmap();
    };

    // Create the Context instance
    Context context = {false, picking_buffer_};
    context.scene = &scene_;
    context.aspect_ratio = scene_viewport()[2]/scene_viewport()[3];

    // Get x and y components of Noramlzied Device Coordinates
    _x -= scene_viewport()[0];
    _y -= scene_viewport()[1];
    context.ndc = {
        (2.0*_x)/scene_viewport()[2] - 1.0,
        1.0-(2.0*_y)/scene_viewport()[3],
        0, 1
    };

    wgpuBufferMapAsync(picking_buffer_, wgpu::MapMode::Read, 0, sizeof(PickResult), on_buffer_mapped, (void*)&context);
    while (!context.ready) {
        wgpuPollEvents(device_, true);
    }
    return context.pick;
}

void Application::configure_surface()
{
    auto viewport = total_viewport();
    //std::cerr << viewport[2] << "/" << viewport[3] << std::endl;

    // Configure the surface
    wgpu::SurfaceConfiguration config = {};

    // Configuration of the textures created for the underlying swap chain
    config.width = static_cast<uint32_t>(viewport[2]);
    config.height = static_cast<uint32_t>(viewport[3]);
    config.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::SurfaceCapabilities capabilities = {};
    surface_.getCapabilities(adapter_, &capabilities);
    config.format = capabilities.formats[0];

    // And we do not need any particular view format:
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.device = device_;
    config.presentMode = wgpu::PresentMode::Fifo;
    config.alphaMode = capabilities.alphaModes[0];

    surface_.unconfigure();
    surface_.configure(config);
}

void Application::create_depth_texture()
{
    if (depth_texture_view_) {
        depth_texture_view_.release();
    }
    if (depth_texture_) {
        depth_texture_.destroy();
        depth_texture_.release();
    }

    // int fbWidth, fbHeight;
    // glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
    // std::cout << "Framebuffer size: " << fbWidth << " x " << fbHeight << std::endl;
    auto viewport = total_viewport();

    wgpu::DepthStencilState depthStencilState = create_default_depth_state();
    depth_texture_format_ = depthStencilState.format;

    // Create the depth texture
    wgpu::TextureDescriptor depthTextureDesc;
    depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
    depthTextureDesc.format = depth_texture_format_;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size = {
        static_cast<uint32_t>(viewport[2]),
        static_cast<uint32_t>(viewport[3]),
        1};
    depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    depthTextureDesc.viewFormatCount = 1;
    depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depth_texture_format_;
    depth_texture_ = device_.createTexture(depthTextureDesc);

    // Create the view of the depth texture manipulated by the rasterizer
    wgpu::TextureViewDescriptor depthTextureViewDesc;
    depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
    depthTextureViewDesc.baseArrayLayer = 0;
    depthTextureViewDesc.arrayLayerCount = 1;
    depthTextureViewDesc.baseMipLevel = 0;
    depthTextureViewDesc.mipLevelCount = 1;
    depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    depthTextureViewDesc.format = depth_texture_format_;
    depth_texture_view_ = depth_texture_.createView(depthTextureViewDesc);

    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;
}

void Application::create_picking_texture()
{
    if (picking_view_) {picking_view_.release();}
    if (picking_texture_) {
        picking_texture_.destroy();
        picking_texture_.release();
    }
    auto viewport = total_viewport();

    wgpu::TextureDescriptor pickDesc{};
    pickDesc.label = "Picking Texture";
    pickDesc.dimension = wgpu::TextureDimension::_2D;
    pickDesc.size = {
        static_cast<uint32_t>(viewport[2]),
        static_cast<uint32_t>(viewport[3]),
        1
    };
    pickDesc.format = wgpu::TextureFormat::RGBA32Uint;
    pickDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    pickDesc.sampleCount = 1;
    pickDesc.mipLevelCount = 1;

    picking_texture_ = device_.createTexture(pickDesc);
    picking_view_ = picking_texture_.createView();
}

}
