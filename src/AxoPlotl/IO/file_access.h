#pragma once

#include <filesystem>
#include "AxoPlotl/typedefs/ToLoG.hpp"
#include "AxoPlotl/typedefs/ovm.hpp"

namespace AxoPlotl::IO
{

std::optional<std::variant<SurfaceMesh, VolumeMesh> > read_mesh(
    const std::filesystem::path& _path);

}
