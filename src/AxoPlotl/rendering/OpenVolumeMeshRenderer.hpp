#pragma once
#include "AxoPlotl/rendering/MeshCellRenderer.hpp"
#include "AxoPlotl/rendering/MeshEdgeRenderer.hpp"
#include "AxoPlotl/rendering/MeshFaceRenderer.hpp"
#include "AxoPlotl/rendering/MeshVertexRenderer.hpp"
#include "AxoPlotl/typedefs/glm.hpp"
#include "webgpu/webgpu.hpp"
#include <cstddef>
#include <vector>
#include <AxoPlotl/AxoPlotl_fwd.hpp>
#include <AxoPlotl/typedefs/ovm.hpp>

namespace AxoPlotl
{

class Application;

class OpenVolumeMeshRenderer : public PropertyRendererBase
{
public:
    inline MeshVertexRenderer& vertices() {
        return vertex_renderer_;
    }

    inline MeshEdgeRenderer& edges() {
        return edge_renderer_;
    }

    inline MeshFaceRenderer& faces() {
        return face_renderer_;
    }

    inline MeshCellRenderer& cells() {
        return cell_renderer_;
    }

    template<typename EntityTag>
    inline auto& entities();
    template<> inline auto& entities<OVM::Entity::Cell>() {return cell_renderer_;}
    template<> inline auto& entities<OVM::Entity::Face>() {return face_renderer_;}
    template<> inline auto& entities<OVM::Entity::Edge>() {return edge_renderer_;}
    template<> inline auto& entities<OVM::Entity::Vertex>() {return vertex_renderer_;}

    struct StaticData
    {
        std::vector<PropertyRendererBase::Position> positions_;
        std::vector<uint32_t> vertices_;
        std::vector<std::pair<uint32_t,uint32_t>> edges_;
        std::vector<std::vector<uint32_t>> faces_;
        std::vector<std::vector<std::vector<uint32_t>>> cells_;
    };

    OpenVolumeMeshRenderer() {}

    ~OpenVolumeMeshRenderer() {
        release();
    }

    // Setup Pipeline and layouts for given static data
    void init(Application* _app, const StaticData& _data);

    void render(const Vec4f& _viewport,
                wgpu::RenderPassEncoder _render_pass,
                const Mat4x4f& _mvp) override;

    void release();

    inline wgpu::Buffer position_buffer() const {
        return position_buffer_;
    }
private:
    size_t n_positions_;
    wgpu::Buffer position_buffer_;
    MeshVertexRenderer vertex_renderer_;
    MeshEdgeRenderer edge_renderer_;
    MeshFaceRenderer face_renderer_;
    MeshCellRenderer cell_renderer_;
};

}
