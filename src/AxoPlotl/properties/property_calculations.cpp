#include <AxoPlotl/properties/property_calculations.hpp>
#include <queue>
#include <ToLoG/Core.hpp>

namespace AxoPlotl
{

OVM::CellPropertyT<int> calc_cell_boundary_distance(OVMVolumeMesh& _mesh)
{
    auto c_boundary_distance = _mesh.request_cell_property<int>("AxoPlotl::boundary_distance", INT_MAX);
    _mesh.set_persistent(c_boundary_distance);
    std::queue<OVM::CH> queue;

    // Init
    for (auto c_it = _mesh.c_iter(); c_it.is_valid(); ++c_it) {
        if (_mesh.is_boundary(*c_it)) {
            c_boundary_distance[*c_it] = 0;
            queue.push(*c_it);
        }
    }

    // Propagate
    while (!queue.empty()) {
        OVM::CH ch = queue.front();
        queue.pop();

        int d = c_boundary_distance[ch];

        // Check neighbours
        for (auto cc_it = _mesh.cc_iter(ch); cc_it.is_valid(); ++cc_it) {
            OVM::CH ch2 = *cc_it;
            if (c_boundary_distance[ch2] > d + 1) {
                c_boundary_distance[ch2] = d + 1;
                queue.push(ch2);
            }
        }
    }

    return c_boundary_distance;
}

OVM::CellPropertyT<double> calc_cell_min_dihedral_angle(OVMVolumeMesh& _mesh)
{
    auto c_min_dihedral_angle = _mesh.request_cell_property<double>("AxoPlotl::min_dihedral_angle");
    _mesh.set_persistent(c_min_dihedral_angle);

    for (auto c_it = _mesh.c_iter(); c_it.is_valid(); ++c_it) {

        // Compute the dihedral angles
        c_min_dihedral_angle[*c_it] = std::numeric_limits<double>::infinity();
        for (auto ce_it = _mesh.ce_iter(*c_it); ce_it.is_valid(); ++ce_it) {
            std::vector<OpenVolumeMesh::HFH> hfhs;
            for (auto hfh : _mesh.cell(*c_it).halffaces()) {
                if (_mesh.is_incident(hfh.face_handle(), *ce_it)) {
                    hfhs.push_back(hfh);
                }
            }
            assert(hfhs.size()==2);
            OpenVolumeMesh::HEH heh = ce_it->halfedge_handle(0);
            for (auto heh0 : _mesh.halfface_halfedges(hfhs[1])) {
                if (heh0 == heh) {
                    heh = heh.opposite_handle();
                    break;
                }
            }
            OpenVolumeMesh::VH vh_a = _mesh.from_vertex_handle(heh);
            OpenVolumeMesh::VH vh_b = _mesh.to_vertex_handle(heh);
            OpenVolumeMesh::VH vh_p(-1);
            for (auto vh : _mesh.get_halfface_vertices(hfhs[0])) {
                if (vh != vh_a && vh != vh_b) {
                    vh_p = vh;
                    break;
                }
            }
            assert(vh_p.is_valid());
            OpenVolumeMesh::VH vh_q(-1);
            for (auto vh : _mesh.get_halfface_vertices(hfhs[1])) {
                if (vh != vh_a && vh != vh_b) {
                    vh_q = vh;
                    break;
                }
            }
            assert(vh_q.is_valid());
            double alpha = ToLoG::dihedral_angle(
                _mesh.vertex(vh_a),
                _mesh.vertex(vh_b),
                _mesh.vertex(vh_p),
                _mesh.vertex(vh_q)
                );
            c_min_dihedral_angle[*c_it] = std::min(c_min_dihedral_angle[*c_it], alpha);
        }
    }
    return c_min_dihedral_angle;
}

}
