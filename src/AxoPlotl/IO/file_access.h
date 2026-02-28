#pragma once

#include <filesystem>
#include "AxoPlotl/typedefs/ToLoG.hpp"
#include "AxoPlotl/typedefs/om.hpp"
#include "AxoPlotl/typedefs/ovm.hpp"

namespace AxoPlotl::IO
{

std::optional<std::variant<SurfaceMesh,OVMVolumeMesh,OMSurfaceMesh>> read_mesh(
    const std::filesystem::path& _path);

}
