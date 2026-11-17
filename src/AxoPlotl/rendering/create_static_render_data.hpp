#pragma once

#include "AxoPlotl/rendering/wgpu_commons.hpp"
#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>

namespace AxoPlotl::internal
{

struct VolMeshData
{
public:
    using Index = uint32_t;
    using Position = RendererBase::Position; // Ensure 16 byte alignment
    using Normal = Position;
    // using Edge = std::pair<Index,Index>;
    // using Face = std::vector<Index>;
    // using Cell = std::vector<Face>;

    struct EdgeInstance {
        uint32_t vh0; // used to lookup position
        uint32_t vh1; // used to lookup position
        uint32_t eh; // points to EdgePropertyData
    };

    struct FaceIndex {
        uint32_t vh;
        uint32_t fh;
    };

    struct CellIndex {
        uint32_t vh;
        uint32_t ch;
        uint32_t hfh;
    };

    template<typename T>
    struct BufferData {
        wgpu::Buffer buffer;
        std::vector<T> data;
        ~BufferData() {destroy();}
        void create_buffer(const wgpu::Device& _device) {
            destroy_buffer(buffer);
            buffer = create_bufferT<T>(_device, data);
        }
        void destroy() {
            destroy_buffer(buffer);
            data.clear();
        }
    };

    VolMeshData() {}

    VolMeshData(OVMVolumeMesh&& _mesh) : mesh(std::move(_mesh))
    {};

    VolMeshData(const VolMeshData&) = delete;
    VolMeshData& operator=(const VolMeshData&) = delete;

    Position position(OVM::VH _vh) const {
        const auto& p = mesh.point(_vh);
        return Position(p[0], p[1], p[2], 1);
    }

    void create_buffers(const wgpu::Device& _device)
    {
        // Positions
        if (mesh.n_vertices()) {
            vertices_positions.data.resize(mesh.n_vertices());
            for (OVM::VH vh : mesh.vertices()) {
                vertices_positions.data[vh.idx()] = position(vh);
            }
            vertices_positions.create_buffer(_device);
        }

        // Edges Barycenters
        if (mesh.n_edges()) {
            edges_centers.data.resize(mesh.n_edges());
            for (OVM::EH eh : mesh.edges()) {
                const auto& p = mesh.barycenter(eh);
                edges_centers.data[eh.idx()] = {p[0], p[1], p[2], 1};
            }
            edges_centers.create_buffer(_device);
        }

        // Faces Barycenters
        if (mesh.n_faces()) {
            faces_centers.data.resize(mesh.n_faces());
            for (OVM::FH fh : mesh.faces()) {
                const auto& p = mesh.barycenter(fh);
                faces_centers.data[fh.idx()] = {p[0], p[1], p[2], 1};
            }
            faces_centers.create_buffer(_device);
        }

        // Cells Barycenters
        if (mesh.n_cells()) {
            cells_centers.data.resize(mesh.n_cells());
            for (OVM::CH ch : mesh.cells()) {
                const auto& p = mesh.barycenter(ch);
                cells_centers.data[ch.idx()] = {p[0], p[1], p[2], 1};
            }
            cells_centers.create_buffer(_device);
        }

        // HalfFaces Normals
        if (mesh.n_faces()) {
            halffaces_normals.data.resize(mesh.n_halffaces());
            for (OVM::HFH hfh : mesh.halffaces()) {
                const auto& n = mesh.normal(hfh);
                halffaces_normals.data[hfh.idx()] = {n[0], n[1], n[2], 0};
            }
            halffaces_normals.create_buffer(_device);
        }

        // Edge Instances
        if (mesh.n_edges()) {
            edges_instances.data.resize(mesh.n_edges());
            for (OVM::EH eh : mesh.edges()) {
                OVM::VH vh0 = mesh.from_vertex_handle(eh.halfedge_handle(0));
                OVM::VH vh1 = mesh.to_vertex_handle(eh.halfedge_handle(0));
                edges_instances.data[eh.idx()] = EdgeInstance{
                    .vh0 = vh0.uidx(),
                    .vh1 = vh1.uidx(),
                    .eh = eh.uidx()
                };
            }
            edges_instances.create_buffer(_device);
        }

        // Face Indices - Triangulated
        if (mesh.n_faces()) {
            faces_indices_.data.clear();
            faces_indices_.data.reserve(3*mesh.n_faces());
            for (OVM::FH fh : mesh.faces()) {
                std::vector<OVM::VH> vhs = mesh.get_halfface_vertices(fh.halfface_handle(0));
                for (int i = 1; i < vhs.size()-1; ++i) {
                    faces_indices_.data.push_back({.vh = vhs[0].uidx(), .fh = fh.uidx()});
                    faces_indices_.data.push_back({.vh = vhs[i].uidx(), .fh = fh.uidx()});
                    faces_indices_.data.push_back({.vh = vhs[i+1].uidx(), .fh = fh.uidx()});
                }
            }
            faces_indices_.create_buffer(_device);
        }

        // Cell Indices
        if (mesh.n_cells()) {
            cells_indices_.data.clear();
            cells_indices_.data.reserve(12*mesh.n_cells());
            for (OVM::CH ch : mesh.cells()) {
                for (OVM::HFH hfh : mesh.cell_halffaces(ch)) {
                    std::vector<OVM::VH> vhs = mesh.get_halfface_vertices(hfh);
                    for (int i = 1; i < vhs.size()-1; ++i) {
                        cells_indices_.data.push_back({.vh = vhs[0].uidx(), .ch=ch.uidx(), .hfh = hfh.uidx()});
                        cells_indices_.data.push_back({.vh = vhs[i].uidx(), .ch=ch.uidx(), .hfh = hfh.uidx()});
                        cells_indices_.data.push_back({.vh = vhs[i+1].uidx(), .ch=ch.uidx(), .hfh = hfh.uidx()});
                    }
                }
            }
            cells_indices_.create_buffer(_device);
        }
    }

    OVMVolumeMesh mesh;
    BufferData<Vec4> vertices_positions;
    BufferData<Vec4> edges_centers;
    BufferData<Vec4> faces_centers;
    BufferData<Vec4> cells_centers;
    BufferData<Vec4> halffaces_normals;
    BufferData<EdgeInstance> edges_instances;
    BufferData<FaceIndex> faces_indices_;
    BufferData<CellIndex> cells_indices_;
};

// VolMeshData create_static_render_data(const OVMVolumeMesh& _mesh);



}
