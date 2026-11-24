#include <AxoPlotl/properties/property_calculations.hpp>

namespace AxoPlotl::internal
{

void add_halfface_boundary_prop(VolMesh& _mesh, const std::string& _name)
{
    auto hf_b = _mesh.request_halfface_property<bool>(_name);
    _mesh.set_persistent(hf_b);
    for (OVM::HFH hfh : _mesh.halffaces()) {
        hf_b[hfh] = _mesh.is_boundary(hfh);
    }
}

void add_vertex_valence_prop(VolMesh& _mesh, const std::string& _name)
{
    auto v_v = _mesh.request_vertex_property<int>(_name);
    _mesh.set_persistent(v_v);
    for (OVM::VH vh : _mesh.vertices()) {
        v_v[vh] = _mesh.valence(vh);
    }
}

void add_edge_valence_prop(VolMesh& _mesh, const std::string& _name)
{
    auto e_v = _mesh.request_edge_property<int>(_name);
    _mesh.set_persistent(e_v);
    for (OVM::EH eh : _mesh.edges()) {
        e_v[eh] = _mesh.valence(eh);
    }
}

}
