#include "file_access.h"
#include <ToLoG/utils/OVM_Traits.hpp>
#include "AxoPlotl/IO/om_to_ovm.hpp"
#include "AxoPlotl/IO/rapidobj_to_ovm.hpp"
#include "AxoPlotl/typedefs/om.hpp"
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
#include <AxoPlotl/IO/happly_to_ovm.hpp>

namespace AxoPlotl
{

IO::ReadMeshResult IO::read_mesh(const std::filesystem::path& _path)
{
    IO::ReadMeshResult res;
    res.status_ = ReadMeshStatus::READ_ERROR;

    if (_path.extension() == ".obj") {
        const auto& robj = rapidobj::ParseFile(_path);
        if (!robj.error) {
            rapidobj_to_openvolumemesh(robj, res.mesh_);
            res.status_ = ReadMeshStatus::OK;
        }
    } else if (_path.extension() == ".ply") {
        try {
            happly::PLYData plyIn(_path);
            happly_to_openvolumemesh(plyIn, res.mesh_);
            res.status_ = ReadMeshStatus::OK;
        } catch (std::runtime_error& _err) {
            std::cerr << _err.what() << std::endl;
        }
    } else if (_path.extension() == ".ovmb") {
        if (OVM::IO::ovmb_read(_path.c_str(), res.mesh_)
            ==OVM::IO::ReadResult::Ok) {
            res.status_ = ReadMeshStatus::OK;
        }
    } else if (_path.extension() == ".ovm") {
        OVM::IO::FileManager fm;
        if (fm.readFile(_path, res.mesh_)) {
            res.status_ = ReadMeshStatus::OK;
        }
    } else if (_path.extension() == ".vtk") {
        OVM::Reader::VtkColorReader fm;
        if (fm.readFile(_path, res.mesh_, true, true)) {
            res.status_ = ReadMeshStatus::OK;
        }
    } else {
        OMSurfaceMesh mesh;
        mesh.request_vertex_colors();
        mesh.request_vertex_texcoords2D();
        mesh.request_face_normals();
        OpenMesh::IO::Options opt(OpenMesh::IO::Options::VertexColor);
        if (OpenMesh::IO::read_mesh(mesh, _path, opt)) {
            openmesh_to_openvolumemesh(mesh, res.mesh_);
            res.status_ = ReadMeshStatus::OK;
        }
    }
    return res;
}

}
