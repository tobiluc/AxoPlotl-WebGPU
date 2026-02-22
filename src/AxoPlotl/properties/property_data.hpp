#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/properties/property_filters.hpp>
#include <AxoPlotl/rendering/VolumeMeshRenderer.hpp>
#include <AxoPlotl/typedefs/ToLoG.hpp>

namespace AxoPlotl
{

template<typename T, typename Entity>
std::pair<T,T> get_scalar_property_range(
    const VolumeMesh& _mesh,
    const OpenVolumeMesh::PropertyPtr<T,Entity>& _prop)
{
    if (mesh_n_entities<EntityType<Entity>::type()>(_mesh)==0) {return {0,0};}
    if constexpr(std::is_same_v<T,bool>) {return {false,true};}
    else {
        auto h0 = mesh_entity_handle<EntityType<Entity>::type()>(0);
        std::pair<T,T> r = {_prop[h0], _prop[h0]};
        for (auto h : mesh_entities<EntityType<Entity>::type()>(_mesh)) {
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
Vec4f vertex_buffer_property_data(const T& _val)
{
    if constexpr(std::is_same_v<T,bool>) {return {_val,0,0,1};}
    if constexpr(std::is_same_v<T,int>) {return {_val,0,0,1};}
    if constexpr(std::is_same_v<T,double>) {return {_val,0,0,1};}
    if constexpr(std::is_same_v<T,float>) {return {_val,0,0,1};}
    if constexpr (ToLoG::vector_type<T>) {
        if (ToLoG::Traits<T>::dim <= 4) {
            Vec4f v;
            for (int i = 0; i < ToLoG::Traits<T>::dim; ++i) {
                v[i] = _val[i];
            }
            return v;
        } else {
            std::cerr << "Vector Properties of Dimension > 4 are not supported" << std::endl;
        }
    }
    throw std::runtime_error("UNKNOWN PROPERTY DATA TYPE: "+std::string(typeid(T).name()));
    return Vec4f(0,0,0,0);
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

template<typename T>
void upload_vertex_property_data(
    const VolumeMesh& _mesh,
    const OpenVolumeMesh::PropertyPtr<T,OpenVolumeMesh::Entity::Vertex>& _prop,
    VolumeMeshRenderer& _vol_rend)
{
    if (_mesh.n_vertices()==0) {return;}
    std::vector<RendererBase::Property::Data> v_data;
    for (auto vh : _mesh.vertices()) {
        Vec4f a = vertex_buffer_property_data(static_cast<T>(_prop[vh])); // cast to avoid the vector<bool> issue
        v_data.push_back({.value_ = a});
    }
    _vol_rend.vertices().update_property_data(v_data);
    _vol_rend.vertices().property_mode() = vertex_buffer_property_mode<T>();
};

template<typename T>
void upload_edge_property_data(
    const VolumeMesh& _mesh,
    const OpenVolumeMesh::PropertyPtr<T,OpenVolumeMesh::Entity::Edge>& _prop,
    VolumeMeshRenderer& _vol_rend)
{
    if (_mesh.n_edges()==0) {return;}
    std::vector<RendererBase::Property::Data> e_data;
    for (auto eh : _mesh.edges()) {
        Vec4f a = vertex_buffer_property_data(static_cast<T>(_prop[eh]));
        e_data.push_back({.value_ = a});
    }
    _vol_rend.edges().update_property_data(e_data);
    _vol_rend.edges().property_mode() = vertex_buffer_property_mode<T>();
};

template<typename T>
void upload_face_property_data(
    const VolumeMesh& _mesh,
    const OpenVolumeMesh::PropertyPtr<T,OpenVolumeMesh::Entity::Face>& _prop,
    VolumeMeshRenderer& _vol_rend)
{
    if (_mesh.n_faces()==0) {return;}
    std::vector<RendererBase::Property::Data> f_data;
    for (auto fh : _mesh.faces()) {
        Vec4f a = vertex_buffer_property_data(static_cast<T>(_prop[fh]));
        f_data.push_back({.value_ = a});
    }
    _vol_rend.faces().update_property_data(f_data);
    _vol_rend.faces().property_mode() = vertex_buffer_property_mode<T>();
};

template<typename T>
void upload_cell_property_data(
    const VolumeMesh& _mesh,
    const OpenVolumeMesh::PropertyPtr<T,OpenVolumeMesh::Entity::Cell>& _prop,
    VolumeMeshRenderer& _vol_rend)
{
    if (_mesh.n_cells()==0) {return;}
    std::vector<RendererBase::Property::Data> c_data;
    for (auto ch : _mesh.cells()) {
        Vec4f a = vertex_buffer_property_data(static_cast<T>(_prop[ch]));
        c_data.push_back({.value_ = a});
    }
    _vol_rend.cells().update_property_data(c_data);
    _vol_rend.cells().property_mode() = vertex_buffer_property_mode<T>();
};

template<typename T, typename Entity>
void upload_property_data(
    const VolumeMesh& _mesh,
    OpenVolumeMesh::PropertyStorageBase* _prop,
    std::vector<std::shared_ptr<PropertyFilterBase>>& _prop_filters,
    VolumeMeshRenderer& _vol_rend
    )
{
    _prop_filters.clear();
    auto prop = _mesh.get_property<T,Entity>((_prop)->name()).value();

    // Setup Property Filters
    if constexpr(std::is_same_v<T,bool>
            || std::is_same_v<T,int>
            || std::is_same_v<T,float>
            || std::is_same_v<T,double>)
    {
        // Compute the Scalar Range and assign it to the visible range
        auto r = get_scalar_property_range<T,Entity>(_mesh, prop);
        get_property_value_filter<Entity>(_vol_rend) = {
            static_cast<float>(r.first), static_cast<float>(r.second)
        };

        // Create filters
        _prop_filters.push_back(std::make_shared<ScalarPropertyRangeFilter<T,Entity>>(r.first, r.second));
        _prop_filters.push_back(std::make_shared<ScalarPropertyExactFilter<T,Entity>>(r.first, r.second));
    }

    // Upload Properties
    if constexpr(std::is_same_v<Entity,OVM::Entity::Vertex>) {
        upload_vertex_property_data<T>(_mesh, prop, _vol_rend);
        _vol_rend.vertices().enabled() = true;
        _vol_rend.edges().enabled() =
        _vol_rend.faces().enabled() =
        _vol_rend.cells().enabled() = false;
    }
    if constexpr(std::is_same_v<Entity,OVM::Entity::Edge>) {
        upload_edge_property_data<T>(_mesh, prop, _vol_rend);
        _vol_rend.edges().enabled() = true;
        _vol_rend.vertices().enabled() =
        _vol_rend.faces().enabled() =
        _vol_rend.cells().enabled() = false;
    }
    if constexpr(std::is_same_v<Entity,OVM::Entity::Face>) {
        upload_face_property_data<T>(_mesh, prop, _vol_rend);
        _vol_rend.faces().enabled() = true;
        _vol_rend.edges().enabled() =
        _vol_rend.vertices().enabled() =
        _vol_rend.cells().enabled() = false;
    }
    if constexpr(std::is_same_v<Entity,OVM::Entity::Cell>) {
        upload_cell_property_data<T>(_mesh, prop, _vol_rend);
        _vol_rend.cells().enabled() = true;
        _vol_rend.edges().enabled() =
        _vol_rend.faces().enabled() =
        _vol_rend.vertices().enabled() = false;
    }
}

}
