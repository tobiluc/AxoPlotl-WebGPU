#pragma once
#include "AxoPlotl/typedefs/glm.hpp"
#include <webgpu/webgpu.hpp>
#include <vector>

namespace AxoPlotl
{

class VolumeMeshRenderer
{
public:

    bool render_anything_ = true;
    bool render_vertices_ = true;
    bool render_edges_ = true;
    bool render_faces_ = true;
    bool render_cells_ = true;

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

    struct PropertyData {
        Vec4f color_;
    };

    using Position = Vec4f;

    struct EdgeHandle {
        uint32_t vertex_index_; // used to lookup position
        uint32_t edge_index_; // points to EdgePropertyData
    };

    struct FaceHandle {
        uint32_t vertex_index_;
        uint32_t face_index_;
    };

    struct CellHandle {
        uint32_t vertex_index_;
        uint32_t cell_index_;
    };

    struct StaticData
    {
        size_t n_faces_ = 0;
        size_t n_cells_ = 0;
        std::vector<Position> positions_;
        std::vector<uint32_t> vertex_draw_indices_;
        std::vector<EdgeHandle> edge_draw_indices_;
        std::vector<FaceHandle> face_draw_triangle_indices_;
        std::vector<CellHandle> cell_draw_triangle_indices_;
    };

    VolumeMeshRenderer() {}

    // Setup Pipeline and layouts for given static data
    void init(Context _context, const StaticData& _data);

    void update_vertex_property_data(const std::vector<PropertyData>& _data);

    void update_edge_property_data(const std::vector<PropertyData>& _data);

    void update_face_property_data(const std::vector<PropertyData>& _data);

    void update_cell_property_data(const std::vector<PropertyData>& _data);

    void render(wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _mvp);
private:
    size_t n_positions_;
    size_t n_vertices_;
    size_t n_edges_;
    size_t n_faces_;
    size_t n_cells_;

    void create_buffers(const StaticData& _data);

    void create_bind_group_layout();

    void create_bind_group();

    void create_vertex_point_pipeline();

    void create_edge_line_pipeline();

    void create_face_triangle_pipeline();

    void create_cell_triangle_pipeline();

    size_t n_vertex_point_indices_ = 0;
    size_t n_edge_line_indices_ = 0;
    size_t n_face_triangle_indices_ = 0;
    size_t n_cell_triangle_indices_ = 0;

    Context context_;

    wgpu::Buffer positionBuffer_;
    wgpu::Buffer vertexIndexBuffer_;
    wgpu::Buffer edgeIndexBuffer_;
    wgpu::Buffer faceTriangleIndexBuffer_;
    wgpu::Buffer cellTriangleIndexBuffer_;

    wgpu::Buffer vertexPropertyBuffer_;
    wgpu::Buffer edgePropertyBuffer_;
    wgpu::Buffer facePropertyBuffer_;
    wgpu::Buffer cellPropertyBuffer_;
    wgpu::Buffer uniformBuffer_;

    static wgpu::RenderPipeline vertex_points_pipeline_;
    static wgpu::RenderPipeline edge_lines_pipeline_;
    static wgpu::RenderPipeline face_triangles_pipeline_;
    static wgpu::RenderPipeline cell_triangles_pipeline_;

    wgpu::BindGroupLayout bind_group_layput_;
    wgpu::BindGroup bind_group_;

};

}
