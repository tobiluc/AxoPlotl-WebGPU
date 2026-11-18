#pragma once

#include <filesystem>
#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl::IO
{

enum class MeshFormat {
    UNKNOWN,
    OVM,
    OM,
    OBJ,
    PLY,
    VTK
};

enum class ReadMeshStatus {
    OK = 0,
    READ_ERROR = 1,
    MESH_CONTAINS_NAN = 2
};

struct ReadMeshResult
{
    VolMesh mesh;
    ReadMeshStatus status;
    MeshFormat format;
};

ReadMeshResult read_mesh(
    const std::filesystem::path& _path);

}
