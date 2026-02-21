#pragma once
#include "AxoPlotl/rendering/ColorMap.hpp"
#include "AxoPlotl/typedefs/glm.hpp"
#include "GLFW/glfw3.h"
#include "webgpu/webgpu.hpp"
#include <vector>
#include <AxoPlotl/AxoPlotl_fwd.hpp>

namespace AxoPlotl
{

class Application;

class VolumeMeshRenderer
{
public:
    struct Property
    {
        enum class Mode : uint32_t {
            COLOR = 0,
            SCALAR = 1,
            VEC3 = 2,
        } mode_ = Mode::COLOR;
        struct Data {
            Vec4f value_ = {0,0,0,1};
        };
        union Filter {
            Vec2f scalar_range_ = {0,5};
        } filter_;
    };

    ColorMap property_color_map_;

    bool render_anything_ = true;
    bool render_vertices_ = true;
    bool render_edges_ = true;
    bool render_faces_ = true;
    bool render_cells_ = true;

    Property vertex_property_;
    Property edge_property_;
    Property face_property_;
    Property cell_property_;

    float line_width_ = 5.0f;
    float point_size_ = 12.0f;
    float cell_scale_ = 0.9f;;

    struct ClipBox {
        alignas(16) Vec3f min_;
        alignas(16) Vec3f max_;
        alignas(16) int32_t enabled_ = false;
    } clip_box_;

    // Mirrors Shader Unfiforms. the 16bit alignment is important!
    struct Uniforms {
        alignas(16) Mat4x4f mvp_;
        alignas(16) Property::Mode mode_ = Property::Mode::COLOR;
        alignas(16) Vec2f viewport_size_;
        alignas(16) float point_size_;
        alignas(16) float line_width_;
        alignas(16) float cell_scale_;
        alignas(16) Property::Filter filter_;
        alignas(16) ClipBox clip_box_;
    };

    using Position = Vec4f;

    struct EdgeInstance {
        uint32_t vh0_; // used to lookup position
        uint32_t vh1_; // used to lookup position
        uint32_t eh_; // points to EdgePropertyData
    };

    struct FaceHandle {
        uint32_t vertex_index_;
        uint32_t face_index_;
    };

    struct CellHandle {
        uint32_t vertex_index_;
        uint32_t cell_index_;
        Vec3f incenter_;
    };

    struct StaticData
    {
        size_t n_faces_ = 0;
        size_t n_cells_ = 0;
        std::vector<Position> positions_;
        std::vector<uint32_t> vertex_instances_;
        std::vector<EdgeInstance> edge_instances_;
        std::vector<FaceHandle> face_draw_triangle_indices_;
        std::vector<CellHandle> cell_draw_triangle_indices_;
    };

    VolumeMeshRenderer() {}

    ~VolumeMeshRenderer() {
        release();
    }

    // Setup Pipeline and layouts for given static data
    void init(Application* _app, const StaticData& _data);

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

    void create_vertices_pipeline();

    void create_edges_pipeline();

    void create_face_triangle_pipeline();

    void create_cell_triangle_pipeline();

    //size_t n_vertex_point_indices_ = 0;
    //size_t n_edge_line_indices_ = 0;
    size_t n_face_triangle_indices_ = 0;
    size_t n_cell_triangle_indices_ = 0;

    Application* app_;

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

    static wgpu::RenderPipeline vertices_pipeline_;
    static wgpu::RenderPipeline edges_pipeline_;
    static wgpu::RenderPipeline face_triangles_pipeline_;
    static wgpu::RenderPipeline cell_triangles_pipeline_;
    //static wgpu::DepthStencilState depth_stencil_state_;

    static wgpu::BindGroupLayout bind_group_layout_;
    wgpu::BindGroup bind_group_;

};

}
