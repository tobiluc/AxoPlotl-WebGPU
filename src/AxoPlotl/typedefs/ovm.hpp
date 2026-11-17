#pragma once
#include <OpenVolumeMesh/Core/GeometryKernel.hh>

namespace AxoPlotl
{

namespace OVM = OpenVolumeMesh;

using OVMVec3 = OVM::Vec3f;

using VolMesh = OVM::GeometryKernel<
    OVMVec3, OpenVolumeMesh::TopologyKernel>;

}
