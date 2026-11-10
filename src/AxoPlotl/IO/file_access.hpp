#pragma once

#include <filesystem>
#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl::IO
{

enum class ReadMeshStatus {
    OK = 0,
    READ_ERROR = 1,
    MESH_CONTAINS_NAN = 2
};

struct ReadMeshResult
{
    OVMVolumeMesh mesh_;
    ReadMeshStatus status_;
};

ReadMeshResult read_mesh(
    const std::filesystem::path& _path);

}
