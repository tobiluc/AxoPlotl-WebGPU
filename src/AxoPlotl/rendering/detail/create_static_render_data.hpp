#pragma once

#include "AxoPlotl/rendering/VolumeMeshRenderer.hpp"
#include "AxoPlotl/typedefs/ToLoG.hpp"
#include "AxoPlotl/typedefs/ovm.hpp"

namespace AxoPlotl
{

VolumeMeshRenderer::StaticData create_static_render_data(const SurfaceMesh& _mesh);

VolumeMeshRenderer::StaticData create_static_render_data(const VolumeMesh& _mesh);

inline wgpu::Buffer create_position_buffer(
    wgpu::Device _device,
    const std::vector<RendererBase::Position>& _positions)
{
    wgpu::BufferDescriptor desc{};
    desc.usage =
        wgpu::BufferUsage::Storage |
        wgpu::BufferUsage::CopyDst |
        wgpu::BufferUsage::Vertex;
    desc.size = sizeof(RendererBase::Position) * _positions.size();
    desc.mappedAtCreation = false;
    desc.label = "Position";

    wgpu::Buffer buffer = _device.createBuffer(desc);
    _device.getQueue().writeBuffer(buffer, 0, _positions.data(), desc.size);
    return buffer;
}

}
