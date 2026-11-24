#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl::internal
{

void add_halfface_boundary_prop(VolMesh& _mesh, const std::string& _name = "AxoPlotl::halfface_is_boundary");

void add_vertex_valence_prop(VolMesh& _mesh, const std::string& _name = "AxoPlotl::vertex_valence");

void add_edge_valence_prop(VolMesh& _mesh, const std::string& _name = "AxoPlotl::edge_valence");

template<typename EntityTag>
void add_entity_index_prop(VolMesh& _mesh, const std::string& _name = "AxoPlotl::idx")
{
    auto p = _mesh.create_persistent_property<int,EntityTag>(_name);
    if (!p) {return;}
    for (const auto& h : _mesh.entities<EntityTag>()) {
        (*p)[h] = h.idx();
    }
}

}
