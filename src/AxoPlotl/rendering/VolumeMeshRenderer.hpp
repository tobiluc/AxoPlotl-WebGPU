#pragma once
#include "AxoPlotl/rendering/ColorMap.hpp"
#include "AxoPlotl/rendering/MeshCellRenderer.hpp"
#include "AxoPlotl/rendering/MeshEdgeRenderer.hpp"
#include "AxoPlotl/rendering/MeshFaceRenderer.hpp"
#include "AxoPlotl/rendering/MeshVertexRenderer.hpp"
#include "AxoPlotl/typedefs/glm.hpp"
#include "GLFW/glfw3.h"
#include "webgpu/webgpu.hpp"
#include <cstddef>
#include <vector>
#include <AxoPlotl/AxoPlotl_fwd.hpp>

namespace AxoPlotl
{

class Application;

class VolumeMeshRenderer
{
public:

    bool render_anything_ = true;

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

    struct StaticData
    {
        std::vector<RendererBase::Position> positions_;
        std::vector<uint32_t> vertices_;
        std::vector<std::pair<uint32_t,uint32_t>> edges_;
        std::vector<std::vector<uint32_t>> faces_;
        std::vector<std::vector<std::vector<uint32_t>>> cells_;
    };

    VolumeMeshRenderer() {}

    ~VolumeMeshRenderer() {
        release();
    }

    // Setup Pipeline and layouts for given static data
    void init(Application* _app, const StaticData& _data);

    void render(const Vec4f& _viewport, wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _mvp);

    void release();
private:
    size_t n_positions_;
    Application* app_;
    wgpu::Buffer position_buffer_;
    MeshVertexRenderer vertex_renderer_;
    MeshEdgeRenderer edge_renderer_;
    MeshFaceRenderer face_renderer_;
    MeshCellRenderer cell_renderer_;
};

}
