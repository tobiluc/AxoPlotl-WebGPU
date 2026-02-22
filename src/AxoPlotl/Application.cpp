#include "Application.hpp"
#include "AxoPlotl/input/Mouse.hpp"
#include "AxoPlotl/rendering/detail/redraw.hpp"
#include "AxoPlotl/utils/fps.hpp"
#include "ImGuiFileDialog.h"
#include <cassert>
#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include <AxoPlotl/PluginRegistry.hpp>
#include <mach/task_info.h>
#include <mach/mach.h>
#include <AxoPlotl/rendering/detail/wgpu_commons.hpp>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__
#ifdef WEBGPU_BACKEND_WGPU
#  include <webgpu/wgpu.h>
#endif // WEBGPU_BACKEND_WGPU

namespace AxoPlotl
{

Application::Application()
{
}

bool Application::init()
{
    // Create Window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window_ = glfwCreateWindow(640, 480, "AxoPlotl - WebGPU", nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }
    assert(window_);

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
    wgpu::SurfaceCapabilities capabilities = {};
    surface_.getCapabilities(adapter_, &capabilities);
    color_format_ = capabilities.formats[0];
    configure_surface();

    //----------
    // Depth
    //----------
    create_depth_texture();

    //----------
    // Scene
    //----------
    scene_.init(this);

    //----------
    // Gui
    //----------
    if (!init_gui()) {
        return false;
    }

    //----------
    // Plugins
    //----------
    PluginRegistry::instantiate_all();

    return true;
}

void Application::run()
{
    // Input and Time Update
    Time::update();
    Input::Mouse::update(window_);
    glfwPollEvents();

    // Only render a certain number of frames
    // before redrawing
    if (!on_draw()) [[likely]] {
        return;
    }

    wgpu::CommandEncoder cmd_encoder = device_.createCommandEncoder();

    // Acquire next frame texture
    wgpu::SurfaceTexture surfaceTexture;
    surface_.getCurrentTexture(&surfaceTexture);
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

#ifndef WEBGPU_BACKEND_WGPU
        wgpuTextureRelease(surfaceTexture.texture);
#endif // WEBGPU_BACKEND_WGPU

    // Color Attachment
    wgpu::RenderPassColorAttachment colorAttachment{};
    colorAttachment.view = targetView;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {clear_color_[0],clear_color_[1],clear_color_[2],1.0f};
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.nextInChain = nullptr;
#ifndef WEBGPU_BACKEND_WGPU
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif //!WEBGPU_BACKEND_WGPU

    wgpu::RenderPassDescriptor renderPassDesc{};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.label = "Main Render Pass";

    // Depth Attachment
    wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
    depthStencilAttachment.view = depthTextureView;
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
    wgpu::RenderPassEncoder renderPass =
        cmd_encoder.beginRenderPass(renderPassDesc);

    // Set the Scene Viewport
    auto viewport = scene_viewport();
    renderPass.setViewport(viewport[0], viewport[1], viewport[2], viewport[3], 0.0f, 1.0f);
    renderPass.setScissorRect(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Render the Scene with its objects
    scene_.render(renderPass);

    // Reset Viewport to full screen before drawing ImGui
    // Otherwise, ImGui will be squashed and unclickable!
    viewport = total_viewport();
    renderPass.setViewport(viewport[0], viewport[1], viewport[2], viewport[3], 0.0f, 1.0f);
    renderPass.setScissorRect(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Render  Gui
    update_gui(renderPass);

    // End pass
    renderPass.end();

    // Submit
    wgpu::CommandBuffer cmdBuffer = cmd_encoder.finish();
    queue_.submit(1, &cmdBuffer);

    // Cleanup
    targetView.release();
    renderPass.release();
    cmdBuffer.release();
    cmd_encoder.release();
    //wgpuTextureRelease(surfaceTexture.texture);

#ifndef __EMSCRIPTEN__
    surface_.present();
#endif

#if defined(WEBGPU_BACKEND_DAWN)
    device_.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
    device.poll(false);
#endif

    // Functions to execute after the command buffer submit
    // For example deletion of scene objects.
    for (auto& fn : deferred_calls_) {fn();}
    deferred_calls_.clear();

}

void Application::on_window_resize(float width, float height)
{
    if (width == 0|| height == 0) {return;} // window minimized

    // Framebuffer size
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);

    //surface_.unconfigure();
    configure_surface();
    create_depth_texture();

    //pipeline.updateProjection(width/height);
}

void Application::terminate()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();

    if (queue_) {queue_.release();}
    if (surface_) {
        surface_.unconfigure();
        surface_.release();
    }
    if (adapter_) {adapter_.release();}
    if (depthTextureView) {depthTextureView.release();}
    if (depthTexture) {
        depthTexture.destroy();
        depthTexture.release();
    }
    if (device_) {device_.release();}

    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool Application::init_gui()
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
    imgui_init_info.DepthStencilFormat = depthTextureFormat;
    imgui_init_info.PipelineMultisampleState.count = 1;
    ImGui_ImplWGPU_Init(&imgui_init_info);

    // This is with the older imgui version from Learn WebGPU
    //ImGui_ImplWGPU_Init(device_, 3, surf_caps.formats[0], depthTextureFormat);

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

void Application::update_gui(wgpu::RenderPassEncoder _render_pass)
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
            ImGui::Checkbox("Right Sidebar", &sidebar_right_aligned_);
            ImGui::SliderFloat("Sidebar Width [%]", &sidebar_rel_width_, 0.1f, 0.9f);

            ImGui::ColorEdit3("Clear color", clear_color_);
            ImGui::EndMenu(); // !Settings
        }

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

