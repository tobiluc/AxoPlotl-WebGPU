#pragma once
#include "AxoPlotl/rendering/RendererBase.hpp"

namespace AxoPlotl
{

class ColoredCellPropertyRenderer : public PropertyRendererBase
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
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(offsetof(Uniforms,type_)%16==0);
    static_assert(offsetof(Uniforms,cell_scale_)%16==0);
    static_assert(offsetof(Uniforms,viewport_size_)%16==0);
    static_assert(offsetof(Uniforms,clip_box_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

    ColorMap property_color_map_;

public:
    inline float& cell_scale() {
        return uniforms_.cell_scale_;
    }

    inline Property::Type& property_type() {
        return uniforms_.type_;
    }

    inline Property::Filter& property_filter() {
        return uniforms_.value_filter_;
    }

    inline ClipBox& clip_box() {
        return uniforms_.clip_box_;
    }

    inline ColorMap& color_map() {
        return property_color_map_;
    }

    ColoredCellPropertyRenderer() {}

    ~ColoredCellPropertyRenderer()
    {
        property_buffer_.destroy();
        property_buffer_.release();
        triangle_index_buffer_.destroy();
        triangle_index_buffer_.release();
        line_index_buffer_.destroy();
        line_index_buffer_.release();
        uniform_buffer_.destroy();
        uniform_buffer_.release();
        // center_buffer_.destroy();
        // center_buffer_.release();
    }

    void init(Application* _app,
      wgpu::Buffer _vertices_position_buffer,
      const std::vector<std::vector<std::vector<uint32_t>>>& _cells,
        wgpu::Buffer _cells_center_buffer);

    void update_property_data(const std::vector<Property::Data>& _data);

    void render(
        const Vec4f& _viewport,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) override;
private:
    size_t n_positions_;
    size_t n_cells_;
    size_t n_triangle_indices_;
    size_t n_line_indices_;

    void create_buffers(
        const std::vector<std::vector<std::vector<uint32_t>>>& _cells);

    void create_bind_group_layout();

    void create_bind_group();

    void create_triangle_pipeline();

    void create_line_pipeline();

    wgpu::Buffer triangle_index_buffer_;
    wgpu::Buffer line_index_buffer_;
    wgpu::Buffer property_buffer_;
    wgpu::Buffer vertices_position_buffer_;
    wgpu::Buffer uniform_buffer_;
    wgpu::Buffer cells_center_buffer_;

    static wgpu::RenderPipeline triangle_pipeline_;
    static wgpu::RenderPipeline line_pipeline_;
    static wgpu::BindGroupLayout bind_group_layout_;
    wgpu::BindGroup bind_group_;
};

}
