#pragma once

#include "AxoPlotl/rendering/VolumeMeshRenderer.hpp"
#include "AxoPlotl/typedefs/ToLoG.hpp"
#include "AxoPlotl/typedefs/ovm.hpp"

namespace AxoPlotl
{

VolumeMeshRenderer::StaticData create_static_render_data(const SurfaceMesh& _mesh);

VolumeMeshRenderer::StaticData create_static_render_data(const VolumeMesh& _mesh);

}
