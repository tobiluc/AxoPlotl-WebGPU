#include "MyGpuContext.hpp"

namespace AxoPlotl::internal
{

wgpu::TextureFormat RenderContext::depth_format = wgpu::TextureFormat::Depth24Plus;
wgpu::TextureFormat RenderContext::picking_format = wgpu::TextureFormat::RGBA32Uint;

void GpuContext::init(wgpu::Instance _instance,
                      const wgpu::Surface& _surface)
{
    //------------------
    // Instance
    //------------------
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.compatibleSurface = _surface;
    adapter = _instance.requestAdapter(adapterOpts);
    _instance.release();

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

    device = adapter.requestDevice(deviceDesc);
}

void GpuContext::release() {
    auto queue = device.getQueue();
    if (queue) {
        queue.release();
        queue = nullptr;
    }
    if (adapter) {
        adapter.release();
        adapter = nullptr;
    }
    if (device) {
        device.release();
        device = nullptr;
    }
}

wgpu::SurfaceCapabilities RenderContext::get_surface_capabilities(const GpuContext& _gpu_context) const {
    wgpu::SurfaceCapabilities surf_caps{};
    surface.getCapabilities(_gpu_context.adapter, &surf_caps);
    return surf_caps;
}

void RenderContext::release() {
    if (surface) {
        surface.unconfigure();
        surface.release();
        surface = nullptr;
    }
    surface_format = wgpu::TextureFormat::Undefined;
}

void RenderContext::configure_surface(const GpuContext& _gpu_context, const int _width, const int _height)
{
    // Configure the surface
    wgpu::SurfaceConfiguration config = {};

    // Configuration of the textures created for the underlying swap chain
    config.width = static_cast<uint32_t>(_width);
    config.height = static_cast<uint32_t>(_height);
    config.usage = wgpu::TextureUsage::RenderAttachment;
    auto capabilities = get_surface_capabilities(_gpu_context);
    config.format = capabilities.formats[0];
    surface_format = config.format;

    // And we do not need any particular view format:
    config.viewFormatCount = 0;
    config.viewFormats = nullptr;
    config.device = _gpu_context.device;
    config.presentMode = wgpu::PresentMode::Fifo;
    config.alphaMode = capabilities.alphaModes[0];

    surface.unconfigure();
    surface.configure(config);
}

}
