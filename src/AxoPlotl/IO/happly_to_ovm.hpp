#pragma once
#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/IO/happly.hpp>

namespace AxoPlotl::IO
{

void happly_to_openvolumemesh(happly::PLYData& _data, OVMVolumeMesh& _ovm);

}
