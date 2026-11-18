#include <AxoPlotl/properties/property_calculations.hpp>

namespace AxoPlotl::internal
{

void add_face_normal_prop(VolMesh& _mesh, std::string _name)
{
    auto f_normal = _mesh.request_face_property<OVM::Vec3f>(_name);
    _mesh.set_persistent(f_normal);
    for (auto fh : _mesh.faces()) {
        f_normal[fh] = _mesh.normal(fh.halfface_handle(0));
    }
}

}
