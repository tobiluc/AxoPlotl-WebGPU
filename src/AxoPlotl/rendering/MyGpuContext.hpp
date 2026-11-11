#pragma once

#include <webgpu/webgpu.hpp>

namespace AxoPlotl::internal
{

struct GpuContext
{
    wgpu::Adapter adapter;
    wgpu::Device device;

    void init(wgpu::Instance _instance,
              const wgpu::Surface& _surface);

    wgpu::Queue queue() const {return device.getQueue();}

    void release();
};

struct RenderContext
{
    wgpu::Surface surface;
    wgpu::TextureFormat surface_format;

    static wgpu::TextureFormat depth_format;
    static wgpu::TextureFormat picking_format;

    wgpu::SurfaceCapabilities get_surface_capabilities(const GpuContext& _gpu_context) const;

    void release();

    void configure_surface(const GpuContext& _gpu_context, const int _width, const int _height);
};


}
