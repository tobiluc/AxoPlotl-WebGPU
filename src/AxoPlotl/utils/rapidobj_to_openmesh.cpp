#include "rapidobj_to_openmesh.hpp"

void AxoPlotl::rapidobj_to_openmesh(
    const rapidobj::Result& _rap,
    OMSurfaceMesh& _mesh)
{
    for (unsigned int shape_i = 0;
         shape_i < _rap.shapes.size(); ++shape_i) {
        for (const auto& pt_idx : _rap.shapes.at(shape_i).points.indices) {
            int i = pt_idx.position_index;
            _mesh.add_vertex(OMVec3(
                _rap.attributes.positions[3*i+0],
                _rap.attributes.positions[3*i+1],
                _rap.attributes.positions[3*i+2]
                ));
        }
    }
}

