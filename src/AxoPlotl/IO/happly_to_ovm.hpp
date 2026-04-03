#pragma once
#include <AxoPlotl/typedefs/ovm.hpp>
#include <ToLoG/io/happly.hpp>

namespace AxoPlotl
{

OVMVolumeMesh happly_to_openvolumemesh(happly::PLYData& _data);

}
