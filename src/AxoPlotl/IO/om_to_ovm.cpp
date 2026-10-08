#include <AxoPlotl/IO/om_to_ovm.hpp>
#include <ToLoG/mesh/cell_complex_iterators.hpp>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

namespace AxoPlotl::IO
{

void openmesh_to_openvolumemesh(const OpenMesh::PolyMesh_ArrayKernelT<OpenMesh::DefaultTraits>& _om, OVMVolumeMesh& _ovm)
{
    _ovm.clear();
    _ovm.reserve_vertices(_om.n_vertices());
    _ovm.reserve_edges(_om.n_edges());
    _ovm.reserve_faces(_om.n_faces());

    // Copy Vertices
    for (auto vh : _om.vertices()) {
        const auto& p = _om.point(vh);
        _ovm.add_vertex(OVMVolumeMesh::PointT(p[0],p[1],p[2]));
    }

    // Copy Edges
    for (auto eh : _om.edges()) {
        _ovm.add_edge(OVM::VH(eh.v0().idx()), OVM::VH(eh.v1().idx()));
    }

    // Copy Faces
    for (auto fh : _om.faces()) {
        std::vector<OVM::VH> vhs;
        vhs.reserve(fh.valence());
        for (auto vh : fh.vertices_ccw()) {
            vhs.emplace_back(vh.idx());
        }
        _ovm.add_face(vhs);
    }

    auto copy_vprop = [&]<typename T>(OpenMesh::BaseProperty* _p) {
        OpenMesh::VPropHandleT<T> p2;
        if (_p && _om.get_property_handle(p2, _p->name())) {
            auto p = _ovm.request_vertex_property<T>(_p->name());
            _ovm.set_persistent(p);
            for (auto vh : _om.vertices()) {
                p[OVM::VH(vh.idx())] = _om.property(p2, vh);
            }
        }
    };

    auto copy_eprop = [&]<typename T>(OpenMesh::BaseProperty* _p) {
        OpenMesh::EPropHandleT<T> p2;
        if (_p && _om.get_property_handle(p2, _p->name())) {
            auto p = _ovm.request_edge_property<T>(_p->name());
            _ovm.set_persistent(p);
            for (auto eh : _om.edges()) {
                p[OVM::EH(eh.idx())] = _om.property(p2, eh);
            }
        }
    };

    auto copy_fprop = [&]<typename T>(OpenMesh::BaseProperty* _p) {
        OpenMesh::FPropHandleT<T> p2;
        if (_p && _om.get_property_handle(p2, _p->name())) {
            auto p = _ovm.request_face_property<T>(_p->name());
            _ovm.set_persistent(p);
            for (auto fh : _om.faces()) {
                p[OVM::FH(fh.idx())] = _om.property(p2, fh);
            }
        }
    };

    auto copy_prop_t = [&]<typename EntityTag, typename T>(OpenMesh::BaseProperty* _p) {
        if constexpr(std::is_same_v<EntityTag,OVM::Entity::Vertex>) {copy_vprop.template operator()<T>(_p);}
        else if constexpr(std::is_same_v<EntityTag,OVM::Entity::Edge>) {copy_eprop.template operator()<T>(_p);}
        else if constexpr(std::is_same_v<EntityTag,OVM::Entity::Face>) {copy_fprop.template operator()<T>(_p);}
    };

    auto copy_prop = [&]<typename EntityTag>(OpenMesh::BaseProperty* _p) {
        if (_p->get_storage_name() == "double") {
            copy_prop_t.template operator()<EntityTag, double>(_p);
        } else if (_p->get_storage_name() == "bool") {
            copy_prop_t.template operator()<EntityTag, bool>(_p);
        } else if (_p->get_storage_name() == "int32_t") {
            copy_prop_t.template operator()<EntityTag, int32_t>(_p);
        }
    };

    for (auto vp = _om.vprops_begin(); vp != _om.vprops_end(); ++vp) {
        copy_prop.template operator()<OVM::Entity::Vertex>(*vp);
    }

    for (auto ep = _om.eprops_begin(); ep != _om.eprops_end(); ++ep) {
        copy_prop.template operator()<OVM::Entity::Edge>(*ep);
    }

    for (auto fp = _om.fprops_begin(); fp != _om.fprops_end(); ++fp) {
        copy_prop.template operator()<OVM::Entity::Face>(*fp);
    }

    auto om_tag = _ovm.request_mesh_property<bool>("OpenMeshTag", true);
    _ovm.set_persistent(om_tag);
}

}
