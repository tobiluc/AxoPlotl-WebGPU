#include <AxoPlotl/IO/happly_to_ovm.hpp>

namespace AxoPlotl
{

OVMVolumeMesh happly_to_openvolumemesh(happly::PLYData &_data)
{
    OVMVolumeMesh ovm;
    std::vector<std::array<double,3>> vPos = _data.getVertexPositions();
    std::vector<std::vector<uint32_t>> fInd = _data.getFaceIndices<uint32_t>();
    ovm.reserve_vertices(vPos.size());
    ovm.reserve_faces(fInd.size());
    for (const auto& p : vPos) {
        ovm.add_vertex(OVMVolumeMesh::Point(p[0],p[1],p[2]));
    }
    for (const auto& f : fInd) {
        std::vector<OVM::VH> vhs(f.size());
        for (int i = 0; i < f.size(); ++i) {
            vhs[i] = OVM::VH(f[i]);
        }
        ovm.add_face(vhs);
    }
    return ovm;
}

}
