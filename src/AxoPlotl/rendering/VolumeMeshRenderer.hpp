#pragma once
#include "AxoPlotl/rendering/ColorMap.hpp"
#include "AxoPlotl/typedefs/glm.hpp"
#include <webgpu/webgpu.hpp>
#include <vector>

namespace AxoPlotl
{

class VolumeMeshRenderer
{
public:
    struct Property
    {
        enum class Mode : uint32_t {
            TRANSPARENT = UINT32_MAX,
            COLOR = 0,
            SCALAR = 1,
            VEC3 = 2,
        } mode_ = Mode::COLOR;
        struct Data {
            Vec4f value_ = {0,0,0,1};
        };
        union Filter {
            Vec2f scalar_range_ = {-1,1};
        } filter_;
        ColorMap color_map_;
    };

    bool render_anything_ = true;
    Property vertex_property_;
    Property edge_property_;
    Property face_property_;
    Property cell_property_;

    struct Context {
        wgpu::Device device_;
        wgpu::Surface surface_;
        wgpu::Adapter adapter_;
    };

    float line_width_ = 5.0f;
    float point_size_ = 12.0f;

    // Mirrors Shader Unfiforms
    struct Uniforms {
        Mat4x4f mvp_; // 64b
        Property::Mode mode_ = Property::Mode::COLOR; // 4b
        std::byte padding_[12]; // total must be multiple of 16 bytes
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

    ~VolumeMeshRenderer() {
        release();
    }

    // Setup Pipeline and layouts for given static data
    void init(const Context& _context, const StaticData& _data);

    void update_vertex_property_data(const std::vector<Property::Data>& _data);

    void update_edge_property_data(const std::vector<Property::Data>& _data);

    void update_face_property_data(const std::vector<Property::Data>& _data);

    void update_cell_property_data(const std::vector<Property::Data>& _data);

    void render(wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _mvp);

    void release();
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

    wgpu::Buffer vertex_property_buffer_;
    wgpu::Buffer edge_property_buffer_;
    wgpu::Buffer face_property_buffer_;
    wgpu::Buffer cell_property_buffer_;
    wgpu::Buffer uniform_buffer_;

    static wgpu::RenderPipeline vertex_points_pipeline_;
    static wgpu::RenderPipeline edge_lines_pipeline_;
    static wgpu::RenderPipeline face_triangles_pipeline_;
    static wgpu::RenderPipeline cell_triangles_pipeline_;
    static wgpu::DepthStencilState depth_stencil_state_;

    static wgpu::BindGroupLayout bind_group_layout_;
    wgpu::BindGroup bind_group_;

};

}
