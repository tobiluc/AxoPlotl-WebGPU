// #include <AxoPlotl/IO/om_to_ovm.hpp>

// namespace AxoPlotl::IO
// {

// void openmesh_to_openvolumemesh(const OpenMesh::PolyMesh_ArrayKernelT<OpenMesh::DefaultTraits>& _om, OVMVolumeMesh& _ovm)
// {
//     _ovm.clear();
//     _ovm.reserve_vertices(_om.n_vertices());
//     _ovm.reserve_edges(_om.n_edges());
//     _ovm.reserve_faces(_om.n_faces());
//     for (auto vh : _om.vertices()) {
//         const auto& p = _om.point(vh);
//         _ovm.add_vertex(OVMVolumeMesh::PointT(p[0],p[1],p[2]));
//     }
//     for (auto eh : _om.edges()) {
//         _ovm.add_edge(OVM::VH(eh.v0().idx()), OVM::VH(eh.v1().idx()));
//     }
//     for (auto fh : _om.faces()) {
//         std::vector<OVM::VH> vhs;
//         vhs.reserve(fh.valence());
//         for (auto vh : fh.vertices_ccw()) {
//             vhs.emplace_back(vh.idx());
//         }
//         _ovm.add_face(vhs);
//     }
// }

// }
