#pragma once

#include <webgpu/webgpu.hpp>

namespace AxoPlotl::internal
{

struct GpuContext
{
    wgpu::Adapter adapter;
    wgpu::Device device;
    // wgpu::Surface surface;
    // wgpu::TextureFormat surface_format;

    wgpu::Queue queue() const {return device.getQueue();}

    void release() {
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
};

struct RenderContext
{
    wgpu::Surface surface;
    wgpu::TextureFormat surface_format;

    static wgpu::TextureFormat depth_format;
    static wgpu::TextureFormat picking_format;

    wgpu::SurfaceCapabilities get_surface_capabilities(const GpuContext& _gpu_context) const {
        wgpu::SurfaceCapabilities surf_caps{};
        surface.getCapabilities(_gpu_context.adapter, &surf_caps);
        return surf_caps;
    }

    void release() {
        if (surface) {
            surface.unconfigure();
            surface.release();
            surface = nullptr;
        }
        surface_format = wgpu::TextureFormat::Undefined;
    }

    void configure_surface(const GpuContext& _gpu_context, const int _width, const int _height)
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
};


}
