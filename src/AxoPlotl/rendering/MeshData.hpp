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

    struct HalfEdgeInstance {
        uint32_t vh0; // used to lookup position
        uint32_t vh1; // used to lookup position
        uint32_t heh; // points to HalfEdgePropertyData
    };

    struct HalfFaceIndex {
        uint32_t vh;
        uint32_t hfh;
    };

    struct CellIndex {
        uint32_t vh;
        uint32_t ch;
        uint32_t hfh;
    };

    VolMeshData() {}

    VolMeshData(VolMesh&& _mesh) : mesh(std::move(_mesh))
    {};

    VolMeshData(const VolMeshData&) = delete;
    VolMeshData& operator=(const VolMeshData&) = delete;

    Position position(OVM::VH _vh) const {
        const auto& p = mesh.point(_vh);
        return Position(p[0], p[1], p[2], 1);
    }

    void create_buffers(const wgpu::Device& _device);

    VolMesh mesh;
    BufferData<Vec4> vertices_positions;
    BufferData<Vec4> edges_centers;
    BufferData<Vec4> faces_centers;
    BufferData<Vec4> cells_centers;
    BufferData<Vec4> halffaces_normals;
    BufferData<HalfEdgeInstance> halfedges_instances;
    BufferData<HalfFaceIndex> halffaces_indices;
    BufferData<CellIndex> cells_indices;
};

}
