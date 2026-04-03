#pragma once
#include <AxoPlotl/typedefs/ovm.hpp>
#include <rapidobj/rapidobj.hpp>

namespace AxoPlotl
{

OVMVolumeMesh rapidobj_to_openvolumemesh(const rapidobj::Result& _robj);

}
