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
    struct Property
    {
        enum class Mode : uint32_t {
            COLOR = 0u,
            SCALAR = 1u,
            VEC3 = 2u,
        };
        struct Data {
            Vec4f value_ = {0,0,0,1};
        };
    };

    ColorMap property_color_map_;

    bool render_anything_ = true;
    bool render_vertices_ = true;
    bool render_edges_ = true;
    bool render_faces_ = true;
    bool render_cells_ = true;

    inline Property::Mode& vertex_property_mode() {
        return uniforms_.vertex_mode_;
    }

    inline Property::Mode& edge_property_mode() {
        return uniforms_.edge_mode_;
    }

    inline Property::Mode& face_property_mode() {
        return uniforms_.face_mode_;
    }

    inline Property::Mode& cell_property_mode() {
        return uniforms_.cell_mode_;
    }

    inline Vec2f& vertex_value_filter() {
        return uniforms_.vertex_value_filter_;
    }

    inline Vec2f& edge_value_filter() {
        return uniforms_.edge_value_filter_;
    }

    inline Vec2f& face_value_filter() {
        return uniforms_.face_value_filter_;
    }

    inline Vec2f& cell_value_filter() {
        return uniforms_.cell_value_filter_;
    }

    inline float& point_size() {
        return uniforms_.point_size_;
    };

    inline float& line_width() {
        return uniforms_.line_width_;
    };

    inline float& cell_scale() {
        return uniforms_.cell_scale_;
    };

    // struct ClipBox {
    //     alignas(16) Vec3f min_;
    //     alignas(16) Vec3f max_;
    //     alignas(16) int32_t enabled_ = false;
    // } clip_box_;

    // // Mirrors Shader Unfiforms. the 16bit alignment is important!
    // struct Uniforms {
    //     alignas(16) Mat4x4f mvp_;
    //     alignas(16) Property::Mode mode_ = Property::Mode::COLOR;
    //     alignas(16) Vec2f viewport_size_;
    //     alignas(16) float point_size_;
    //     alignas(16) float line_width_;
    //     alignas(16) float cell_scale_;
    //     alignas(16) Property::Filter filter_;
    //     alignas(16) ClipBox clip_box_;
    // };
    using Pad4 = uint32_t[1];
    using Pad8 = uint32_t[2];
    using Pad12 = uint32_t[3];

    struct alignas(16) ClipBox {
        alignas(16) Vec3f min_;
        alignas(16) Vec3f max_;
        alignas(16) int32_t enabled_ = false;
    } clip_box_;
    static_assert(sizeof(ClipBox)%16==0);

    // Mirrors Shader Unfiforms. the 16bit alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4f mvp_;
        alignas(16) Vec2f viewport_size_;
        alignas(16) float point_size_ = 5.0f;
        alignas(16) float line_width_ = 12.0f;
        alignas(16) float cell_scale_ = 0.9f;
        alignas(16) ClipBox clip_box_;
        alignas(16) Property::Mode vertex_mode_ = Property::Mode::COLOR;;
        alignas(16) Property::Mode edge_mode_ = Property::Mode::COLOR;
        alignas(16) Property::Mode face_mode_ = Property::Mode::COLOR;
        alignas(16) Property::Mode cell_mode_ = Property::Mode::COLOR;
        alignas(16) Vec2f vertex_value_filter_;
        alignas(16) Vec2f edge_value_filter_;
        alignas(16) Vec2f face_value_filter_;
        alignas(16) Vec2f cell_value_filter_;
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(offsetof(Uniforms,vertex_mode_)%16==0);
    static_assert(offsetof(Uniforms,viewport_size_)%16==0);
    static_assert(offsetof(Uniforms,point_size_)%16==0);
    static_assert(offsetof(Uniforms,line_width_)%16==0);
    static_assert(offsetof(Uniforms,cell_scale_)%16==0);
    static_assert(offsetof(Uniforms,clip_box_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

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
        std::vector<CellHandle> cell_outline_indices_;
        std::vector<Position> cell_incenters_;
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

    void render(const Vec4f& _viewport, wgpu::RenderPassEncoder _render_pass, const Mat4x4f& _mvp);

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

    void create_cell_outline_pipeline();

    //size_t n_vertex_point_indices_ = 0;
    //size_t n_edge_line_indices_ = 0;
    size_t n_face_triangle_indices_ = 0;
    size_t n_cell_triangle_indices_ = 0;
    size_t n_cell_outline_indices_ = 0;

    Application* app_;

    wgpu::Buffer positionBuffer_;
    wgpu::Buffer vertexIndexBuffer_;
    wgpu::Buffer edgeIndexBuffer_;
    wgpu::Buffer faceTriangleIndexBuffer_;
    wgpu::Buffer cellTriangleIndexBuffer_;
    wgpu::Buffer cellOutlineIndexBuffer;

    wgpu::Buffer vertex_property_buffer_;
    wgpu::Buffer edge_property_buffer_;
    wgpu::Buffer face_property_buffer_;
    wgpu::Buffer cell_property_buffer_;
    wgpu::Buffer cell_incenter_buffer_;
    wgpu::Buffer uniform_buffer_;

    static wgpu::RenderPipeline vertices_pipeline_;
    static wgpu::RenderPipeline edges_pipeline_;
    static wgpu::RenderPipeline face_triangles_pipeline_;
    static wgpu::RenderPipeline cell_triangles_pipeline_;
    static wgpu::RenderPipeline cell_outline_pipeline_;
    //static wgpu::DepthStencilState depth_stencil_state_;

    static wgpu::BindGroupLayout bind_group_layout_;
    wgpu::BindGroup bind_group_;

    MeshVertexRenderer vertex_renderer_;
    MeshEdgeRenderer edge_renderer_;
    MeshFaceRenderer face_renderer_;
    MeshCellRenderer cell_renderer_;
};

}
