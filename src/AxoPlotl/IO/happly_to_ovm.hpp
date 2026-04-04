#pragma once
#include <AxoPlotl/typedefs/ovm.hpp>
#include <ToLoG/io/happly.hpp>

namespace AxoPlotl::IO
{

void happly_to_openvolumemesh(happly::PLYData& _data, OVMVolumeMesh& _ovm);

}
