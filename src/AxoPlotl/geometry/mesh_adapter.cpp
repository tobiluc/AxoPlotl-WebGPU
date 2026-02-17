// #include "mesh_adapter.h"

// #include "OpenVolumeMesh/IO/ovmb_write.hh"
// #include "OpenVolumeMesh/IO/PropertyCodecs.hh"
// #include "OpenVolumeMesh/IO/ovmb_read.hh"

// bool AxoPlotl::IO::loadMeshFromFile(const std::filesystem::path& path, PolyMesh& mesh)
// {
//     auto codecs = OVM::IO::g_default_property_codecs;
//     //OVM::IO::register_eigen_codecs(codecs);
//     auto res = OVM::IO::ovmb_read("/Users/tobiaskohler/Uni/HexHex/dataset/tet-ovmb/s17c_sphere_new_hex_igm.ovmb", mesh, OVM::IO::ReadOptions(), codecs);
//     if (res != OVM::IO::ReadResult::Ok) {std::cerr << OVM::IO::to_string(res) << std::endl; return false;}
//     return true;
// }
