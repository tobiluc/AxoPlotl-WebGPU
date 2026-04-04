#pragma once
#include <AxoPlotl/typedefs/om.hpp>
#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl::IO
{

void openmesh_to_openvolumemesh(const OMSurfaceMesh& _om, OVMVolumeMesh& _ovm);

}
