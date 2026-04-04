#pragma once

#include <glm/detail/qualifier.hpp>
#include <type_traits>
#include <OpenVolumeMesh/Geometry/Vector11T.hh>

namespace AxoPlotl
{

template<typename T>
struct vector_traits : std::false_type {};

template<typename FT, int DIM>
struct vector_traits<glm::vec<DIM,FT>> : std::true_type
{
    static constexpr int dim = DIM;
};

template<typename FT, int DIM>
struct vector_traits<OpenVolumeMesh::VectorT<FT,DIM>> : std::true_type
{
    static constexpr int dim = DIM;
};

template<typename T>
inline constexpr bool is_vector = vector_traits<T>::value;

template<typename T>
inline constexpr int vector_dim = vector_traits<T>::dim;

}