    for (const auto& plugin : PluginRegistry::get_plugins()) {
        if (ImGui::CollapsingHeader(plugin.second->name())) {
            plugin.second->render_ui(*this);
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), _render_pass);
}

void Application::configure_surface()
{
    if (surface_) {surface_.unconfigure();}

    //int fbWidth, fbHeight;
    //glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
    auto viewport = total_viewport();

    // Configure the surface
    wgpu::SurfaceConfiguration config = {};

    // Configuration of the textures created for the underlying swap chain
    config.width = static_cast<uint32_t>(viewport[2]);
    config.height = static_cast<uint32_t>(viewport[3]);
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.format = color_format_;

    // And we do not need any particular view format:
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.device = device_;
    config.presentMode = wgpu::PresentMode::Fifo;
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;

    surface_.configure(config);
}

void Application::create_depth_texture()
{
    if (depthTextureView) {depthTextureView.release();}
    if (depthTexture) {
        depthTexture.destroy();
        depthTexture.release();
    }

    // int fbWidth, fbHeight;
    // glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
    // std::cout << "Framebuffer size: " << fbWidth << " x " << fbHeight << std::endl;
    auto viewport = total_viewport();

    wgpu::DepthStencilState depthStencilState = create_default_depth_state();
    depthTextureFormat = depthStencilState.format;

    // Create the depth texture
    wgpu::TextureDescriptor depthTextureDesc;
    depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
    depthTextureDesc.format = depthTextureFormat;
    depthTextureDesc.mipLevelCount = 1;
    depthTextureDesc.sampleCount = 1;
    depthTextureDesc.size = {
        static_cast<uint32_t>(viewport[2]),
        static_cast<uint32_t>(viewport[3]),
        1};
    depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
    depthTextureDesc.viewFormatCount = 1;
    depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
    depthTexture = device_.createTexture(depthTextureDesc);

    // Create the view of the depth texture manipulated by the rasterizer
    wgpu::TextureViewDescriptor depthTextureViewDesc;
    depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
    depthTextureViewDesc.baseArrayLayer = 0;
    depthTextureViewDesc.arrayLayerCount = 1;
    depthTextureViewDesc.baseMipLevel = 0;
    depthTextureViewDesc.mipLevelCount = 1;
    depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    depthTextureViewDesc.format = depthTextureFormat;
    depthTextureView = depthTexture.createView(depthTextureViewDesc);

    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;
}

}
