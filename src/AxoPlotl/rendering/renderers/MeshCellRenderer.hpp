#pragma once
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>
#include "AxoPlotl/rendering/MyGpuContext.hpp"
#include "AxoPlotl/rendering/MeshData.hpp"
#include "AxoPlotl/rendering/renderers/PropertyRendererBase.hpp"

namespace AxoPlotl::internal
{

class MeshCellRenderer : public PropertyRendererBase
{
protected:
    // Mirrors Shader Unfiforms. the 16byte alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4 mvp_;
        alignas(16) Vec2 viewport_size_;
        alignas(16) float cell_scale_ = 0.9f;
        alignas(16) ClipBox clip_box_;
        alignas(16) uint32_t object_id_ = 0;
        alignas(16) Vec4 ambient_ = {0.8,0.8,0.8,1};
        alignas(16) DirectionalLight dir_light_ ;
        alignas(16) PropFilterOptions filter_options_;
    } uniforms_;
    static_assert(offsetof(Uniforms, clip_box_) == 96);
    static_assert(sizeof(Uniforms) == 304);

public:
    float& cell_scale() {
        return uniforms_.cell_scale_;
    }

    PropFilterOptions& property_filter_options() override {
        return uniforms_.filter_options_;
    }

    ClipBox& clip_box() {
        return uniforms_.clip_box_;
    }

    Vec4& ambient() {
        return uniforms_.ambient_;
    }

    DirectionalLight& directional_light() {
        return uniforms_.dir_light_;
    }

    MeshCellRenderer(uint32_t _max_draw_calls = 1)
        : PropertyRendererBase(_max_draw_calls)
    {}

    ~MeshCellRenderer()
    {
        clear();
    }

    void clear() override;

    void init(uint32_t _object_id,
              const GpuContext& _gpu_context,
              const RenderContext& _render_context,
              const BufferData<Position>& _vertices_positions,
              const BufferData<VolMeshData::CellIndex>& _cells_triangle_indices,
              const BufferData<Position>& _cells_centers,
              const BufferData<VolMeshData::Normal>& _halffaces_normals);

    void update_property_data(const std::vector<PropContainer>& _data);

    void render(
        const ViewSize& _size,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4& _mvp);

    void configure_like(const MeshCellRenderer& _r) {
        uniforms_ = _r.uniforms_;
        enabled_ = _r.enabled_;
    }

    void set_default_uniforms() override {
        uniforms_ = Uniforms();
    };

private:
    uint32_t object_id_ = UINT32_MAX;
    size_t n_positions_ = 0;
    size_t n_cells_ = 0;
    size_t n_triangle_indices_ = 0;
    size_t n_normals_ = 0;
    bool outlines_enabled_ = false;

    void create_buffers();

    void create_bind_group_layout();

    void create_bind_group(const GpuContext& _gpu_context);

    void create_triangle_pipeline(const GpuContext& _gpu_context, const RenderContext &_render_context);

    wgpu::Buffer triangle_index_buffer_;
    wgpu::Buffer property_buffer_;
    wgpu::Buffer vertices_position_buffer_;
    wgpu::Buffer uniform_buffer_;
    wgpu::Buffer cells_center_buffer_;
    wgpu::Buffer halfface_normal_buffer_;

    static PipelineState triangle_pipeline_state_;
    wgpu::BindGroup bind_group_;
};

}
