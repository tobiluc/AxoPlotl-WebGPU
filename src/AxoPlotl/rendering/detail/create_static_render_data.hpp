#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/rendering/RendererBase.hpp>

namespace AxoPlotl
{

struct StaticRenderData
{
    std::vector<RendererBase::Position> positions_;
    std::vector<uint32_t> vertices_;
    std::vector<std::pair<uint32_t,uint32_t>> edges_;
    std::vector<std::vector<uint32_t>> faces_;
    std::vector<std::vector<std::vector<uint32_t>>> cells_;
    std::vector<RendererBase::Position> edges_barycenters_;
    std::vector<RendererBase::Position> faces_barycenters_;
    std::vector<RendererBase::Position> cells_barycenters_;
};

StaticRenderData create_static_render_data(const OVMVolumeMesh& _mesh);

wgpu::Buffer create_position_buffer(
    wgpu::Device _device,
    const std::vector<RendererBase::Position>& _positions);

}
