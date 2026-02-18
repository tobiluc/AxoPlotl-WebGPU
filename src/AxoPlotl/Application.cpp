#include "Application.hpp"
#include "AxoPlotl/input/Mouse.hpp"
#include "ImGuiFileDialog.h"
#include <cassert>
#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include <AxoPlotl/PluginRegistry.hpp>

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__
#ifdef WEBGPU_BACKEND_WGPU
#  include <webgpu/wgpu.h>
#endif // WEBGPU_BACKEND_WGPU

namespace AxoPlotl
{

Application::Application()
{}

Application::~Application()
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
    // glfwSetMouseButtonCallback(window, [](GLFWwindow*,int,int,int) {Rendering::triggerRedraw();});
    // glfwSetKeyCallback(window, [](GLFWwindow*,int,int,int,int) {Rendering::triggerRedraw();});

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

    // wgpu::SupportedLimits supportedLimits;
    // adapter.getLimits(&supportedLimits);
    // wgpu::RequiredLimits requiredLimits = wgpu::Default;
    // requiredLimits.limits.maxVertexAttributes = 1;
    // requiredLimits.limits.maxVertexBuffers = 1;
    // requiredLimits.limits.maxBufferSize = 100000 * (3+3) * sizeof(float);
    // requiredLimits.limits.maxVertexBufferArrayStride = (3+3) * sizeof(float);
    // requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
    // requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;

    // requiredLimits.limits.maxBindGroups = 1;
    // requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
    // requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4;

    // requiredLimits.limits.maxDynamicUniformBuffersPerPipelineLayout = 1;

    // // For the depth buffer, we enable textures (up to the size of the window):
    // requiredLimits.limits.maxTextureDimension1D = 480;
    // requiredLimits.limits.maxTextureDimension2D = 640;
    // requiredLimits.limits.maxTextureArrayLayers = 1;

    // deviceDesc.requiredLimits = &requiredLimits;
    // adapter.getLimits(&supportedLimits);
    // std::cout << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;

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

    //------------------
    // Surface
    //------------------

    wgpu::SurfaceCapabilities capabilities = {};
    surface_.getCapabilities(adapter_, &capabilities);
    color_format_ = capabilities.formats[0];

    // Get Width and Height
    int w, h;
    glfwGetWindowSize(window_, &w, &h);

    reconfigure_surface(w, h);

    scene_.init(VolumeMeshRenderer::Context{
        .device_=device_,
        .surface_=surface_,
        .adapter_=adapter_
    });

    if (!init_gui()) {return false;}

    return true;
}

void Application::run()
{
    Input::Mouse::update(window_);
    glfwPollEvents();

    wgpu::CommandEncoder cmdEncoder = device_.createCommandEncoder();

    // Acquire next frame texture
    wgpu::SurfaceTexture surfaceTexture;
    surface_.getCurrentTexture(&surfaceTexture);
    // wgpu::TextureView backBufferView =
    //     wgpu::Texture(surfaceTexture.texture).createView();
    // Create a view for this surface texture
    WGPUTextureViewDescriptor viewDescriptor;
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

    // Begin render pass
    wgpu::RenderPassColorAttachment colorAttachment{};
    colorAttachment.view = targetView;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = {0.6f, 0.6f, 0.6f, 1.0f};
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.nextInChain = nullptr;
#ifndef WEBGPU_BACKEND_WGPU
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif //!WEBGPU_BACKEND_WGPU

    wgpu::RenderPassDescriptor renderPassDesc{};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;

    wgpu::RenderPassEncoder renderPass =
        cmdEncoder.beginRenderPass(renderPassDesc);

    // Call renderer
    scene_.render(window_, renderPass);

    update_gui(renderPass);

    // End pass
    renderPass.end();

    // Submit
    wgpu::CommandBuffer cmdBuffer = cmdEncoder.finish();
    queue_.submit(1, &cmdBuffer);

    // Present
    surface_.present();
    wgpuTextureRelease(surfaceTexture.texture);
}

void Application::on_window_resize(float width, float height)
{
    if (width == 0|| height == 0) {return;} // window minimized

    surface_.unconfigure();
    reconfigure_surface(width, height);

    //pipeline.updateProjection(width/height);
}

void Application::terminate()
{
    terminate_gui();

    adapter_.release();
    surface_.unconfigure();
    queue_.release();
    surface_.release();
    device_.release();
    adapter_.release();

    glfwDestroyWindow(window_);
    glfwTerminate();
}

bool Application::init_gui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
     ImGui::GetIO();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOther(window_, true);
    wgpu::SurfaceCapabilities surf_caps{};
    surface_.getCapabilities(adapter_, &surf_caps);
    // TODO: pass depth format
    ImGui_ImplWGPU_Init(device_, 3, surf_caps.formats[0]);

    return true;
}

void Application::update_gui(wgpu::RenderPassEncoder _render_pass)
{
    // Start the Dear ImGui frame
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Top Menu
    // Start the main menu bar
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0,0,0,0.5f));
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

        // Help menu
        if (ImGui::BeginMenu("Help"))
        {
            ImGui::EndMenu();
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

    auto& plugins = PluginRegistry::get_plugins();
    for (const auto& plugin : plugins) {
        if (ImGui::CollapsingHeader(plugin->name())) {
            plugin->render_ui(*this);
        }
    }

    ImGui::PopStyleColor();

    // Draw the UI
    ImGui::EndFrame();
    // Convert the UI defined above into low-level drawing commands
    ImGui::Render();

    // std::cerr << "Scale: "
    //           << ImGui::GetDrawData()->FramebufferScale.x << " x "
    //           << ImGui::GetDrawData()->FramebufferScale.x << std::endl;

    // Execute the low-level drawing commands on the WebGPU backend
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), _render_pass);
}

void Application::terminate_gui()
{
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();
}

void Application::reconfigure_surface(float width, float height)
{
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);

    // Configure the surface
    wgpu::SurfaceConfiguration config = {};

    // Configuration of the textures created for the underlying swap chain
    config.width = static_cast<uint32_t>(fbWidth);
    config.height = static_cast<uint32_t>(fbHeight);
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

}
