#include "MeshData.hpp"

namespace AxoPlotl::internal
{

void VolMeshData::create_buffers(const wgpu::Device& _device)
{
    // Positions
    if (mesh.n_vertices()) {
        vertices_positions.data.resize(mesh.n_vertices());
        for (OVM::VH vh : mesh.vertices()) {
            vertices_positions.data[vh.idx()] = position(vh);
        }
        vertices_positions.create_and_write(_device);
    }

    // Edges Barycenters
    if (mesh.n_edges()) {
        edges_centers.data.resize(mesh.n_edges());
        for (OVM::EH eh : mesh.edges()) {
            const auto& p = mesh.barycenter(eh);
            edges_centers.data[eh.idx()] = {p[0], p[1], p[2], 1};
        }
        edges_centers.create_and_write(_device);
    }

    // Faces Barycenters
    if (mesh.n_faces()) {
        faces_centers.data.resize(mesh.n_faces());
        for (OVM::FH fh : mesh.faces()) {
            const auto& p = mesh.barycenter(fh);
            faces_centers.data[fh.idx()] = {p[0], p[1], p[2], 1};
        }
        faces_centers.create_and_write(_device);
    }

    // Cells Barycenters
    if (mesh.n_cells()) {
        cells_centers.data.resize(mesh.n_cells());
        for (OVM::CH ch : mesh.cells()) {
            const auto& p = mesh.barycenter(ch);
            cells_centers.data[ch.idx()] = {p[0], p[1], p[2], 1};
        }
        cells_centers.create_and_write(_device);
    }

    // HalfFaces Normals
    if (mesh.n_faces()) {
        halffaces_normals.data.resize(mesh.n_halffaces());
        for (OVM::HFH hfh : mesh.halffaces()) {
            const auto& n = mesh.normal(hfh);
            halffaces_normals.data[hfh.idx()] = {n[0], n[1], n[2], 0};
        }
        halffaces_normals.create_and_write(_device);
    }

    // Halfedge Instances
    if (mesh.n_halfedges()) {
        halfedges_instances.data.resize(mesh.n_halfedges());
        for (OVM::HEH heh : mesh.halfedges()) {
            OVM::VH vh0 = mesh.from_vertex_handle(heh);
            OVM::VH vh1 = mesh.to_vertex_handle(heh);
            halfedges_instances.data[heh.idx()] = HalfEdgeInstance{
                .vh0 = vh0.uidx(),
                .vh1 = vh1.uidx(),
                .heh = heh.uidx()
            };
        }
        halfedges_instances.create_and_write(_device);
    }

    // HalfFace Indices - Triangulated
    if (mesh.n_faces()) {
        halffaces_indices.data.clear();
        halffaces_indices.data.reserve(6*mesh.n_faces());
        for (OVM::HFH hfh : mesh.halffaces()) {
            std::vector<OVM::VH> vhs = mesh.get_halfface_vertices(hfh);
            for (int i = 1; i < vhs.size()-1; ++i) {
                halffaces_indices.data.push_back({.vh = vhs[0].uidx(), .hfh = hfh.uidx()});
                halffaces_indices.data.push_back({.vh = vhs[i].uidx(), .hfh = hfh.uidx()});
                halffaces_indices.data.push_back({.vh = vhs[i+1].uidx(), .hfh = hfh.uidx()});
            }
        }
        halffaces_indices.create_and_write(_device);
    }

    // Cell Indices
    if (mesh.n_cells()) {
        cells_indices.data.clear();
        cells_indices.data.reserve(12*mesh.n_cells());
        for (OVM::CH ch : mesh.cells()) {
            for (OVM::HFH hfh : mesh.cell_halffaces(ch)) {
                std::vector<OVM::VH> vhs = mesh.get_halfface_vertices(hfh);
                for (int i = 1; i < vhs.size()-1; ++i) {
                    cells_indices.data.push_back({.vh = vhs[0].uidx(), .ch=ch.uidx(), .hfh = hfh.uidx()});
                    cells_indices.data.push_back({.vh = vhs[i].uidx(), .ch=ch.uidx(), .hfh = hfh.uidx()});
                    cells_indices.data.push_back({.vh = vhs[i+1].uidx(), .ch=ch.uidx(), .hfh = hfh.uidx()});
                }
            }
        }
        cells_indices.create_and_write(_device);
    }
}

}
