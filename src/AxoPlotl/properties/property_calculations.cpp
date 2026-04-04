#include <AxoPlotl/properties/property_calculations.hpp>
#include <queue>

namespace AxoPlotl
{

OVM::CellPropertyT<int> calc_cell_boundary_distance(OVMVolumeMesh& _mesh)
{
    auto c_boundary_distance = _mesh.request_cell_property<int>("AxoPlotl::boundary_distance", INT_MAX);
    _mesh.set_persistent(c_boundary_distance);
    std::queue<OVM::CH> queue;

    // Init
    for (auto ch : _mesh.cells()) {
        if (ch.is_boundary()) {
            c_boundary_distance[ch] = 0;
            queue.push(ch);
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

// OVM::CellPropertyT<double> calc_cell_min_dihedral_angle(OVMVolumeMesh& _mesh)
// {
//     auto c_min_dihedral_angle = _mesh.request_cell_property<double>("AxoPlotl::min_dihedral_angle");
//     _mesh.set_persistent(c_min_dihedral_angle);

//     for (auto ch : _mesh.cells()) {

//         // Compute the dihedral angles

//         c_min_dihedral_angle[ch] = std::numeric_limits<double>::infinity();
//         for (auto eh : ch.edges()) {
//             std::vector<OpenVolumeMesh::HFH> hfhs;
//             for (auto hfh :ch.halffaces()) {
//                 if (_mesh.is_incident(hfh.face_handle(), eh)) {
//                     hfhs.push_back(hfh);
//                 }
//             }
//             assert(hfhs.size()==2);
//             OpenVolumeMesh::HEH heh = eh.h0();
//             for (auto heh0 : _mesh.halfface_halfedges(hfhs[1])) {
//                 if (heh0 == heh) {
//                     heh = heh.opposite_handle();
//                     break;
//                 }
//             }
//             OpenVolumeMesh::VH vh_a = _mesh.from_vertex_handle(heh);
//             OpenVolumeMesh::VH vh_b = _mesh.to_vertex_handle(heh);
//             OpenVolumeMesh::VH vh_p(-1);
//             for (auto vh : _mesh.get_halfface_vertices(hfhs[0])) {
//                 if (vh != vh_a && vh != vh_b) {
//                     vh_p = vh;
//                     break;
//                 }
//             }
//             assert(vh_p.is_valid());
//             OpenVolumeMesh::VH vh_q(-1);
//             for (auto vh : _mesh.get_halfface_vertices(hfhs[1])) {
//                 if (vh != vh_a && vh != vh_b) {
//                     vh_q = vh;
//                     break;
//                 }
//             }
//             assert(vh_q.is_valid());
//             double alpha = ToLoG::dihedral_angle(
//                 _mesh.vertex(vh_a),
//                 _mesh.vertex(vh_b),
//                 _mesh.vertex(vh_p),
//                 _mesh.vertex(vh_q)
//                 );
//             c_min_dihedral_angle[ch] = std::min(c_min_dihedral_angle[ch], alpha);
//         }
//     }
//     return c_min_dihedral_angle;
// }

OVM::VertexPropertyT<OVM::Vec3f> calc_vertex_normalized_position(OVMVolumeMesh& _mesh)
{
    auto v_normalized_pos = _mesh.request_vertex_property<OVM::Vec3f>("AxoPlotl::normalized_position");
    _mesh.set_persistent(v_normalized_pos);

    for (auto vh : _mesh.vertices()) {
        v_normalized_pos[vh] = _mesh.vertex(vh).normalized();
    }

    return v_normalized_pos;
}

}
