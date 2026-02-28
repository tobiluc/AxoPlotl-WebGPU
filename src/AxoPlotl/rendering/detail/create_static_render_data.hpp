#pragma once

#include "AxoPlotl/rendering/OpenVolumeMeshRenderer.hpp"
#include "AxoPlotl/typedefs/ToLoG.hpp"
#include "AxoPlotl/typedefs/om.hpp"
#include "AxoPlotl/typedefs/ovm.hpp"

namespace AxoPlotl
{

// VolumeMeshRenderer::StaticData create_static_render_data(const SurfaceMesh& _mesh);

OpenVolumeMeshRenderer::StaticData create_static_render_data(const OMSurfaceMesh& _mesh);

OpenVolumeMeshRenderer::StaticData create_static_render_data(const OVMVolumeMesh& _mesh);

wgpu::Buffer create_position_buffer(
    wgpu::Device _device,
    const std::vector<RendererBase::Position>& _positions);

}
