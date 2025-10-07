#include "Renderer.h"
#include "Buffer.h"
#include "Pipeline.h"
#include "Shader.h"

namespace AxoPlotl
{

Renderer::Renderer()
{}

bool Renderer::init(GLFWwindow* window)
{
    //------------------
    // Instance
    //------------------

    wgpu::Instance instance = wgpuCreateInstance(nullptr);

    surface = glfwGetWGPUSurface(instance, window);

    std::cout << "Requesting adapter..." << std::endl;
    surface = glfwGetWGPUSurface(instance, window);
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.compatibleSurface = surface;
    wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
    std::cout << "Got adapter: " << adapter << std::endl;

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
    deviceDesc.deviceLostCallbackInfo;
    deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
        std::cout << "Device lost: reason " << reason;
        if (message) std::cout << " (" << message << ")";
        std::cout << std::endl;
    }; // Warning: DeviceDescriptor.deviceLostCallback and DeviceDescriptor.deviceLostUserdata are deprecated. Use DeviceDescriptor.deviceLostCallbackInfo instead.

    wgpu::SupportedLimits supportedLimits;
    adapter.getLimits(&supportedLimits);
    wgpu::RequiredLimits requiredLimits = wgpu::Default;
    requiredLimits.limits.maxVertexAttributes = 1;
    requiredLimits.limits.maxVertexBuffers = 1;
    requiredLimits.limits.maxBufferSize = 6 * (2+3) * sizeof(float);
    requiredLimits.limits.maxVertexBufferArrayStride = (2+3) * sizeof(float);
    requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
    requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
    deviceDesc.requiredLimits = &requiredLimits;
    adapter.getLimits(&supportedLimits);
    std::cout << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;
    device = adapter.requestDevice(deviceDesc);
    std::cout << "Got device: " << device << std::endl;
    device.getLimits(&supportedLimits);
    std::cout << "device.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;

    uncapturedErrorCallbackHandle = device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
        std::cout << "Uncaptured device error: type " << type;
        if (message) std::cout << " (" << message << ")";
        std::cout << std::endl;
    });

    queue = device.getQueue();

    //------------------
    // Surface
    //------------------

    // Configure the surface
    wgpu::SurfaceConfiguration config = {};

    // Configuration of the textures created for the underlying swap chain
    config.width = 640;
    config.height = 480;
    config.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::SurfaceCapabilities capabilities = {};
    surface.getCapabilities(adapter, &capabilities);
    colorFormat = capabilities.formats[0];
    config.format = colorFormat;

    // And we do not need any particular view format:
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.device = device;
    config.presentMode = wgpu::PresentMode::Fifo;
    config.alphaMode = wgpu::CompositeAlphaMode::Auto;

    surface.configure(config);

    // Release the adapter only after it has been fully utilized
    adapter.release();

    //-----------
    // Pipeline
    //-----------
    wgpu::ShaderModule shaderModule = loadShaderModule(device, "/Users/tobiaskohler/Projects/AxoPlotl-WebGPU/shaders/Shader.wgsl");
    pipeline = createPipeline(device, shaderModule, colorFormat);

    //-----------
    // Buffer
    //-----------
    std::vector<float> vertexData = {
        // x0, y0
        -0.5, -0.5,  1.0,0.0,0.0,
        // x1, y1
        +0.5, -0.5, 0.0,1.0,0.0,
        // x2, y2
        +0.0, +0.5,0.0,0.0,1.0,

        // Add a second triangle:
        -0.55f, -0.5,1.0,0.0,0.0,
        -0.05f, +0.5,0.0,1.0,0.0,
        -0.55f, +0.5, 0.0,0.0,1.0
    };
    vertexCount = static_cast<uint32_t>(vertexData.size() / (2+3));
    vertexBuffer = createVertexBuffer(device, vertexData);
    queue.writeBuffer(vertexBuffer, 0, vertexData.data(), vertexData.size() * sizeof(float));


    std::vector<uint32_t> indexData = {
        0, 1, 2
    };
    indexCount = static_cast<uint32_t>(indexData.size());
    indexBuffer = createIndexBuffer(device, indexData);
    queue.writeBuffer(indexBuffer, 0, indexData.data(), indexData.size() * sizeof(uint32_t));

    return true;
}

Renderer::~Renderer()
{
    vertexBuffer.release();
    indexBuffer.release();
    pipeline.release();
    surface.unconfigure();
    queue.release();
    surface.release();
    device.release();
}

void Renderer::render()
{
    // Get the surface texture
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {return;}

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

    if (!targetView) {return;}

    // Create a command encoder for the draw call
    wgpu::CommandEncoderDescriptor encoderDesc = {};
    encoderDesc.label = "My command encoder";
    wgpu::CommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

    // Create the render pass that clears the screen with our color
    wgpu::RenderPassDescriptor renderPassDesc = {};

    // The attachment part of the render pass descriptor describes the target texture of the pass
    wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
    renderPassColorAttachment.view = targetView;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
    renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
    renderPassColorAttachment.clearValue = WGPUColor{ 0.9, 0.1, 0.2, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &renderPassColorAttachment;
    renderPassDesc.depthStencilAttachment = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

    renderPass.setPipeline(pipeline); // Select which render pipeline to use
    renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexBuffer.getSize());
    renderPass.setIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0, indexBuffer.getSize());
    //renderPass.draw(vertexCount, 1, 0, 0);
    renderPass.drawIndexed(indexCount, 1, 0, 0, 0);

    renderPass.end();
    renderPass.release();

    // Finally encode and submit the render pass
    wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.label = "Command buffer";
    wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
    encoder.release();

    std::cout << "Submitting command..." << std::endl;
    queue.submit(1, &command);
    command.release();
    std::cout << "Command submitted." << std::endl;

    // At the enc of the frame
    targetView.release();
#ifndef __EMSCRIPTEN__
    surface.present();
#endif

#if defined(WEBGPU_BACKEND_DAWN)
    device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
    device.poll(false);
#endif
}

}
