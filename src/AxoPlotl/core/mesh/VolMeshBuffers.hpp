#pragma once

#include "AxoPlotl/rendering/wgpu_commons.hpp"
#include <AxoPlotl/typedefs/ovm.hpp>
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>

namespace AxoPlotl::internal
{

struct VolMeshBuffers
{
public:
    using Index = uint32_t;
    using Position = RendererBase::Position; // Ensure 16 byte alignment
    using Normal = Position;

    enum Flag : uint32_t
    {
        NONE = 0,
        INIT_EDGE_BUFFERS = 1<<0,
        INIT_NORMAL_BUFFER = 1<<1,
        INIT_FACE_BUFFERS = 1<<2 | INIT_NORMAL_BUFFER,
        INIT_CELL_BUFFERS = 1<<3 | INIT_NORMAL_BUFFER,
        SPLIT_EDGES_IN_HALF = 1<<4 | INIT_EDGE_BUFFERS,
        SPLIT_FACES_IN_HALF = 1<<5 | INIT_FACE_BUFFERS
    };
    using Flags = uint32_t;
    static constexpr Flags default_flags = UINT32_MAX;

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
private:
    bool initialized_ = false;
    Flags flags_;

public:
    VolMeshBuffers() {}

    VolMeshBuffers(VolMesh&& _mesh) : mesh(std::move(_mesh))
    {};

    VolMeshBuffers(const VolMeshBuffers&) = delete;
    VolMeshBuffers& operator=(const VolMeshBuffers&) = delete;

    bool is_initialized() const {
        return initialized_;
    }

    Position position(OVM::VH _vh) const {
        const auto& p = mesh.point(_vh);
        return Position(p[0], p[1], p[2], 1);
    }

    void create_buffers(
        const wgpu::Device& _device,
        const Flags& _flags = default_flags);

    VolMesh mesh;
    BufferData<Position> vertices_positions;
    BufferData<Position> edges_centers;
    BufferData<Position> faces_centers;
    BufferData<Position> cells_centers;
    BufferData<Normal> halffaces_normals;
    BufferData<HalfEdgeInstance> halfedges_instances;
    BufferData<HalfFaceIndex> halffaces_indices;
    BufferData<CellIndex> cells_indices;

    template<typename EntityTag>
    const wgpu::Buffer& entity_center_buffer() const;
    template<> const wgpu::Buffer& entity_center_buffer<OVM::Entity::Cell>() const {return cells_centers.buffer;}
    template<> const wgpu::Buffer& entity_center_buffer<OVM::Entity::Face>() const {return faces_centers.buffer;}
    template<> const wgpu::Buffer& entity_center_buffer<OVM::Entity::HalfFace>() const {return faces_centers.buffer;}
    template<> const wgpu::Buffer& entity_center_buffer<OVM::Entity::Edge>() const {return edges_centers.buffer;}
    template<> const wgpu::Buffer& entity_center_buffer<OVM::Entity::HalfEdge>() const {return edges_centers.buffer;}
    template<> const wgpu::Buffer& entity_center_buffer<OVM::Entity::Vertex>() const {return vertices_positions.buffer;}
};

}
