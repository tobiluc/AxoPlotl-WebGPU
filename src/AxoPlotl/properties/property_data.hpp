#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/properties/property_filters.hpp>
#include <AxoPlotl/rendering/OpenVolumeMeshRenderer.hpp>
#include <AxoPlotl/typedefs/ToLoG.hpp>

namespace AxoPlotl
{

template<typename T, typename EntityTag>
std::pair<T,T> get_scalar_property_range(
    const OVMVolumeMesh& _mesh,
    const OpenVolumeMesh::PropertyPtr<T,EntityTag>& _prop)
{
    if (_mesh.n<EntityTag>()==0) {return {0,0};}
    if constexpr(std::is_same_v<T,bool>) {return {false,true};}
    else {
        auto h0 = OVM::handle_for_tag_t<EntityTag>(0);
        std::pair<T,T> r = {_prop[h0], _prop[h0]};
        for (auto h : _mesh.entities<EntityTag>()) {
            r.first = std::min(r.first, _prop[h]);
            r.second = std::max(r.second, _prop[h]);
        }
        return r;
    }
};

template<typename T>
static std::string value_to_string(const T& _val)
{
    if constexpr(std::is_same_v<T,bool>) {return _val? "True" : "False";}
    if constexpr(std::is_scalar_v<T>) {return std::string(_val);}
    if constexpr (ToLoG::vector_type<T>) {
        std::string s = "[";
        for (int i = 0; i < ToLoG::Traits<T>::dim-1; ++i) {
            s += std::string(_val[i]) + ", ";
        }
        s += std::string(_val[ToLoG::Traits<T>::dim-1]) + "]";
    }
    return "UNKNOWN";
}

/// Converts a generic value to a Vec4f to store in the Vertex Buffer as v_data.
template<typename T>
RendererBase::Property::Data vertex_buffer_property_data(const T& _val)
{
    using D = RendererBase::Property::Data;

    if constexpr(std::is_floating_point_v<T>) {
        if (std::isnan(_val)) {
            constexpr float nanf = std::numeric_limits<float>::signaling_NaN();
            return D{nanf,0,0,1};
        }
        else if (std::isinf(_val)) {
            constexpr float inff = std::numeric_limits<float>::infinity();
            return D{inff,0,0,1};
        }
        else {return D{_val,0,0,1};}
    } else if constexpr(std::is_integral_v<T>) {
        return D{_val,0,0,1};
    } else if constexpr (ToLoG::vector_type<T>) {
        if (ToLoG::Traits<T>::dim <= 4) {
            D v;
            for (int i = 0; i < ToLoG::Traits<T>::dim; ++i) {
                v[i] = _val[i];
            }
            return v;
        } else {
            std::cerr << "Vector Properties of Dimension > 4 are not supported" << std::endl;
        }
    }
    throw std::runtime_error("UNKNOWN PROPERTY DATA TYPE: "+std::string(typeid(T).name()));
    return D(0,0,0,0);
}

template<typename T>
constexpr RendererBase::Property::Mode vertex_buffer_property_mode()
{
    if constexpr(std::is_same_v<T,bool>
                  || std::is_same_v<T,int>
                  || std::is_same_v<T,float>
                  || std::is_same_v<T,double>)
    {return RendererBase::Property::Mode::SCALAR;}
    if constexpr(ToLoG::vector_type<T>) {
        if constexpr(ToLoG::Traits<T>::dim == 3) {
            return RendererBase::Property::Mode::VEC3;
        }
    }
    return RendererBase::Property::Mode::COLOR;
}

// template<typename T>
// void upload_vertex_property_data(
//     const OVMVolumeMesh& _mesh,
//     const OpenVolumeMesh::PropertyPtr<T,OpenVolumeMesh::Entity::Vertex>& _prop,
//     OpenVolumeMeshRenderer& _vol_rend)
// {
//     if (_mesh.n_vertices()==0) {return;}
//     std::vector<RendererBase::Property::Data> v_data;
//     for (auto vh : _mesh.vertices()) {
//         v_data.push_back(vertex_buffer_property_data(static_cast<T>(_prop[vh]))); // cast to avoid the vector<bool> issue
//     }
//     _vol_rend.vertices().update_property_data(v_data);
//     _vol_rend.vertices().property_mode() = vertex_buffer_property_mode<T>();
// };

// template<typename T>
// void upload_edge_property_data(
//     const OVMVolumeMesh& _mesh,
//     const OpenVolumeMesh::PropertyPtr<T,OpenVolumeMesh::Entity::Edge>& _prop,
//     OpenVolumeMeshRenderer& _vol_rend)
// {
//     if (_mesh.n_edges()==0) {return;}
//     std::vector<RendererBase::Property::Data> e_data;
//     for (auto eh : _mesh.edges()) {
//         e_data.push_back(vertex_buffer_property_data(static_cast<T>(_prop[eh])));
//     }
//     _vol_rend.edges().update_property_data(e_data);
//     _vol_rend.edges().property_mode() = vertex_buffer_property_mode<T>();
// };

// template<typename T>
// void upload_face_property_data(
//     const OVMVolumeMesh& _mesh,
//     const OpenVolumeMesh::PropertyPtr<T,OpenVolumeMesh::Entity::Face>& _prop,
//     OpenVolumeMeshRenderer& _vol_rend)
// {
//     if (_mesh.n<OVM::Entity::Face>()==0) {return;}
//     std::vector<RendererBase::Property::Data> f_data;
//     for (auto fh : _mesh.faces()) {
//         f_data.push_back(vertex_buffer_property_data(static_cast<T>(_prop[fh])));
//     }
//     _vol_rend.faces().update_property_data(f_data);
//     _vol_rend.faces().property_mode() = vertex_buffer_property_mode<T>();
// };

// template<typename T>
// void upload_cell_property_data(
//     const OVMVolumeMesh& _mesh,
//     const OpenVolumeMesh::PropertyPtr<T,OpenVolumeMesh::Entity::Cell>& _prop,
//     OpenVolumeMeshRenderer& _vol_rend)
// {
//     if (_mesh.n_cells()==0) {return;}
//     std::vector<RendererBase::Property::Data> c_data;
//     for (auto ch : _mesh.cells()) {
//         c_data.push_back(vertex_buffer_property_data(static_cast<T>(_prop[ch])));
//     }
//     _vol_rend.cells().update_property_data(c_data);
//     _vol_rend.cells().property_mode() = vertex_buffer_property_mode<T>();
// };

template<typename T, typename EntityTag>
std::vector<RendererBase::Property::Data> vertex_buffer_property_data(
    const OVMVolumeMesh& _mesh,
    OpenVolumeMesh::PropertyStorageBase* _prop
    )
{
    auto prop = _mesh.get_property<T,EntityTag>((_prop)->name()).value();
    std::vector<RendererBase::Property::Data> data;
    data.reserve(_mesh.n<EntityTag>());
    for (auto h : _mesh.entities<EntityTag>()) {
        data.push_back(vertex_buffer_property_data(static_cast<T>(prop[h])));
    }
    return data;
}

template<typename T, typename Entity>
void upload_property_data(
    const OVMVolumeMesh& _mesh,
    OpenVolumeMesh::PropertyStorageBase* _prop,
    std::vector<std::shared_ptr<PropertyFilterBase>>& _prop_filters,
    OpenVolumeMeshRenderer& _vol_rend
    )
{
    _prop_filters.clear();
    auto prop = _mesh.get_property<T,Entity>((_prop)->name()).value();

    // Setup Property Filters
    if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>)
    {
        // Compute the Scalar Range and assign it to the visible range
        auto hist = Histogram(_prop->cast_to_StorageT<T>());
        //std::cerr << hist << std::endl;

        //auto r = get_scalar_property_range<T,Entity>(_mesh, prop);
        get_property_value_filter<Entity>(_vol_rend) = {
            static_cast<float>(hist.min_), static_cast<float>(hist.max_)
        };

        // Create filters
        _prop_filters.push_back(std::make_shared<ScalarPropertyRangeFilter<T,Entity>>(hist));
        _prop_filters.push_back(std::make_shared<ScalarPropertyExactFilter<T,Entity>>(hist));
    }

    const auto& data = vertex_buffer_property_data<T,Entity>(
        _mesh, _prop);
    _vol_rend.entities<Entity>().update_property_data(data);
    _vol_rend.entities<Entity>().property_mode() = vertex_buffer_property_mode<T>();
    // _vol_rend.vertices().enabled() = false;
    // _vol_rend.edges().enabled() = false;
    // _vol_rend.faces().enabled() = false;
    // _vol_rend.cells().enabled() = false;
    _vol_rend.entities<Entity>().enabled() = true;
}

}
