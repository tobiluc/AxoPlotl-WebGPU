#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl
{

OVM::CellPropertyT<int> calc_cell_boundary_distance(
    OVMVolumeMesh& _mesh);

OVM::CellPropertyT<double> calc_cell_min_dihedral_angle(OVMVolumeMesh& _mesh);

OVM::VertexPropertyT<OVM::Vec3f> calc_vertex_normalized_position(OVMVolumeMesh& _mesh);

}
