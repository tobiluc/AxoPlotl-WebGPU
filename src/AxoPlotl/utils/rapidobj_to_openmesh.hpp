#pragma once

#include "AxoPlotl/typedefs/ToLoG.hpp"
#include "AxoPlotl/typedefs/om.hpp"
#include <rapidobj/rapidobj.hpp>

namespace AxoPlotl
{

void rapidobj_to_openmesh(
    const rapidobj::Result& _rap,
    OMSurfaceMesh& _mesh);
}
