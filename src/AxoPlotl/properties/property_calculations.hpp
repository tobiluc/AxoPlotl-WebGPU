#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl::internal
{

void add_face_normal_prop(
    VolMesh& _mesh, std::string _name = "AxoPlotl::face_normal");

template<typename EntityTag>
void add_entity_index_prop(VolMesh& _mesh, std::string _name = "AxoPlotl::idx")
{
    auto p = _mesh.create_persistent_property<int,EntityTag>(_name);
    if (!p) {return;}
    for (const auto& h : _mesh.entities<EntityTag>()) {
        (*p)[h] = h.idx();
    }
}

}
