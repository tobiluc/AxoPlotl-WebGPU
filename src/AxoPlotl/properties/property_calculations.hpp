#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl::internal
{

OVM::CellPropertyT<int> calc_cell_boundary_distance(
    OVMVolumeMesh& _mesh, std::string _name = "AxoPlotl::boundary_distance");

//OVM::CellPropertyT<double> calc_cell_min_dihedral_angle(OVMVolumeMesh& _mesh);

OVM::VertexPropertyT<OVM::Vec3f> calc_vertex_normalized_position(
    OVMVolumeMesh& _mesh, std::string _name = "AxoPlotl::normalized_position");

OVM::FacePropertyT<OVM::Vec3f> calc_face_normal(
    OVMVolumeMesh& _mesh, std::string _name = "AxoPlotl::face_normal");

}
