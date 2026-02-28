#include "file_access.h"
#include <ToLoG/utils/OVM_Traits.hpp>
#include "OpenVolumeMesh/FileManager/FileManager.hh"
#include "OpenVolumeMesh/IO/ovmb_read.hh"
#include <OpenVolumeMesh/FileManager/VtkColorReader.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <rapidobj/rapidobj.hpp>
#include <ToLoG/io/obj_reader.hpp>
#include <ToLoG/io/ply_reader.hpp>
#include <ToLoG/io/medit_reader.hpp>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/Options.hh>

namespace AxoPlotl
{

std::optional<std::variant<SurfaceMesh,OVMVolumeMesh,OMSurfaceMesh>>
IO::read_mesh(const std::filesystem::path& _path)
{
    if (_path.extension() == ".obj") {
        const auto& obj = rapidobj::ParseFile(_path);
        SurfaceMesh mesh;
        if (ToLoG::IO::read_polygon_mesh_obj(_path, mesh)==0) {return mesh;}
    } else if (_path.extension() == ".ply") {
        SurfaceMesh mesh;
        if (ToLoG::IO::read_polygon_mesh_ply(_path, mesh)==0) {return mesh;}
    } else if (_path.extension() == ".mesh") {
        // VolumeMesh mesh;
        // if (ToLoG::IO::read_polyhedral_mesh_medit(_path, mesh)==0) {return mesh;}
    } else if (_path.extension() == ".ovmb") {
        OVMVolumeMesh mesh;
        if (OVM::IO::ovmb_read(_path.c_str(), mesh)
            ==OVM::IO::ReadResult::Ok) {return mesh;}
    } else if (_path.extension() == ".ovm") {
        OVMVolumeMesh mesh;
        OVM::IO::FileManager fm;
        if (fm.readFile(_path, mesh)) {return mesh;}
    } else if (_path.extension() == ".vtk") {
        OVMVolumeMesh mesh;
        OVM::Reader::VtkColorReader fm;
        fm.readFile(_path, mesh, true, true);
    } else {
        OMSurfaceMesh mesh;
        mesh.request_vertex_colors();
        mesh.request_vertex_texcoords2D();
        mesh.request_face_normals();
        OpenMesh::IO::Options opt(OpenMesh::IO::Options::VertexColor);
        if (OpenMesh::IO::read_mesh(mesh, _path, opt)) {
            return mesh;
        }
    }
    return std::nullopt;
}

}
