#pragma once
#include <AxoPlotl/rendering/renderers/RendererBase.hpp>
#include "AxoPlotl/rendering/MyGpuContext.hpp"
#include "AxoPlotl/rendering/renderers/PropertyRendererBase.hpp"

namespace AxoPlotl::internal
{

class ColoredCellRenderer : public PropertyRendererBase
{
protected:
    // Mirrors Shader Unfiforms. the 16byte alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4f mvp_;
        alignas(16) Vec2f viewport_size_;
        alignas(16) float cell_scale_ = 0.9f;
        alignas(16) ClipBox clip_box_;
        alignas(16) Property::Type type_ = Property::Type::COLOR;
        alignas(16) Property::Filter value_filter_;
        alignas(16) uint32_t object_id_ = 0;
        alignas(16) Vec4f ambient_ = Vec4f(0.9,0.9,0.9,1);
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(offsetof(Uniforms,type_)%16==0);
    static_assert(offsetof(Uniforms,cell_scale_)%16==0);
    static_assert(offsetof(Uniforms,viewport_size_)%16==0);
    static_assert(offsetof(Uniforms,clip_box_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

public:
    float& cell_scale() {
        return uniforms_.cell_scale_;
    }

    Property::Type& property_type() override {
        return uniforms_.type_;
    }

    Property::Filter& property_filter() override {
        return uniforms_.value_filter_;
    }

    ClipBox& clip_box() {
        return uniforms_.clip_box_;
    }

    Vec4f& ambient() {
        return uniforms_.ambient_;
    }

    ColoredCellRenderer() {}

    ~ColoredCellRenderer()
    {
        clear();
    }

    void clear() override;

    void init(uint32_t _object_id,
              const GpuContext& _gpu_context,
              const RenderContext& _render_context,
      wgpu::Buffer _vertices_position_buffer,
      const std::vector<std::vector<std::vector<uint32_t>>>& _cells,
        wgpu::Buffer _cells_center_buffer);

    void update_property_data(const std::vector<Property::Data>& _data);

    void render(
        const ViewSize& _size,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) override;

    void configure_like(const ColoredCellRenderer& _r) {
        uniforms_ = _r.uniforms_;
        enabled_ = _r.enabled_;
    }

private:
    uint32_t object_id_ = UINT32_MAX;
    size_t n_positions_ = 0;
    size_t n_cells_ = 0;
    size_t n_triangle_indices_ = 0;
    size_t n_line_indices_ = 0;

    void create_buffers(
        const std::vector<std::vector<std::vector<uint32_t>>>& _cells);

    void create_bind_group_layout();

    void create_bind_group(const GpuContext& _gpu_context);

    void create_triangle_pipeline(const GpuContext& _gpu_context, const RenderContext &_render_context);

    void create_line_pipeline(const GpuContext& _gpu_context, const RenderContext &_render_context);

    wgpu::Buffer triangle_index_buffer_;
    wgpu::Buffer line_index_buffer_;
    wgpu::Buffer property_buffer_;
    wgpu::Buffer vertices_position_buffer_;
    wgpu::Buffer uniform_buffer_;
    wgpu::Buffer cells_center_buffer_;

    static PipelineState triangle_pipeline_state_;
    static PipelineState outline_pipeline_state_;
    wgpu::BindGroup bind_group_;
};

}
