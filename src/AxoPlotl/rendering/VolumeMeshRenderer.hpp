#pragma once
#include "AxoPlotl/typedefs/glm.hpp"
#include <webgpu/webgpu.hpp>
#include <vector>

namespace AxoPlotl
{

class VolumeMeshRenderer
{
public:

    struct Context {
        wgpu::Device device_;
        wgpu::Surface surface_;
        wgpu::Adapter adapter_;
    };

    float line_width_ = 5.0f;
    float point_size_ = 12.0f;

    struct Uniforms {
        Mat4x4f mvp_;
    } unforms_;

    struct VertexPropertyData {
        Vec4f color_;
    };

    struct EdgePropertyData {
        Vec4f color_;
    };

    using Position = Vec4f;

    struct EdgeHandle {
        uint32_t vertex_index_; // used to lookup position
        uint32_t edge_index_; // points to EdgePropertyData
    };

    struct StaticData
    {
        std::vector<Position> positions_;
        std::vector<uint32_t> vertex_draw_indices_;
        std::vector<EdgeHandle> edge_draw_indices_;
    };

    VolumeMeshRenderer(Context _context);

    // Setup Pipeline and layouts for given static data
    void init(const StaticData& _data);

    void update_vertex_property_data(const std::vector<VertexPropertyData>& _data);

    void update_edge_property_data(const std::vector<EdgePropertyData>& _data);

    void render(wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _mvp);
private:
    size_t n_vertices_;
    size_t n_edges_;

    void create_buffers(const StaticData& _data);

    void create_bind_group_layout();

    void create_bind_group();

    void create_vertex_point_pipeline();

    void create_edge_line_pipeline();

    size_t n_vertex_point_indices_ = 0;
    size_t n_edge_line_indices_ = 0;

    Context context_;

    wgpu::Buffer positionBuffer_;
    wgpu::Buffer vertexIndexBuffer_;
    wgpu::Buffer edgeIndexBuffer_;

    wgpu::Buffer vertexPropertyBuffer_;
    wgpu::Buffer edgePropertyBuffer_;
    wgpu::Buffer uniformBuffer_;

    wgpu::RenderPipeline vertex_points_pipeline_;
    wgpu::RenderPipeline edge_lines_pipeline_;

    wgpu::BindGroupLayout bind_group_layput_;
    wgpu::BindGroup bind_group_;

};

}
