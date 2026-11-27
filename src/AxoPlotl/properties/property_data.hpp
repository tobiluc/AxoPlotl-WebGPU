#pragma once

#include "AxoPlotl/geometry/geometry_traits.hpp"
#include <AxoPlotl/rendering/renderers/PropertyRendererBase.hpp>
#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl::internal
{

// template<typename T, typename EntityTag>
// std::pair<T,T> get_scalar_property_range(
//     const VolMesh& _mesh,
//     const OpenVolumeMesh::PropertyPtr<T,EntityTag>& _prop)
// {
//     if (_mesh.n<EntityTag>()==0) {return {0,0};}
//     if constexpr(std::is_same_v<T,bool>) {return {false,true};}
//     else {
//         auto h0 = OVM::handle_for_tag_t<EntityTag>(0);
//         std::pair<T,T> r = {_prop[h0], _prop[h0]};
//         for (auto h : _mesh.entities<EntityTag>()) {
//             r.first = std::min(r.first, _prop[h]);
//             r.second = std::max(r.second, _prop[h]);
//         }
//         return r;
//     }
// };

template<typename T>
static std::string value_to_string(const T& _val)
{
    if constexpr(std::is_same_v<T,bool>) {return _val? "True" : "False";}
    else if constexpr(std::is_scalar_v<T>) {return std::to_string(_val);}
    else if constexpr (is_vector<T>) {
        std::string s = "[";
        for (int i = 0; i < vector_dim<T>-1; ++i) {
            s += std::to_string(_val[i]) + ", ";
        }
        s += std::to_string(_val[vector_dim<T>-1]) + "]";
        return s;
    } else if constexpr(is_matrix<T>) {
        std::string s = "[";
        for (int c = 0; c < matrix_traits<T>::cols; ++c) {
            s += "[";
            for (int r = 0; r < matrix_traits<T>::rows; ++r) {
                s += std::to_string(_val[c][r]);
                if (r < matrix_traits<T>::rows-1) {s += ", ";}
            }
            s += "]";
            if (c < matrix_traits<T>::cols-1) {s += ", ";}
        }
        s += "]";
        return s;
    }
    return "UNKNOWN";
}

/// Converts a generic value to a Vec4f to store in the Vertex Buffer as v_data.
template<typename T>
std::optional<PropContainer> get_buffer_property_data(const T& _val)
{
    if constexpr(std::is_floating_point_v<T>) {
        if (std::isnan(_val)) {
            constexpr float nanf = std::numeric_limits<float>::signaling_NaN();
            return nanf;
        }
        else if (std::isinf(_val)) {
            constexpr float inff = std::numeric_limits<float>::infinity();
            return inff;
        }
        else {return _val;}
    } else if constexpr(std::is_integral_v<T>) {
        return _val;
    }

    std::cerr << "UNKNOWN PROPERTY DATA TYPE FOR FLOAT CONVERSION: "
              << std::string(typeid(T).name()) << std::endl;
    return std::nullopt;
}

template<typename T>
constexpr PropValueMode get_buffer_property_type()
{
    if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>) {
        return PropValueMode::SCALAR;
    } else if constexpr(is_vector<T>) {
        if constexpr(vector_dim<T> == 3) {
            return PropValueMode::VEC3;
        }
    } else if constexpr(is_matrix<T>) {
        if constexpr(has_rows_and_cols<T,3,3>) {
            return PropValueMode::MAT3x3;
        }
    }
    return PropValueMode::NOPROP;
}

template<typename T, typename EntityTag>
std::vector<PropContainer> get_buffer_property_data(
    const VolMesh& _mesh,
    OpenVolumeMesh::PropertyStorageBase* _prop
    )
{
    auto prop = _mesh.get_property<T,EntityTag>((_prop)->name()).value();
    std::vector<PropContainer> data;
    data.reserve(_mesh.n<EntityTag>());
    for (auto h : _mesh.entities<EntityTag>()) {
        auto d = get_buffer_property_data(static_cast<T>(prop[h]));
        if (d.has_value()) {data.push_back(*d);
        } else {data.emplace_back(0);}
    }
    return data;
}

template<typename T, typename EntityTag>
    requires(is_matrix<T> && has_rows_and_cols<T,3,3>)
std::vector<glm::mat3x4> get_buffer_property_data_mat3x4(
    const VolMesh& _mesh,
    OpenVolumeMesh::PropertyStorageBase* _prop)
{
    auto prop = _mesh.get_property<T,EntityTag>((_prop)->name()).value();
    std::vector<glm::mat3x4> data;
    data.reserve(_mesh.n<EntityTag>());
    for (auto h : _mesh.entities<EntityTag>()) {
        const auto& m3x3 = prop[h];
        data.emplace_back();
        for (int c = 0; c < 3; ++c) {
            for (int r = 0; r < 3; ++r) {
                data.back()[c][r] = static_cast<float>(m3x3[c][r]);
            }
            data.back()[c][3] = 1.0f;
        }
    }
    return data;
}

template<typename T, typename EntityTag>
requires(is_vector<T> && vector_dim<T> <= 4)
std::vector<Vec4> get_buffer_property_data_vec4(
    const VolMesh& _mesh,
    OpenVolumeMesh::PropertyStorageBase* _prop)
{
    auto prop = _mesh.get_property<T,EntityTag>((_prop)->name()).value();
    std::vector<Vec4> data;
    data.reserve(_mesh.n<EntityTag>());
    for (auto h : _mesh.entities<EntityTag>()) {
        const auto& v = prop[h];
        data.emplace_back();
        for (int i = 0; i < vector_dim<T>; ++i) {
            data.back()[i] = v[i];
        }
        for (int i = vector_dim<T>; i < 4; ++i) {
            data.back()[i] = 0.0f;
        }
    }
    return data;
}


}
