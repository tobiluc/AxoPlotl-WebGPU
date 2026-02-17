#include "Application.hpp"
#include <cassert>

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

bool Application::init(GLFWwindow *_window)
{
    assert(_window);

    //------------------
    // Instance
    //------------------

    wgpu::Instance instance = wgpuCreateInstance(nullptr);

    surface_ = glfwGetWGPUSurface(instance, _window);

    std::cout << "Requesting adapter..." << std::endl;
    surface_ = glfwGetWGPUSurface(instance, _window);
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
        std::cout << "Uncaptured device error: type " << type;
        if (message) std::cout << " (" << message << ")";
        std::cout << std::endl;
    });

    queue_ = device_.getQueue();

    //------------------
    // Surface
    //------------------

    wgpu::SurfaceCapabilities capabilities = {};
    surface_.getCapabilities(adapter_, &capabilities);
    color_format_ = capabilities.formats[0];

    reconfigure_surface(640.f, 480.f);

    scene_.init(VolumeMeshRenderer::Context{
        .device_=device_,
        .surface_=surface_,
        .adapter_=adapter_
    });

    return true;
}

void Application::run()
{
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

    // Call your renderer
    scene_.render(renderPass);

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

void Application::release()
{
    adapter_.release();
    surface_.unconfigure();
    queue_.release();
    surface_.release();
    device_.release();
    adapter_.release();
}

void Application::reconfigure_surface(float width, float height)
{
    // Configure the surface
    wgpu::SurfaceConfiguration config = {};

    // Configuration of the textures created for the underlying swap chain
    config.width = static_cast<uint32_t>(width);
    config.height = static_cast<uint32_t>(height);
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
