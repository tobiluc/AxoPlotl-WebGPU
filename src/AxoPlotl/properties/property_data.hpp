#pragma once

#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/properties/property_filters.hpp>
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
    else if constexpr(std::is_scalar_v<T>) {return std::to_string(_val);}
    else if constexpr (ToLoG::vector_type<T>) {
        std::string s = "[";
        for (int i = 0; i < ToLoG::Traits<T>::dim-1; ++i) {
            s += std::to_string(_val[i]) + ", ";
        }
        s += std::to_string(_val[ToLoG::Traits<T>::dim-1]) + "]";
        return s;
    }
    return "UNKNOWN";
}

/// Converts a generic value to a Vec4f to store in the Vertex Buffer as v_data.
template<typename T>
PropertyRendererBase::Property::Data get_buffer_property_data(const T& _val)
{
    using D = PropertyRendererBase::Property::Data;

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
constexpr PropertyRendererBase::Property::Type get_buffer_property_type()
{
    if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>) {
        return PropertyRendererBase::Property::Type::SCALAR;
    } else if constexpr(ToLoG::vector_type<T>) {
        if constexpr(ToLoG::Traits<T>::dim == 3) {
            return PropertyRendererBase::Property::Type::VEC3;
        }
    }
    return PropertyRendererBase::Property::Type::COLOR;
}

template<typename T, typename EntityTag>
std::vector<PropertyRendererBase::Property::Data> get_buffer_property_data(
    const OVMVolumeMesh& _mesh,
    OpenVolumeMesh::PropertyStorageBase* _prop
    )
{
    auto prop = _mesh.get_property<T,EntityTag>((_prop)->name()).value();
    std::vector<PropertyRendererBase::Property::Data> data;
    data.reserve(_mesh.n<EntityTag>());
    for (auto h : _mesh.entities<EntityTag>()) {
        data.push_back(get_buffer_property_data(static_cast<T>(prop[h])));
    }
    return data;
}

template<typename T, typename Entity, typename Renderer>
void upload_buffer_property_data(
    const OVMVolumeMesh& _mesh,
    OpenVolumeMesh::PropertyStorageBase* _prop,
    std::vector<std::shared_ptr<PropertyFilterForRenderer<Renderer>>>& _prop_filters,
    Renderer& _r
    )
{
    _prop_filters.clear();
    auto prop = _mesh.get_property<T,Entity>((_prop)->name()).value();

    // Setup Property Filters
    if constexpr(std::is_same_v<bool,T>) {
        _prop_filters.push_back(std::make_shared<PropertyFilterBool<Entity,Renderer>>(_prop->cast_to_StorageT<bool>()));
        _prop_filters.back()->init(_r);
    } else if constexpr(std::is_floating_point_v<T>) {
        _prop_filters.push_back(std::make_shared<PropertyFilterFloatRange<T,Entity,Renderer>>(_prop->cast_to_StorageT<T>()));
        _prop_filters.back()->init(_r);
    } else if constexpr(std::is_integral_v<T>) {
        _prop_filters.push_back(std::make_shared<PropertyFilterIntValue<T,Entity,Renderer>>(_prop->cast_to_StorageT<T>()));
        _prop_filters.back()->init(_r);
        _prop_filters.push_back(std::make_shared<PropertyFilterIntRange<T,Entity,Renderer>>(_prop->cast_to_StorageT<T>()));
        _prop_filters.back()->init(_r);
    }

    const auto& data = get_buffer_property_data<T,Entity>(
        _mesh, _prop);
    _r.update_property_data(data);
    _r.property_type() = get_buffer_property_type<T>();
    _r.enabled() = true;
}

}
