#pragma once
#include "AxoPlotl/rendering/RendererBase.hpp"

namespace AxoPlotl
{

class ColoredEdgePropertyRenderer : public PropertyRendererBase
{
protected:
    // Mirrors Shader Unfiforms. the 16byte alignment is important!
    struct alignas(16) Uniforms {
        alignas(16) Mat4x4f mvp_;
        alignas(16) Vec2f viewport_size_;
        alignas(16) float line_width_ = 8.0f;
        alignas(16) ClipBox clip_box_;
        alignas(16) Property::Type type_ = Property::Type::COLOR;;
        alignas(16) Property::Filter value_filter_;
        alignas(16) uint32_t object_id_ = 0;
    } uniforms_;
    static_assert(offsetof(Uniforms,mvp_)%16==0);
    static_assert(offsetof(Uniforms,type_)%16==0);
    static_assert(offsetof(Uniforms,viewport_size_)%16==0);
    static_assert(offsetof(Uniforms,line_width_)%16==0);
    static_assert(offsetof(Uniforms,clip_box_)%16==0);
    static_assert(sizeof(Uniforms)%16==0);

    ColorMap property_color_map_;

public:
    inline float& line_width() {
        return uniforms_.line_width_;
    };

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

    ColoredEdgePropertyRenderer() {}

    ~ColoredEdgePropertyRenderer()
    {
        property_buffer_.destroy();
        property_buffer_.release();
        edge_index_buffer_.destroy();
        edge_index_buffer_.release();
        uniform_buffer_.destroy();
        uniform_buffer_.release();
    }

    struct EdgeInstance {
        uint32_t vh0_; // used to lookup position
        uint32_t vh1_; // used to lookup position
        uint32_t eh_; // points to EdgePropertyData
    };

    void init(uint32_t _object_id, Application* _app,
              wgpu::Buffer _position_buffer,
              const std::vector<std::pair<uint32_t,uint32_t>>& _edges);

    void update_property_data(const std::vector<Property::Data>& _data);

    void render(
        const Vec4f& _viewport,
        wgpu::RenderPassEncoder _render_pass,
        const Mat4x4f& _mvp) override;
private:
    uint32_t object_id_ = UINT32_MAX;
    size_t n_positions_;
    size_t n_edges_;

    void create_buffers(const std::vector<std::pair<uint32_t,uint32_t>>& _edges);

    void create_bind_group_layout();

    void create_bind_group();

    void create_pipeline();

    wgpu::Buffer edge_index_buffer_;
    wgpu::Buffer property_buffer_;
    wgpu::Buffer position_buffer_;
    wgpu::Buffer uniform_buffer_;

    static PipelineState pipeline_state_;
    wgpu::BindGroup bind_group_;
};

}
