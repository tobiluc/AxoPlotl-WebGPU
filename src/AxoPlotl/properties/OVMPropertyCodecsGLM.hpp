#pragma once

#include <OpenVolumeMesh/IO/PropertyCodecs.hh>
#include <OpenVolumeMesh/IO/PropertyCodecsT_impl.hh>
#include <OpenVolumeMesh/IO/detail/Encoder.hh>
#include <OpenVolumeMesh/IO/detail/Decoder.hh>

#include <glm/glm.hpp>

namespace OpenVolumeMesh
{
    template<> const std::string typeName<glm::mat<3,3,float>>() {return "3x3f";}
    template<> const std::string typeName<glm::mat<3,3,double>>() {return "3x3d";}
}

namespace OpenVolumeMesh::IO::Codecs {

template<
    glm::length_t Rows,
    glm::length_t Cols,
    typename Scalar,
    glm::qualifier Q = glm::defaultp
    >
struct GLMDenseFixedMatrix
{
    // GLM uses (columns, rows) as template parameters.
    using T = glm::mat<Cols, Rows, Scalar, Q>;

    static void encode(detail::Encoder& enc, const T& val)
    {
        for (size_t r = 0; r < Rows; ++r) {
            for (size_t c = 0; c < Cols; ++c) {
                enc.write(val[c][r]);
            }
        }
    }

    static void decode(detail::Decoder& reader, T& val)
    {
        for (size_t r = 0; r < Rows; ++r) {
            for (size_t c = 0; c < Cols; ++c) {
                reader.read(val[c][r]);
            }
        }
    }
};

} // namespace OpenVolumeMesh::IO::Codecs


namespace OpenVolumeMesh::IO {

inline void register_glm_codecs(PropertyCodecs& codecs)
{
    using namespace Codecs;

    // // 2x2 matrices
    // codecs.register_codec<
    //     SimplePropCodec<GLMDenseFixedMatrix<2, 2, float>>
    //     >("2x2f");

    // codecs.register_codec<
    //     SimplePropCodec<GLMDenseFixedMatrix<2, 2, double>>
    //     >("2x2d");

    // 3x3 matrices
    // codecs.register_codec<
    //     SimplePropCodec<GLMDenseFixedMatrix<3, 3, float>>
    //     >("3x3f");

    codecs.register_codec<
        SimplePropCodec<GLMDenseFixedMatrix<3, 3, double>>
        >("3x3d");

    // // 4x4 matrices
    // codecs.register_codec<
    //     SimplePropCodec<GLMDenseFixedMatrix<4, 4, float>>
    //     >("4x4f");

    // codecs.register_codec<
    //     SimplePropCodec<GLMDenseFixedMatrix<4, 4, double>>
    //     >("4x4d");
}

} // namespace OpenVolumeMesh::IO
