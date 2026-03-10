#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl
{

inline OVMVolumeMesh mesh_with_nan_and_inf_props()
{
    OVMVolumeMesh mesh;
    auto v_d = mesh.create_persistent_vertex_property<double>("v_double").value();
    auto e_f = mesh.create_persistent_edge_property<float>("e_float").value();
    v_d[mesh.add_vertex(OVM::Vec3f(1,1,0))] = 0.0;
    v_d[mesh.add_vertex(OVM::Vec3f(2,1,0))] = 1.0;
    v_d[mesh.add_vertex(OVM::Vec3f(3,1,0))] = 2.0;
    v_d[mesh.add_vertex(OVM::Vec3f(4,1,0))] = 3.0;

    v_d[mesh.add_vertex(OVM::Vec3f(1,2,0))] = NAN;
    v_d[mesh.add_vertex(OVM::Vec3f(2,2,0))] = std::numeric_limits<double>::quiet_NaN();
    v_d[mesh.add_vertex(OVM::Vec3f(3,2,0))] = -std::numeric_limits<double>::quiet_NaN();

    v_d[mesh.add_vertex(OVM::Vec3f(1,3,0))] = INFINITY;
    v_d[mesh.add_vertex(OVM::Vec3f(2,3,0))] = std::numeric_limits<double>::infinity();
    v_d[mesh.add_vertex(OVM::Vec3f(3,3,0))] = -std::numeric_limits<double>::infinity();

    e_f[mesh.add_edge(OVM::VH(0), OVM::VH(1))] = std::numeric_limits<float>::quiet_NaN();

    return mesh;
}

}
