#pragma once

#include <glm/detail/qualifier.hpp>
#include <type_traits>
#include <OpenVolumeMesh/Geometry/Vector11T.hh>
#include <glm/glm.hpp>

namespace AxoPlotl
{

template<typename T>
struct vector_traits : std::false_type {};

template<typename FT, int DIM>
struct vector_traits<glm::vec<DIM,FT>> : std::true_type {
    static constexpr int dim = DIM;
};

template<typename FT, int DIM>
struct vector_traits<OpenVolumeMesh::VectorT<FT,DIM>> : std::true_type {
    static constexpr int dim = DIM;
};

template<typename T>
inline constexpr bool is_vector = vector_traits<T>::value;

template<typename T>
inline constexpr int vector_dim = vector_traits<T>::dim;

template<typename T>
struct matrix_traits : std::false_type {};

template<
    glm::length_t Rows,
    glm::length_t Cols,
    typename Scalar
    >
struct matrix_traits<glm::mat<Cols,Rows,Scalar>> : std::true_type {
    static constexpr int cols = Cols;
    static constexpr int rows = Rows;
};

template<typename T>
inline constexpr bool is_matrix = matrix_traits<T>::value;

template<typename T, int Rows, int Cols>
inline constexpr bool has_rows_and_cols = (matrix_traits<T>::rows == Rows) && (matrix_traits<T>::cols == Cols);

}
