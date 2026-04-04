#include <AxoPlotl/IO/rapidobj_to_ovm.hpp>

namespace AxoPlotl::IO
{

void rapidobj_to_openvolumemesh(const rapidobj::Result& _robj, OVMVolumeMesh& _ovm)
{
    _ovm.clear();
    auto v_shape_i = _ovm.create_persistent_vertex_property<int>("rabidobj::v_shape", -1).value();
    auto e_shape_i = _ovm.create_persistent_edge_property<int>("rabidobj::e_shape", -1).value();
    auto f_shape_i = _ovm.create_persistent_face_property<int>("rabidobj::f_shape", -1).value();
    auto f_normal = _ovm.create_persistent_face_property<OVMVolumeMesh::Point>("rabidobj::f_normal", {0,0,0}).value();
    using Vec3 = OVMVolumeMesh::Point;

    // Positions
    _ovm.reserve_vertices(_robj.attributes.positions.size()/3);
    for (int i = 0; i < _robj.attributes.positions.size(); i += 3) {
        _ovm.add_vertex(Vec3(
            _robj.attributes.positions[i+0],
            _robj.attributes.positions[i+1],
            _robj.attributes.positions[i+2]
            ));
    }

    // Shapes
    for (unsigned int shape_i = 0; shape_i < _robj.shapes.size(); ++shape_i) {

        // Points
        for (const auto& pt_idx : _robj.shapes.at(shape_i).points.indices) {
            OVM::VH vh(pt_idx.position_index);
            v_shape_i[vh] = shape_i;
        }

        // Edges
        int idx(0);
        for (const auto& n_inds : _robj.shapes.at(shape_i).lines.num_line_vertices) {
            std::vector<OVM::VH> vhs;
            for (int i = idx; i < idx+n_inds; ++i) {
                int pt_idx = _robj.shapes.at(shape_i).lines.indices[i].position_index;
                vhs.push_back(OVM::VH(pt_idx));
            }
            for (int i = 0; i < vhs.size()-1; ++i) {
                OVM::EH eh = _ovm.add_edge(vhs[i], vhs[i+1]);
                e_shape_i[eh] = shape_i;
            }
            idx += n_inds;
        }

        // Faces
        idx = 0;
        for (const auto& n_inds : _robj.shapes.at(shape_i).mesh.num_face_vertices) {
            std::vector<OVM::VH> vhs;
            OVMVolumeMesh::PointT normal(0,0,0);
            for (int i = idx; i < idx+n_inds; ++i) {

                int pt_idx = _robj.shapes.at(shape_i).mesh.indices[i].position_index;
                OVM::VH vh(pt_idx);
                vhs.push_back(vh);

                int n_idx = _robj.shapes.at(shape_i).mesh.indices[i].normal_index;
                if (n_idx >= 0) {
                    normal[0] += _robj.attributes.normals[3*n_idx+0];
                    normal[1] += _robj.attributes.normals[3*n_idx+1];
                    normal[2] += _robj.attributes.normals[3*n_idx+2];
                }
            }
            OVM::FH fh = _ovm.add_face(vhs);
            f_shape_i[fh] = shape_i;
            f_normal[fh] = (normal==OVMVolumeMesh::PointT(0,0,0))? normal : normal.normalized();
            idx += n_inds;
        }
    }
}

}
