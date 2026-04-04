#pragma once

#include <AxoPlotl/typedefs/ToLoG.hpp>
#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/rendering/RendererBase.hpp>

namespace AxoPlotl
{

struct StaticRenderData
{
    std::vector<PropertyRendererBase::Position> positions_;
    std::vector<uint32_t> vertices_;
    std::vector<std::pair<uint32_t,uint32_t>> edges_;
    std::vector<std::vector<uint32_t>> faces_;
    std::vector<std::vector<std::vector<uint32_t>>> cells_;
    std::vector<PropertyRendererBase::Position> edges_barycenters_;
    std::vector<PropertyRendererBase::Position> faces_barycenters_;
    std::vector<PropertyRendererBase::Position> cells_barycenters_;
};

StaticRenderData create_static_render_data(const OVMVolumeMesh& _mesh);

wgpu::Buffer create_position_buffer(
    wgpu::Device _device,
    const std::vector<PropertyRendererBase::Position>& _positions);

}
