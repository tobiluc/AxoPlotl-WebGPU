#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl
{

OVM::CellPropertyT<int> calc_cell_boundary_distance(
    VolumeMesh& _mesh);

OVM::CellPropertyT<double> calc_cell_min_dihedral_angle(VolumeMesh& _mesh);

}
