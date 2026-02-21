#include "file_access.h"
#include <ToLoG/utils/OVM_Traits.hpp>
#include "OpenVolumeMesh/FileManager/FileManager.hh"
#include "OpenVolumeMesh/IO/ovmb_read.hh"
#include <OpenVolumeMesh/FileManager/VtkColorReader.hh>

#include <ToLoG/io/obj_reader.hpp>
#include <ToLoG/io/ply_reader.hpp>
#include <ToLoG/io/medit_reader.hpp>

namespace AxoPlotl
{

std::optional<std::variant<SurfaceMesh,VolumeMesh>>
IO::read_mesh(const std::filesystem::path& _path)
{
    if (_path.extension() == ".obj") {
        SurfaceMesh mesh;
        if (ToLoG::IO::read_polygon_mesh_obj(_path, mesh)==0) {return mesh;}
    } else if (_path.extension() == ".ply") {
        SurfaceMesh mesh;
        if (ToLoG::IO::read_polygon_mesh_ply(_path, mesh)==0) {return mesh;}
    } else if (_path.extension() == ".mesh") {
        // VolumeMesh mesh;
        // if (ToLoG::IO::read_polyhedral_mesh_medit(_path, mesh)==0) {return mesh;}
    } else if (_path.extension() == ".ovmb") {
        VolumeMesh mesh;
        if (OVM::IO::ovmb_read(_path.c_str(), mesh)
            ==OVM::IO::ReadResult::Ok) {return mesh;}
    } else if (_path.extension() == ".ovm") {
        VolumeMesh mesh;
        OVM::IO::FileManager fm;
        if (fm.readFile(_path, mesh)) {return mesh;}
    } else if (_path.extension() == ".vtk") {
        VolumeMesh mesh;
        OVM::Reader::VtkColorReader fm;
        fm.readFile(_path, mesh, true, true);
    }
    return std::nullopt;
}

}
