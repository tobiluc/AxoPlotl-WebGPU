#include <AxoPlotl/IO/happly_to_ovm.hpp>

namespace AxoPlotl::IO
{

void happly_to_openvolumemesh(happly::PLYData &_data, OVMVolumeMesh& _ovm)
{
    _ovm.clear();
    std::vector<std::array<double,3>> vPos = _data.getVertexPositions();
    std::vector<std::vector<uint32_t>> fInd = _data.getFaceIndices<uint32_t>();
    _ovm.reserve_vertices(vPos.size());
    _ovm.reserve_faces(fInd.size());
    for (const auto& p : vPos) {
        _ovm.add_vertex(OVMVolumeMesh::Point(p[0],p[1],p[2]));
    }
    for (const auto& f : fInd) {
        std::vector<OVM::VH> vhs(f.size());
        for (int i = 0; i < f.size(); ++i) {
            vhs[i] = OVM::VH(f[i]);
        }
        _ovm.add_face(vhs);
    }
}

}
